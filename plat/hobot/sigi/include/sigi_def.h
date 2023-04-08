/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SIGI_DEF_H
#define SIGI_DEF_H

#include <stdint.h>

#define PLATFORM_NAME   "hobot sigi virt platform"

/* Clock configuration */
#define SIGI_OSC24M_CLK_IN_HZ		24000000

/* UART configuration */
#define SIGI_UART0_BAUDRATE         115200
#define SIGI_UART0_CLK_IN_HZ        SIGI_OSC24M_CLK_IN_HZ

unsigned int sigi_calc_core_pos(u_register_t mpidr);
void sigi_console_init(void);
void plat_sigi_gic_init(void);
void sigi_pwr_gic_on_finish(void);
void sigi_pwr_gic_off(void);

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
};

#define SIGI_PMU_BASE	0x23190000U

#endif /* SIGI_DEF_H */