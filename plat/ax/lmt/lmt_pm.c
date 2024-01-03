/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#include <assert.h>
#include <platform_def.h>
#include <lmt_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/psci/psci.h>
#include <lib/el3_runtime/cpu_data.h>
#include <plat/common/platform.h>
#include <drivers/arm/gicv3.h>
#include <drivers/arm/gic_common.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>

struct cpu_offset_map {
    uint64_t offset;
    u_register_t cpu_idx;
} cpu_map [] = {
    {CPU_CL0_C0_0, 0x000},
    {CPU_CL0_C1_0, 0x100},
    {CPU_CL0_C2_0, 0x200},
    {CPU_CL0_C3_0, 0x300},
    {CPU_CL1_C0_0, 0x10000},
    {CPU_CL1_C1_0, 0x10100},
};

/*
 * The secure entry point to be used on warm reset.
 */
static unsigned long secure_entrypoint;

/* Make composite power state parameter till power level 0 */
#if PSCI_EXTENDED_STATE_ID

#define lmt_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))
#else
#define lmt_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))
#endif /* PSCI_EXTENDED_STATE_ID */


#define lmt_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << LMT_LOCAL_PSTATE_WIDTH) | \
		 lmt_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

/*
 *  The table storing the valid idle power states. Ensure that the
 *  array entries are populated in ascending order of state-id to
 *  enable us to use binary search during power state validation.
 *  The table must be terminated by a NULL entry.
 */
static const unsigned int lmt_pm_idle_states[] = {
	/* State-id - 0x01 */
	lmt_make_pwrstate_lvl1(LMT_LOCAL_STATE_RUN, LMT_LOCAL_STATE_RET,
				MPIDR_AFFLVL0, PSTATE_TYPE_STANDBY),
	/* State-id - 0x02 */
	lmt_make_pwrstate_lvl1(LMT_LOCAL_STATE_RUN, LMT_LOCAL_STATE_OFF,
				MPIDR_AFFLVL0, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x22 */
	lmt_make_pwrstate_lvl1(LMT_LOCAL_STATE_OFF, LMT_LOCAL_STATE_OFF,
				MPIDR_AFFLVL1, PSTATE_TYPE_POWERDOWN),
	0,
};

/**
 * lmt_validate_power_state() - Platform handler to check if power state is valid
 *
 * @power_state: power state prepares to go to.
 * @req_state: power domain level specific local states
 *
 * This function is called by the PSCI implementation during the ``CPU_SUSPEND``
 * call to validate the ``power_state`` parameter of the PSCI API and if valid,
 * populate it in ``req_state`` (second argument) array as power domain level
 * specific local states. If the ``power_state`` is invalid, the platform must
 * return PSCI_E_INVALID_PARAMS as error, which is propagated back to the
 * normal world PSCI client.
 *
 * Return: PSCI_E_SUCCESS if success, error code otherwise.
 */
int lmt_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;

	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!lmt_pm_idle_states[i]; i++) {
		if (power_state == lmt_pm_idle_states[i])
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!lmt_pm_idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id &
						LMT_LOCAL_PSTATE_MASK;
		state_id >>= LMT_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}

/**
 * lmt_validate_ns_entrypoint() - Platform handler to check ns_entrypoint
 *
 * @ns_entrypoint: entrypoint address to check
 *
 * This function need to check if the ns_entrypoint is in non-secure world.
 *
 * Return: PSCI_E_SUCCESS if success, error code otherwise.
 */
static int lmt_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= LMT_DRAM_BASE) &&
		(entrypoint < (LMT_DRAM_BASE + LMT_NS_DDR_SIZE)))
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

/**
 * lmt_cpu_standby() - Put CPU to standy mode
 *
 * @cpu_state: local cpu state
 *
 * This function will put cpu into idle state, and will return when cpu wakeup.
 *
 * Return: void
 */
void lmt_cpu_standby(plat_local_state_t cpu_state)
{
	assert(cpu_state == LMT_LOCAL_STATE_RET);

	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

static uint64_t get_offset_addr_by_mpidr(u_register_t mpidr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cpu_map); i++) {
		if (cpu_map[i].cpu_idx == mpidr)
			return cpu_map[i].offset;
	}

	return ULONG_MAX;
}

/**
 * lmt_pwr_domain_on() -	Power on a cpu.
 *
 * @mpidr: mpidr value for this cpu
 *
 * This will be call before cpu power on, need to take action to bring cpu on
 *
 * Return: PSCI_E_SUCCESS if success, error code otherwise
 */
static int lmt_pwr_domain_on(u_register_t mpidr)
{
	int rc = PSCI_E_SUCCESS;
	unsigned pos = plat_core_pos_by_mpidr(mpidr);
	uint64_t *hold_base = (uint64_t *)PLAT_LMT_HOLD_BASE;
	uint64_t offset = get_offset_addr_by_mpidr(mpidr);

	hold_base[pos] = PLAT_LMT_HOLD_STATE_GO;
	sev();

	mmio_setbits_32(LMT_PMU_BASE + offset, BIT(12));

	return rc;
}

/**
 * lmt_pwr_domain_off() - Power off a cpu.
 *
 * @target_state: CPU topological state
 *
 * This will be call before cpu power off, need to take action to put cpu off
 *
 * Return: void
 */
static void lmt_pwr_domain_off(const psci_power_state_t *target_state)
{
	lmt_pwr_gic_off();
}

/**
 * lmt_pwr_domain_suspend() - Put system into suspend state.
 *
 * @target_state: Platform coordinated target local power states
 *
 * Perform the platform specific actions to prepare to suspend the calling
 * CPU and its higher parent power domain levels as indicated by the
 * ``target_state`` (first argument). It is called by the PSCI ``CPU_SUSPEND``
 * API implementation.
 *
 * Return: void
 */
static void lmt_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	assert(0);
}

void __dead2 plat_secondary_cold_boot_setup(void);
/**
 * lmt_pwr_down_wfi() - Powerdown the CPU
 *
 * @target_state: Platform coordinated target local power states
 *
 * Perform platform specific actions including the ``wfi`` invocation which
 * allows the CPU to powerdown.
 *
 * Return: void
 */
static void __dead2 lmt_pwr_down_wfi(const psci_power_state_t *target_state)
{
	disable_mmu_el3();
	plat_secondary_cold_boot_setup();
}

/**
 * lmt_pwr_domain_on_finish() - Platform handler when cpu is powered on.
 *
 * @target_state: Platform coordinated target local power states
 *
 * This function is called by the PSCI implementation after the calling CPU is
 * powered on and released from reset in response to an earlier PSCI ``CPU_ON``
 * call. It performs the platform-specific setup required to initialize enough
 * state for this CPU to enter the normal world and also provide secure runtime
 * firmware services.
 *
 * Return: void
 */
static void lmt_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					LMT_LOCAL_STATE_OFF);

	lmt_pwr_gic_on_finish();
}

/**
 * lmt_pwr_domain_suspend_finish() - Platform handler after wakeup.
 *
 * @target_state: Platform coordinated target local power states
 *
 * Performs the platform-specific setup required to restore the saved state for
 * this CPU to resume execution in the normal world and also provide secure
 * runtime firmware services. This function will need to restore Distributor,
 * Redistributors or ITS context.
 *
 * Return: void
 */
static void lmt_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	assert(0);
}

/**
 * lmt_system_off() - Platform handler to poweroff the system.
 *
 * Performs the platform-specific setup to power off the whole system.
 * Due to we use fake shutdown for J5, this function just while (1) to wfi().
 *
 * Return: void
 */
static void __dead2 lmt_system_off(void)
{
	panic();
}

/**
 * lmt_system_reset() - Platform handler to reboot the system.
 *
 * Performs the platform-specific setup to reboot the whole system.
 * We need to generated a wdt reset to reset the whole soc. In case of wdt
 * trigger failed, going to panic().
 *
 * Return: void
 */
static void __dead2 lmt_system_reset(void)
{
	panic();
}

/**
 * lmt_psci_ops - j6 platform psci ops
 */
static plat_psci_ops_t lmt_psci_ops = {
	.cpu_standby = lmt_cpu_standby,
	.pwr_domain_on = lmt_pwr_domain_on,
	.pwr_domain_off = lmt_pwr_domain_off,
	.pwr_domain_suspend = lmt_pwr_domain_suspend,
	.pwr_domain_pwr_down_wfi = lmt_pwr_down_wfi,
	.pwr_domain_on_finish = lmt_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = lmt_pwr_domain_suspend_finish,
	.system_off = lmt_system_off,
	.system_reset = lmt_system_reset,
	.validate_power_state = lmt_validate_power_state,
	.validate_ns_entrypoint = lmt_validate_ns_entrypoint,
};

/**
 * plat_setup_psci_ops() - Platform handler to setup psci ops
 *
 * @sec_entrypoint: entry to going to after cpu powered on.
 * @psci_ops: pointer to store psci ops.
 *
 * This function need to initialze
 *
 * Return: void.
 */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uintptr_t *mailbox = (void *)PLAT_LMT_TRUSTED_MAILBOX_BASE;

	*mailbox = sec_entrypoint;
	secure_entrypoint = (unsigned long)sec_entrypoint;

	assert(psci_ops);
	*psci_ops = &lmt_psci_ops;

	return 0;
}
