/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <plat/common/common_def.h>

/* CPU topology */
#define PLAT_MAX_CORES_PER_CLUSTER    U(4)
#define PLAT_CLUSTER_COUNT            U(1)
#define PLATFORM_CORE_COUNT           (PLAT_CLUSTER_COUNT * PLAT_MAX_CORES_PER_CLUSTER)

#define PLAT_MAX_PWR_LVL        U(1)
#define PLAT_MAX_RET_STATE      U(1)
#define PLAT_MAX_OFF_STATE      U(2)

/* Local power state for power domains in Run state. */
#define LUA_LOCAL_STATE_RUN     U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define LUA_LOCAL_STATE_RET     U(1)
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define LUA_LOCAL_STATE_OFF     U(2)

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define LUA_LOCAL_PSTATE_WIDTH      U(4)
#define LUA_LOCAL_PSTATE_MASK       ((1 << LUA_LOCAL_PSTATE_WIDTH) - 1)

#define CACHE_WRITEBACK_SHIFT       U(6)
#define CACHE_WRITEBACK_GRANULE     (1 << CACHE_WRITEBACK_SHIFT)

/* xlat table v2 related to contants */
#define PLAT_PHY_ADDR_SPACE_SIZE    (1ULL << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE   (1ULL << 40)
#define MAX_XLAT_TABLES             U(8)
#define MAX_MMAP_REGIONS            U(8)

#define PLATFORM_STACK_SIZE         (1UL << 12)

/* physical memory related constants */
#define LUA_DRAM_BASE               ULL(0x100000000)
#define LUA_NS_DDR_SIZE             (ULL(0x10) * SZ_1G)
#define LUA_BL31_IMG_OFFSET         0x00104000
#define LUA_BL32_IMG_OFFSET         0x04000000
#define LUA_BL33_IMG_OFFSET         0x00200000

#define LUA_IRAM_BASE                0x00400000
#define LUA_IRAM_SIZE                SZ_64K

#define SHARED_RAM_BASE         (LUA_IRAM_BASE + LUA_IRAM_SIZE - SZ_4K)
#define SHARED_RAM_SIZE         SZ_4K

#define PLAT_LUA_TRUSTED_MAILBOX_BASE    SHARED_RAM_BASE
#define PLAT_LUA_TRUSTED_MAILBOX_SIZE    (8 + PLAT_LUA_HOLD_SIZE)
#define PLAT_LUA_HOLD_BASE               (PLAT_LUA_TRUSTED_MAILBOX_BASE + 8)
#define PLAT_LUA_HOLD_SIZE               (PLATFORM_CORE_COUNT * PLAT_LUA_HOLD_ENTRY_SIZE)
#define PLAT_LUA_HOLD_ENTRY_SHIFT        U(3)
#define PLAT_LUA_HOLD_ENTRY_SIZE         (1 << PLAT_LUA_HOLD_ENTRY_SHIFT)
#define PLAT_LUA_HOLD_STATE_WAIT         U(0)
#define PLAT_LUA_HOLD_STATE_GO           U(1)

/*
 * BL3-1 specific defines.
 *
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_BASE           (LUA_DRAM_BASE + LUA_BL31_IMG_OFFSET)
#define BL31_SIZE           SZ_256K
#define BL31_LIMIT          (BL31_BASE + BL31_SIZE)

#define BL32_BASE           (LUA_DRAM_BASE + LUA_BL32_IMG_OFFSET)
#define BL32_SIZE           SZ_32M
#define BL32_LIMIT          (BL32_BASE + BL32_SIZE)

/*******************************************************************************
 * BL33 specific defines.
 ******************************************************************************/
#ifndef PRELOADED_BL33_BASE
# define PLAT_ARM_NS_IMAGE_BASE  U(LUA_DRAM_BASE + LUA_BL33_IMG_OFFSET)
#else
# define PLAT_ARM_NS_IMAGE_BASE  U(PRELOADED_BL33_BASE)
#endif

/* Clock configuration */
#ifdef LUA_FPGA
#define LUA_OSC24M_CLK_IN_HZ       10000000
#else
#define LUA_OSC24M_CLK_IN_HZ       24000000
#endif

/* UART configuration */
#define LUA_UART_BAUDRATE         115200
#define LUA_UART_CLK_IN_HZ        LUA_OSC24M_CLK_IN_HZ

/*
 * UART related constants
 */
#define PLAT_PRI_BITS                   U(3)

#define PLAT_LUA_UART0_BASE             0x00602000
#define PLAT_LUA_UART0_SIZE             ULL(0x1000)
#define PLAT_LUA_UART0_MMAP             MAP_REGION_FLAT(PLAT_LUA_UART0_BASE,   \
											PLAT_LUA_UART0_SIZE,               \
											MT_DEVICE | MT_RW |                \
											MT_NS | MT_PRIVILEGED)

#define PLAT_LUA_UART1_PRIO             U(0x40)
#define PLAT_LUA_UART1_IRQ              U(196)
#define PLAT_LUA_UART1_BASE             0x0E403000
#define PLAT_LUA_UART1_SIZE             ULL(0x1000)
#define PLAT_LUA_UART1_MMAP             MAP_REGION_FLAT(PLAT_LUA_UART1_BASE,   \
											PLAT_LUA_UART1_SIZE,               \
											MT_DEVICE | MT_RW |                \
											MT_NS | MT_PRIVILEGED)

#ifdef LUA_UART0_CONSOLE
#define PLAT_LUA_BOOT_UART_BASE         PLAT_LUA_UART0_BASE
#define PLAT_LUA_BOOT_MMAP              PLAT_LUA_UART0_MMAP
#else
#define PLAT_LUA_BOOT_UART_BASE         PLAT_LUA_UART1_BASE
#define PLAT_LUA_BOOT_MMAP              PLAT_LUA_UART1_MMAP
#endif
#define PLAT_LUA_BOOT_UART_CLK_IN_HZ    LUA_UART_CLK_IN_HZ
#define PLAT_LUA_CONSOLE_BAUDRATE       LUA_UART_BAUDRATE

#define DEVICE_BASE            0x04000000
#define DEVICE_SIZE            SZ_512M

/*
 * GIC related constants
 */
#define GICD_BASE               0x08001000
#define GICC_BASE               0x08002000

#define LUA_IRQ_SEC_SGI_0       8
#define LUA_IRQ_SEC_SGI_1       9
#define LUA_IRQ_SEC_SGI_2       10
#define LUA_IRQ_SEC_SGI_3       11
#define LUA_IRQ_SEC_SGI_4       12
#define LUA_IRQ_SEC_SGI_5       13
#define LUA_IRQ_SEC_SGI_6       14
#define LUA_IRQ_SEC_SGI_7       15

#define PLATFORM_G1S_PROPS(grp)                                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE),                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE),                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE),                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE),                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE),                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE),                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE),                     \
	INTR_PROP_DESC(LUA_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,     \
					   grp, GIC_INTR_CFG_EDGE)

#define PLATFORM_UART1_G0_PROPS(grp)                                \
	INTR_PROP_DESC(PLAT_LUA_UART1_IRQ, PLAT_LUA_UART1_PRIO,         \
	                   grp, GIC_INTR_CFG_LEVEL)

#define PLAT_LUA_PRIMARY_CPU                  0x0
#define PLAT_LUA_PRIMARY_CPU_SHIFT            8
#define PLAT_LUA_PRIMARY_CPU_BIT_WIDTH        6

#endif /* PLATFORM_DEF_H */
