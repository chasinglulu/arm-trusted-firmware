/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#include <assert.h>

#include <platform_def.h>
#include <lua_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <plat/arm/common/plat_arm.h>
#include <lib/extensions/ras.h>

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_lua_mmap[] = {
	MAP_REGION_FLAT(DEVICE_BASE, DEVICE_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(GICD_BASE, GICD_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CPU_SYSCTL_BASE, CPU_SYSCTL_SIZE, MT_DEVICE | MT_RW | MT_NS),
	PLAT_LUA_BOOT_MMAP,
	{ 0 }
};

const mmap_region_t *plat_lua_get_mmap(void)
{
	return plat_lua_mmap;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return LUA_OSC24M_CLK_IN_HZ;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	//assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE) ? \
						&bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*
 * Set the build time defaults,if we can't find any config data.
 */
static inline void bl31_set_default_config(void)
{
	bl32_image_ep_info.pc = (uintptr_t)BL32_BASE;
	bl32_image_ep_info.spsr = (uint32_t)arm_get_spsr_for_bl32_entry();
	bl33_image_ep_info.pc = (uintptr_t)plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = (uint32_t)SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}

static void lua_print_platform_name(void)
{
	NOTICE("ATF running on %s\n", PLATFORM_NAME);
}

void lua_config_setup(void)
{
	lua_print_platform_name();

	generic_delay_timer_init();
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 * BL2 has flushed this information to memory, so we are guaranteed to pick up
 * good data.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	lua_console_init();

	/* Initialize the platform config for future decision making */
	lua_config_setup();

	/*
	 * Do initial security configuration to allow DRAM/device access. On
	 * Base Sigi only DRAM security is programmable (via TrustZone), but
	 * other platforms might have more programmable security devices
	 * present.
	 */

	/* Populate common information for BL32 and BL33 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	bl31_set_default_config();

	VERBOSE("BL31: early platform setup\n");
	NOTICE("BL31: Secure code at 0x%08lx\n", bl32_image_ep_info.pc);
	NOTICE("BL31: Non secure code at 0x%08lx\n", bl33_image_ep_info.pc);
}

#ifdef LUA_SEC_UART1_IRQ
static int uart1_interrupt_handler(uint32_t intr_raw, uint32_t flags,
		void *handle, void *cookie)
{
	NOTICE("An UART1 interrupt is taken from non-secure EL1/EL2 to EL3.\n");

	/* Disable all interrupts */
	mmio_write_32(PLAT_LUA_UART1_BASE + UARTIER, 0x0);

	plat_ic_end_of_interrupt(intr_raw);

	return 0;
}
#endif

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	plat_lua_gic_init();

#ifdef LUA_SEC_UART1_IRQ
	ehf_register_priority_handler(PLAT_LUA_UART1_PRIO,
				uart1_interrupt_handler);
#endif

#if ENABLE_FEAT_RAS && FFH_SUPPORT
	ras_init();
#endif
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
				MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE, MT_RO_DATA| MT_SECURE),
		MAP_REGION_FLAT(SHARED_RAM_BASE, SHARED_RAM_SIZE, MT_MEMORY | MT_RW | MT_SECURE),
		{0}
	};
	setup_page_tables(bl_regions, plat_lua_get_mmap());
	enable_mmu_el3(0);
}
