#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright (C) 2024, Charleye <wangkart@aliyun.com>
# All rights reserved.
#

override RESET_TO_BL31	:= 1
override CTX_INCLUDE_AARCH32_REGS := 0
override USE_COHERENT_MEM		:= 0
override SEPARATE_CODE_AND_RODATA	:= 1
override HW_ASSISTED_COHERENCY := 1

# Enable exception handling at EL3
EL3_EXCEPTION_HANDLING	:= 1
GICV2_G0_FOR_EL3	:= 1

# RAS options
ENABLE_FEAT_RAS	:= 1
HANDLE_EA_EL3_FIRST_NS	:= 1

CRASH_REPORTING	:= 1
ENABLE_ASSERTIONS	:= 1

AX_PLAT        := plat/ax
AX_PLAT_SOC    := ${AX_PLAT}/${PLAT}

PLAT_INCLUDES  := -I${AX_PLAT_SOC}/include

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk
include lib/xlat_tables_v2/xlat_tables.mk

LUA_GIC_SOURCES := ${GICV2_SOURCES}

PLAT_BL_COMMON_SOURCES := common/desc_image_load.c            \
				lib/bl_aux_params/bl_aux_params.c             \
				plat/common/plat_gicv2.c                      \
				plat/common/plat_psci_common.c                \
				plat/arm/common/arm_common.c                  \
				${AX_PLAT_SOC}/plat_helper.S                  \
				${XLAT_TABLES_LIB_SRCS}

BL31_SOURCES += ${LUA_GIC_SOURCES}                        \
			drivers/ti/uart/aarch64/16550_console.S       \
			drivers/delay_timer/delay_timer.c             \
			drivers/delay_timer/generic_delay_timer.c     \
			lib/cpus/aarch64/cortex_a55.S                 \
			${AX_PLAT_SOC}/bl31_plat_setup.c              \
			${AX_PLAT_SOC}/lua_topology.c                 \
			${AX_PLAT_SOC}/lua_pm.c                       \
			${AX_PLAT_SOC}/lua_console.c                  \
			${AX_PLAT_SOC}/lua_gic.c                      \
			${AX_PLAT_SOC}/lua_ehf.c                      \
			${AX_PLAT_SOC}/lua_ras.c

# RAS sources
ifeq (${ENABLE_FEAT_RAS}-${HANDLE_EA_EL3_FIRST_NS},1-1)
BL31_SOURCES += lib/extensions/ras/std_err_record.c       \
				lib/extensions/ras/ras_common.c
endif

include lib/libfdt/libfdt.mk