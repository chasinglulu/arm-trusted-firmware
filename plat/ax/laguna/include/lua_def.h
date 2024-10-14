/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#ifndef LUA_DEF_H
#define LUA_DEF_H

#define PLATFORM_NAME   "Laguna Virtual Platform"

unsigned int lua_calc_core_pos(u_register_t mpidr);
void lua_console_init(void);
void plat_lua_gic_init(void);
void lua_pwr_gic_on_finish(void);
void lua_pwr_gic_off(void);

#endif /* LUA_DEF_H */