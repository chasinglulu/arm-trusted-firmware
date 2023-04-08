/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/psci/psci.h>
#include <lib/el3_runtime/cpu_data.h>
#include <plat/common/platform.h>
#include <drivers/arm/gicv3.h>
#include <drivers/arm/gic_common.h>
#include <lib/el3_runtime/context_mgmt.h>

/* Macros to read the rk power domain state */
#define HB_CORE_PWR_STATE(state)			\
	((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define HB_CLUSTER_PWR_STATE(state)			\
	((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define HB_SYSTEM_PWR_STATE(state)			\
	((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

/*
 * The secure entry point to be used on warm reset.
 */
static unsigned long secure_entrypoint;

/**
 * sigi_cpu_standby() - Put CPU to standy mode
 *
 * @cpu_state: local cpu state
 *
 * This function will put cpu into idle state, and will return when cpu wakeup.
 *
 * Return: void
 */
void sigi_cpu_standby(plat_local_state_t cpu_state)
{
}

/**
 * sigi_pwr_domain_on() -	Power on a cpu.
 *
 * @mpidr: mpidr value for this cpu
 *
 * This will be call before cpu power on, need to take action to bring cpu on
 *
 * Return: PSCI_E_SUCCESS if success, error code otherwise
 */
static int sigi_pwr_domain_on(u_register_t mpidr)
{
    return 0;
}

/**
 * sigi_pwr_domain_off() - Power off a cpu.
 *
 * @target_state: CPU topological state
 *
 * This will be call before cpu power off, need to take action to put cpu off
 *
 * Return: void
 */
static void sigi_pwr_domain_off(const psci_power_state_t *target_state)
{
}

/**
 * sigi_pwr_domain_suspend() - Put system into suspend state.
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
static void sigi_pwr_domain_suspend(const psci_power_state_t *target_state)
{
}

void __dead2 plat_secondary_cold_boot_setup(void);
/**
 * sigi_pwr_down_wfi() - Powerdown the CPU
 *
 * @target_state: Platform coordinated target local power states
 *
 * Perform platform specific actions including the ``wfi`` invocation which
 * allows the CPU to powerdown.
 *
 * Return: void
 */
static void __dead2 sigi_pwr_down_wfi(const psci_power_state_t *target_state)
{
    disable_mmu_el3();
	plat_secondary_cold_boot_setup();
}

/**
 * sigi_pwr_domain_on_finish() - Platform handler when cpu is powered on.
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
static void sigi_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
}

/**
 * sigi_pwr_domain_suspend_finish() - Platform handler after wakeup.
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
static void sigi_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
}

/**
 * sigi_system_off() - Platform handler to poweroff the system.
 *
 * Performs the platform-specific setup to power off the whole system.
 * Due to we use fake shutdown for J5, this function just while (1) to wfi().
 *
 * Return: void
 */
static void __dead2 sigi_system_off(void)
{
    panic();
}

/**
 * sigi_system_reset() - Platform handler to reboot the system.
 *
 * Performs the platform-specific setup to reboot the whole system.
 * We need to generated a wdt reset to reset the whole soc. In case of wdt
 * trigger failed, going to panic().
 *
 * Return: void
 */
static void __dead2 sigi_system_reset(void)
{
    panic();
}

/**
 * sigi_validate_power_state() - Platform handler to check if power state is valid
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
int sigi_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
    return 0;
}

/**
 * sigi_validate_ns_entrypoint() - Platform handler to check ns_entrypoint
 *
 * @ns_entrypoint: entrypoint address to check
 *
 * This function need to check if the ns_entrypoint is in non-secure world.
 *
 * Return: PSCI_E_SUCCESS if success, error code otherwise.
 */
static int sigi_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

/**
 * sigi_get_sys_suspend_power_state() - Platform handler to get state to suspend
 *
 * @req_state: state pointer to be filled
 *
 * This function need to to get state encodes the power domain level specific
 * local states to suspend to system affinity level.
 *
 * Return: void.
 */
static void sigi_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
}

/**
 * sigi_psci_ops - j6 platform psci ops
 */
static plat_psci_ops_t sigi_psci_ops = {
	.cpu_standby = sigi_cpu_standby,
	.pwr_domain_on = sigi_pwr_domain_on,
	.pwr_domain_off = sigi_pwr_domain_off,
	.pwr_domain_suspend = sigi_pwr_domain_suspend,
	.pwr_domain_pwr_down_wfi = sigi_pwr_down_wfi,
	.pwr_domain_on_finish = sigi_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = sigi_pwr_domain_suspend_finish,
	.system_off = sigi_system_off,
	.system_reset = sigi_system_reset,
	.validate_power_state = sigi_validate_power_state,
	.validate_ns_entrypoint = sigi_validate_ns_entrypoint,
	.get_sys_suspend_power_state = sigi_get_sys_suspend_power_state,
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
	assert(psci_ops);
    secure_entrypoint = (unsigned long) sec_entrypoint;
	*psci_ops = &sigi_psci_ops;

	return 0;
}
