/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2024, Charleye <wangkart@aliyun.com>
 * All rights reserved.
 */

#include <drivers/arm/gicv2.h>
#include <drivers/arm/gic_common.h>
#include <platform_def.h>

static const interrupt_prop_t lmt_interrupt_props[] = {
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0),
	PLATFORM_G0_PROPS(GICV2_INTR_GROUP0)
};

static const struct gicv2_driver_data plat_gicv2_driver_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
	.interrupt_props = lmt_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(lmt_interrupt_props),
};

void plat_lmt_gic_init(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	gicv2_driver_init(&plat_gicv2_driver_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

void lmt_pwr_gic_on_finish(void)
{
	/* TODO: This setup is needed only after a cold boot */
	gicv2_pcpu_distif_init();

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
}

void lmt_pwr_gic_off(void)
{
	gicv2_cpuif_disable();
}
