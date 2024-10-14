/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#include <drivers/arm/gicv2.h>
#include <drivers/arm/gic_common.h>
#include <platform_def.h>

static const interrupt_prop_t lua_interrupt_props[] = {
#if ENABLE_FEAT_RAS && FFH_SUPPORT
	PLATFORM_RAS_G0_PROPS(GICV2_INTR_GROUP0),
#endif
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0),
#if LUA_SEC_UART1_IRQ
	PLATFORM_UART1_G0_PROPS(GICV2_INTR_GROUP0)
#endif
};

static unsigned int lua_target_masks[] = {
	BIT_32(0),
	BIT_32(1),
	BIT_32(2),
	BIT_32(3),
};

static const struct gicv2_driver_data plat_gicv2_driver_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
	.target_masks = lua_target_masks,
	.target_masks_num = ARRAY_SIZE(lua_target_masks),
	.interrupt_props = lua_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(lua_interrupt_props),
};

static inline void lua_ras_intr_configure(void)
{
	gicv2_set_spi_routing(PLAT_LUA_CPU0_ERRIRQ, 0);
	gicv2_set_spi_routing(PLAT_LUA_CPU0_FAULTIRQ, 0);

	gicv2_set_spi_routing(PLAT_LUA_CPU1_ERRIRQ, 1);
	gicv2_set_spi_routing(PLAT_LUA_CPU1_FAULTIRQ, 1);

	gicv2_set_spi_routing(PLAT_LUA_CPU2_ERRIRQ, 2);
	gicv2_set_spi_routing(PLAT_LUA_CPU2_FAULTIRQ, 2);

	gicv2_set_spi_routing(PLAT_LUA_CPU3_ERRIRQ, 3);
	gicv2_set_spi_routing(PLAT_LUA_CPU3_FAULTIRQ, 3);
}

void plat_lua_gic_init(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	gicv2_driver_init(&plat_gicv2_driver_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
#if ENABLE_FEAT_RAS && FFH_SUPPORT
	lua_ras_intr_configure();
#endif
}

void lua_pwr_gic_on_finish(void)
{
	/* TODO: This setup is needed only after a cold boot */
	gicv2_pcpu_distif_init();

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
}

void lua_pwr_gic_off(void)
{
	gicv2_cpuif_disable();
}
