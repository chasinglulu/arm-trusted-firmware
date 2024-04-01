/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

	.global	lua_calc_core_pos
	.global	plat_my_core_pos
	.global	platform_mem_init
	.global	plat_is_my_cpu_primary
	.global plat_secondary_cold_boot_setup
	.globl  plat_get_my_entrypoint

/*
 * unsigned int lua_calc_core_pos(u_register_t mpidr)
 * core_pos = (cluster_id * max_cpus_per_cluster) + core_id
 */
func lua_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, lsr #6
	ret
endfunc lua_calc_core_pos

func plat_my_core_pos
	mrs	x0, mpidr_el1
	lsr	x0, x0, #8
	b	lua_calc_core_pos
endfunc plat_my_core_pos

func platform_mem_init
	ret
endfunc platform_mem_init

/*
 * Secondary CPUs are placed in a holding pen, waiting for their mailbox
 * to be populated. Note that all CPUs share the same mailbox ; therefore,
 * populating it will release all CPUs from their holding pen. If
 * finer-grained control is needed then this should be handled in the
 * code that secondary CPUs jump to.
 */
func plat_secondary_cold_boot_setup
	/* Calculate address of our hold entry */
	bl	plat_my_core_pos
	lsl	x0, x0, #PLAT_LUA_HOLD_ENTRY_SHIFT
	mov_imm	x2, PLAT_LUA_HOLD_BASE

	/* Wait until we have a go */
poll_mailbox:
	ldr	x1, [x2, x0]
	cbz	x1, 1f

	/* Clear the mailbox again ready for next time. */
	mov x1, #PLAT_LUA_HOLD_STATE_WAIT
	str x1, [x2, x0]

	/* Jump to the provided entrypoint. */
	mov_imm	x0, PLAT_LUA_TRUSTED_MAILBOX_BASE
	ldr	x1, [x0]
	br	x1
1:
	wfe
	b	poll_mailbox
endfunc plat_secondary_cold_boot_setup

func plat_get_my_entrypoint
	/* TODO support warm boot */
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint

/*
 * Find out whether the current cpu is the primary
 * cpu (applicable only after a cold boot)
 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_LUA_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary
