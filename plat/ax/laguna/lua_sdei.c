/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

/* SDEI configuration for ARM platforms */

#include <bl31/ehf.h>
#include <common/debug.h>
#include <plat/common/platform.h>
#include <services/sdei.h>

#include <platform_def.h>

int arm_validate_ns_entrypoint(uintptr_t entrypoint)
{
	uint64_t base = BL31_BASE;
	uint64_t limit = BL31_LIMIT;

	return (entrypoint < base || entrypoint > limit) ? 0 : -1;
}

/* Private event mappings */
static sdei_ev_map_t lua_sdei_private[] = {
	SDEI_DEFINE_EVENT_0(LUA_SDEI_SGI_PRIVATE),
};

/* Shared event mappings */
static sdei_ev_map_t lua_sdei_shared[] = {
};

void plat_sdei_setup(void)
{
	INFO("SDEI platform setup\n");
}

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(lua_sdei_private, lua_sdei_shared);