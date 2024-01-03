/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#include <platform_def.h>
#include <lmt_def.h>

#include <drivers/console.h>
#include <drivers/ti/uart/uart_16550.h>

static console_t console;

void lmt_console_init(void)
{
	(void)console_16550_register(PLAT_LMT_BOOT_UART_BASE,
			       PLAT_LMT_BOOT_UART_CLK_IN_HZ,
			       PLAT_LMT_CONSOLE_BAUDRATE, &console);

	console_set_scope(&console, CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME);
}
