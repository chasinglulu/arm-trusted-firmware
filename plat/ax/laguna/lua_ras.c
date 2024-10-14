/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */
#include <lib/extensions/ras.h>
#include <services/sdei.h>
#include <plat/common/platform.h>
#include <lib/libc/aarch64/inttypes_.h>

static int injected_fault_handler(const struct err_record_info *info,
		int probe_data, const struct err_handler_data *const data)
{
	uint64_t status;
	uint64_t misc0;
	uint32_t intr;
	// int ret;

	intr = data->interrupt;

	NOTICE("ERXMISC0_EL1 = 0x%lx\n", read_erxmisc0_el1());
	NOTICE("ERXSTATUS_EL1 = 0x%lx\n", read_erxstatus_el1());
	NOTICE("Raw Interrupt: %d\n", intr);
	/*
	 * The faulting error record is already selected by the SER probe
	 * function.
	 */
	status = read_erxstatus_el1();
	/* Clear error */
	write_erxstatus_el1(status);
	isb();
	dsb();
	NOTICE("[Cleared] ERXMISC0_EL1 = 0x%lx\n", read_erxmisc0_el1());
	NOTICE("[Cleared] ERXSTATUS_EL1 = 0x%lx\n", read_erxstatus_el1());

	ERROR("Fault reported by system error record %d on 0x%lx: status=0x%" PRIx64 "\n",
			probe_data, read_mpidr_el1(), status);
	ERROR(" exception reason=%u syndrome=0x%" PRIx64 "\n", data->ea_reason,
			data->flags);

	misc0 = read_erxmisc0_el1();

	/* Clear Other Error Count Overflow */
	if (misc0 & BIT(47)) {
		misc0 &= ~BIT(47);
		misc0 &= ~GENMASK_64(46, 40);
		write_erxmisc0_el1(misc0);
	}

	/* Clear Repeat Error Count Overflow */
	if (misc0 & BIT(39)) {
		misc0 &= ~BIT(39);
		misc0 &= ~GENMASK_64(38, 32);
		write_erxmisc0_el1(misc0);
	}

	/* Clear errors again
	 * It might clear more than one error(CE and UC)
	 * at the same time.
	 */
	status = read_erxstatus_el1();
	write_erxstatus_el1(status);
	plat_ic_end_of_interrupt(intr);

	// ret = sdei_dispatch_event(5000);
	// if (ret < 0) {
	// 	ERROR("Can't dispatch event to SDEI\n");
	// 	panic();
	// } else {
	// 	INFO("SDEI event dispatched\n");
	// }

	return 0;
}

void plat_handle_uncontainable_ea(void)
{
	/* Do not change the string, CI expects it. Wait forever */
	INFO("Injected Uncontainable Error\n");
	while (true) {
		wfe();
	}
}

struct err_record_info lua_err_records[] = {
	/* Record for injected fault */
	ERR_RECORD_SYSREG_V1(0, 2, ras_err_ser_probe_sysreg,
			injected_fault_handler, NULL),
};
REGISTER_ERR_RECORD_INFO(lua_err_records);

struct ras_interrupt lua_ras_interrupts[] = {
	{
		.intr_number = PLAT_LUA_DSU_ERRIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_DSU_FAULTIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU0_ERRIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU1_ERRIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU2_ERRIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU3_ERRIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU0_FAULTIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU1_FAULTIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU2_FAULTIRQ,
		.err_record = &lua_err_records[0],
	},
	{
		.intr_number = PLAT_LUA_CPU3_FAULTIRQ,
		.err_record = &lua_err_records[0],
	},
};
REGISTER_RAS_INTERRUPTS(lua_ras_interrupts);
