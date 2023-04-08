/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

	.global	sigi_calc_core_pos
	.globl	plat_get_my_entrypoint
	.global	plat_my_core_pos
	.global	platform_mem_init
	.global	plat_is_my_cpu_primary
	.global plat_secondary_cold_boot_setup
	.globl  plat_get_my_entrypoint

/*
 * unsigned int sigi_calc_core_pos(u_register_t mpidr)
 * core_pos = (cluster_id * max_cpus_per_cluster) + core_id
 */
func sigi_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, lsr #7
	ret
endfunc sigi_calc_core_pos

func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	sigi_calc_core_pos
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
	ldr	x0, #0

	/* Wait until the mailbox gets populated */
poll_mailbox:
	cbz	x0, 1f
	br	x0
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
	cmp	x0, #PLAT_SIGI_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary
