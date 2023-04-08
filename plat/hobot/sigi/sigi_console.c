/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <sigi_def.h>

#include <drivers/console.h>
#include <drivers/ti/uart/uart_16550.h>

static console_t console;

void sigi_console_init(void)
{
	(void)console_16550_register(PLAT_SIGI_BOOT_UART_BASE,
			       PLAT_SIGI_BOOT_UART_CLK_IN_HZ,
			       PLAT_SIGI_CONSOLE_BAUDRATE, &console);

	console_set_scope(&console, CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME);
}
