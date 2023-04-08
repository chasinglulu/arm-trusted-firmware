/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SIGI_DEF_H
#define SIGI_DEF_H

#include <stdint.h>

#define PLATFORM_NAME   "Horizon Robotics Journey 6 platform"

/* Clock configuration */
#define SIGI_OSC24M_CLK_IN_HZ		24000000

/* UART configuration */
#define SIGI_UART0_BAUDRATE         115200
#define SIGI_UART0_CLK_IN_HZ        SIGI_OSC24M_CLK_IN_HZ

unsigned int sigi_calc_core_pos(u_register_t mpidr);
void sigi_console_init(void);
void plat_sigi_gic_init(void);

#endif /* SIGI_DEF_H */