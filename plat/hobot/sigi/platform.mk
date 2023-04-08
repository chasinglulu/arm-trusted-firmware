#
# Copyright (c) 2016-2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#override PROGRAMMABLE_RESET_ADDRESS := 1
override RESET_TO_BL31	:= 1
override CTX_INCLUDE_AARCH32_REGS := 0
override WORKAROUND_CVE_2022_23960 := 1
override ERRATA_A78_AE_1941500 := 1
override ERRATA_A78_AE_1951502 := 1
override ERRATA_A78_AE_2376748 := 1
override ERRATA_A78_AE_2395408 := 1
override USE_COHERENT_MEM		:= 0
override SEPARATE_CODE_AND_RODATA	:= 1
override HW_ASSISTED_COHERENCY := 1

HOBOT_PLAT		:=	plat/hobot
HOBOT_PLAT_SOC	:=	${HOBOT_PLAT}/${PLAT}

PLAT_INCLUDES		:=	-I${HOBOT_PLAT_SOC}/include

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

SIGI_GIC_SOURCES	:=	${GICV3_SOURCES}

PLAT_BL_COMMON_SOURCES	:=	common/desc_image_load.c			\
				lib/bl_aux_params/bl_aux_params.c		\
				plat/common/aarch64/crash_console_helpers.S \
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c	\
				plat/arm/common/arm_common.c	\
				${HOBOT_PLAT_SOC}/plat_helper.S	\
				${XLAT_TABLES_LIB_SRCS}

BL31_SOURCES	+=	${SIGI_GIC_SOURCES}					\
			drivers/ti/uart/aarch64/16550_console.S		\
			drivers/delay_timer/delay_timer.c			\
			drivers/delay_timer/generic_delay_timer.c	\
			lib/cpus/aarch64/cortex_a78_ae.S			\
			${HOBOT_PLAT_SOC}/bl31_plat_setup.c			\
			${HOBOT_PLAT_SOC}/sigi_topology.c			\
			${HOBOT_PLAT_SOC}/sigi_pm.c					\
			${HOBOT_PLAT_SOC}/sigi_console.c			\
			${HOBOT_PLAT_SOC}/sigi_gicv3.c

include lib/libfdt/libfdt.mk