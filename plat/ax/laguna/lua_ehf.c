/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#include <bl31/ehf.h>

#include <platform_def.h>

ehf_pri_desc_t lua_exceptions[] = {
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_LUA_UART1_PRIO),
};

/* Plug in ARM exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(lua_exceptions, ARRAY_SIZE(lua_exceptions), PLAT_PRI_BITS);
