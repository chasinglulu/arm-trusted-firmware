/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#ifndef LUA_DEF_H
#define LUA_DEF_H

#define PLATFORM_NAME   "Laguna Virtual Platform"

/* Clock configuration */
#define LUA_OSC24M_CLK_IN_HZ       24000000

/* UART configuration */
#define LUA_UART0_BAUDRATE         115200
#define LUA_UART0_CLK_IN_HZ        LUA_OSC24M_CLK_IN_HZ

unsigned int lua_calc_core_pos(u_register_t mpidr);
void lua_console_init(void);
void plat_lua_gic_init(void);
void lua_pwr_gic_on_finish(void);
void lua_pwr_gic_off(void);

/* PMU register offsets for CPU*/
enum {
	CPU_CL0_C0_0            = 0x0214,
	CPU_CL0_C0_1            = 0x0218,
	CPU_CL0_C1_0            = 0x0220,
	CPU_CL0_C1_1            = 0x0224,
	CPU_CL0_C2_0            = 0x022c,
	CPU_CL0_C2_1            = 0x0230,
	CPU_CL0_C3_0            = 0x0238,
	CPU_CL0_C3_1            = 0x023c,
	CPU_CL1_C0_0            = 0x0240,
	CPU_CL1_C0_1            = 0x0244,
	CPU_CL1_C1_0            = 0x0248,
	CPU_CL1_C1_1            = 0x024c,
	CPU_CL1_C2_0            = 0x0250,
	CPU_CL1_C2_1            = 0x0254,
	CPU_CL1_C3_0            = 0x0258,
	CPU_CL1_C3_1            = 0x025c,
};

#define LUA_PMU_BASE        0x06000000U

#endif /* LUA_DEF_H */