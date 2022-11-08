// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#define LOG_TAG "INFO"
#include "ddp_info.h"
#include "ddp_debug.h"
#include "ddp_log.h"

#define DDP_MODULE_REG_RANGE	(0x1000)

static const char reg_magic[] = "no_regs_info";

struct ddp_module ddp_modules[DISP_MODULE_NUM] = {
/*
 * {module_id,
 *  module_type,
 *  module_name,
 *  can_connect,
 *  module_driver,
 *
 *  {reg_dt_name,
 *  reg_pa_check,
 *  reg_irq_check,
 *  irq_max_bit: The number of module INTEN reg enable bit
 *  reg_va,
 *  reg_irq}
 * },
 */
	{DISP_MODULE_CONFIG,
	 DISP_T_UNKNOWN,
	 "disp_config",
	 0,
	 NULL,
	 {"mediatek,dispsys_config",
	  0x14000000,
	  0,
	  0,
	  0,
	  0}
	 },

	{DISP_MODULE_MUTEX,
	 DISP_T_UNKNOWN,
	 "mutex",
	 0,
	 NULL,
	 {"mediatek,disp_mutex0",
	  0x14001000,
	  284,
	  25,
	  0,
	  0}
	 },

	{DISP_MODULE_SMI_COMMON,
	 DISP_T_UNKNOWN,
	 "disp_smi_common",
	 0,
	 NULL,
	 {"mediatek,disp_smi_common",
	  0x14002000,
	  0,
	  0,
	  0,
	  0}
	 },

	{DISP_MODULE_SMI_LARB0,
	 DISP_T_UNKNOWN,
	 "smi_larb0",
	 0,
	 NULL,
	 {"mediatek,smi_larb0",
	  0x14003000,
	  315,
	  4,
	  0,
	  0}
	 },

	{DISP_MODULE_SMI_LARB1,
	 DISP_T_UNKNOWN,
	 "smi_larb1",
	 0,
	 NULL,
	 {"mediatek,smi_larb1",
	  0x14004000,
	  314,
	  4,
	  0,
	  0}
	 },

	{DISP_MODULE_OVL0,
	 DISP_T_OVL,
	 "ovl0",
	 1,
	 &ddp_driver_ovl,
	 {"mediatek,disp_ovl0",
	  0x14005000,
	  286,
	  14,
	  0,
	  0}
	},

	{DISP_MODULE_OVL0_2L,
	 DISP_T_OVL,
	 "ovl0_2l",
	 1,
	 &ddp_driver_ovl,
	 {"mediatek,disp_ovl0_2l",
	  0x14006000,
	  287,
	  14,
	  0,
	  0}
	},

	{DISP_MODULE_RDMA0,
	 DISP_T_RDMA,
	 "rdma0",
	 1,
	 &ddp_driver_rdma,
	 {"mediatek,disp_rdma0",
	  0x14007000,
	  288,
	  7,
	  0,
	  0}
	},

	{DISP_MODULE_RSZ0,
	 DISP_T_RSZ,
	 "rsz0",
	 1,
	 &ddp_driver_rsz,
	 {"mediatek,disp_rsz0",
	  0x14008000,
	  289,
	  0,
	  0,
	  0}
	},

	{DISP_MODULE_COLOR0,
	 DISP_T_COLOR,
	 "color0",
	 1,
	 &ddp_driver_color,
	 {"mediatek,disp_color0",
	  0x14009000,
	  290,
	  0,
	  0,
	  0}
	},

	{DISP_MODULE_CCORR0,
	 DISP_T_CCORR,
	 "ccorr0",
	 1,
	 &ddp_driver_ccorr,
	 {"mediatek,disp_ccorr0",
	  0x1400a000,
	  291,
	  1,
	  0,
	  0}
	},

	{DISP_MODULE_AAL0,
	 DISP_T_AAL,
	 "aal0",
	 1,
	 &ddp_driver_aal,
	 {"mediatek,disp_aal0",
	  0x1400b000,
	  292,
	  1,
	  0,
	  0}
	},

	{DISP_MODULE_GAMMA0,
	 DISP_T_GAMMA,
	 "gamma0",
	 1,
	 &ddp_driver_gamma,
	 {"mediatek,disp_gamma0",
	  0x1400c000,
	  293,
	  0,
	  0,
	  0}
	},

	{DISP_MODULE_POSTMASK0,
	 DISP_T_POSTMASK,
	 "postmask0",
	 1,
	 &ddp_driver_postmask,
	 {"mediatek,disp_postmask0",
	  0x1400d000,
	  294,
	  12,
	  0,
	  0}
	},

	{DISP_MODULE_DITHER0,
	 DISP_T_DITHER,
	 "dither0",
	 1,
	 &ddp_driver_dither,
	 {"mediatek,disp_dither0",
	  0x1400e000,
	  295,
	  1,
	  0,
	  0}
	},

/* TODO : Add dsc wrap module */
	{DISP_MODULE_DSC_WRAP0_CORE0,
	 DISP_T_UNKNOWN,//NDISP_T_DSC_WRAP,
	 "dsc_wrap0_core0",
	 1,
	 NULL,//&ddp_driver_dsc,
	 {"mediatek,disp_dsc_wrap0_core0",
	  0x1400f000,
	  296,
	  1,
	  0,
	  0}
	},

	{DISP_MODULE_DSC_WRAP0_CORE1,
	 DISP_T_UNKNOWN,//DISP_T_DSC_WRAP,
	 "dsc_wrap0_core1",
	 1,
	 NULL,//&ddp_driver_dsc,
	 {"mediatek,disp_dsc_wrap0_core1",
	  0x1400f400,
	  296,
	  1,
	  0,
	  0}
	},

	{DISP_MODULE_DSI0,
	 DISP_T_DSI,
	 "dsi0",
	 1,
	 &ddp_driver_dsi0,
	 {"mediatek,dsi0",
	  0x14010000,
	  297,
	  15,
	  0,
	  0}
	 },

	{DISP_MODULE_WDMA0,
	 DISP_T_WDMA,
	 "wdma0",
	 1,
	 &ddp_driver_wdma,
	 {"mediatek,disp_wdma0",
	  0x14011000,
	  298,
	  2,
	  0,
	  0}
	},

	{DISP_MODULE_UFBC_WDMA0,
	 DISP_T_WDMA,
	 "ufbc_wdma0",
	 1,
	 &ddp_driver_wdma,
	 {"mediatek,disp_ufbc_wdma0",
	  0x14012000,
	  299,
	  0,
	  0,
	  0}
	},

	{DISP_MODULE_OVL2_2L,
	 DISP_T_OVL,
	 "ovl0",
	 1,
	 &ddp_driver_ovl,
	 {"mediatek,disp_ovl2_2l",
	  0x14014000,
	  300,
	  14,
	  0,
	  0}
	},

	{DISP_MODULE_RDMA4,
	 DISP_T_RDMA,
	 "rdma4",
	 1,
	 &ddp_driver_rdma,
	 {"mediatek,disp_rdma4",
	  0x14015000,
	  301,
	  7,
	  0,
	  0}
	},

	{DISP_MODULE_DPI,
	 DISP_T_UNKNOWN,//DISP_T_DPI,
	 "dpi0",
	 1,
	 NULL,//&ddp_driver_dpi,
	 {"mediatek,dpi0",
	  0x14016000,
	  302,
	  2,
	  0,
	  0}
	 },

	/* TODO : add dmdp modules */
	{DISP_MODULE_MDP_RDMA4,
	 DISP_T_UNKNOWN,//DISP_T_MDP_RDMA,
	 "mdp_rdma4",
	 1,
	 NULL,//&ddp_driver_mdp_rdma,
	 {"mediatek,mdp_rdma4",
	  0x14017000,
	  303,
	  2,
	  0,
	  0}
	},

	{DISP_MODULE_MDP_HDR4,
	 DISP_T_UNKNOWN,//DISP_T_MDP_HDR,
	 "mdp_hdr4",
	 1,
	 NULL,//&ddp_driver_mdp_hdr,
	 {"mediatek,mdp_hdr4",
	  0x14018000,
	  304,
	  3,
	  0,
	  0}
	},

	{DISP_MODULE_MDP_RSZ4,
	 DISP_T_UNKNOWN,//DISP_T_MDP_RSZ,
	 "mdp_rsz4",
	 1,
	 NULL,//&ddp_driver_mdp_rsz,
	 {"mediatek,mdp_rsz4",
	  0x14019000,
	  305,
	  8,
	  0,
	  0}
	},

	{DISP_MODULE_MDP_AAL4,
	 DISP_T_UNKNOWN,//DISP_T_MDP_AAL,
	 "mdp_aal4",
	 1,
	 NULL,//&ddp_driver_mdp_aal,
	 {"mediatek,mdp_aal4",
	  0x1401a000,
	  306,
	  1,
	  0,
	  0}
	},

	{DISP_MODULE_MDP_TDSHP4,
	 DISP_T_UNKNOWN,//DISP_T_MDP_TDSHP,
	 "mdp_tdshp4",
	 1,
	 NULL,//&ddp_driver_mdp_tdshp,
	 {"mediatek,mdp_tdshp4",
	  0x1401b000,
	  307,
	  1,
	  0,
	  0}
	},

	{DISP_MODULE_MDP_COLOR4,
	 DISP_T_UNKNOWN,//DISP_T_MDP_COLOR,
	 "mdp_color4",
	 1,
	 NULL,//&ddp_driver_mdp_color,
	 {"mediatek,mdp_color4",
	  0x1401c000,
	  308,
	  1,
	  0,
	  0}
	},

	{DISP_MODULE_Y2R0,
	 DISP_T_UNKNOWN,//DISP_T_Y2R,
	 "y2r0",
	 1,
	 NULL,//&ddp_driver_y2r,
	 {reg_magic,}
	},

	{DISP_MODULE_PWM0,
	 DISP_T_PWM,
	 "pwm0",
	 0,
	 NULL,
	 {"mediatek,disp_pwm0",
	  0x1100E000,
	  0,
	  0,
	  0,
	  0}
	 },

	{DISP_MODULE_DSIDUAL,
	 DISP_T_DSI,
	 "dsidual",
	 0,
	 NULL,
	 {reg_magic,}
	 },

	{DISP_MODULE_DSI1,
	 DISP_T_DSI,
	 "dsi1",
	 0,
	 NULL,
	 {reg_magic,}
	 },

	{DISP_MODULE_MIPI0,
	 DISP_T_UNKNOWN,
	 "mipi0",
	 0,
	 NULL,
	 {"mediatek,mipi_tx_config0",
	  0x11e50000,
	  0,
	  0,
	  0,
	  0}
	 },

	{DISP_MODULE_MIPI1,
	 DISP_T_UNKNOWN,
	 "mipi1",
	 0,
	 NULL,
	 {reg_magic,}
	 },

	{DISP_MODULE_OVL0_2L_VIRTUAL0,
	 DISP_T_UNKNOWN,
	 "ovl0_2l_virt",
	 1,
	 NULL,
	 {reg_magic,}
	},

	{DISP_MODULE_OVL0_VIRTUAL0,
	 DISP_T_UNKNOWN,
	 "ovl0_virt",
	 1,
	 NULL,
	 {reg_magic,}
	},

	{DISP_MODULE_OVL0_OVL0_2L_VIRTUAL,
	 DISP_T_UNKNOWN,
	 "ovl0_ovl0_2l_virt",
	 1,
	 NULL,
	 {reg_magic,}
	},

	{DISP_MODULE_RDMA2_VIRTUAL,
	 DISP_T_UNKNOWN,
	 "rdma2_virt",
	 1,
	 NULL,
	 {reg_magic,}
	},

	{DISP_MODULE_DMDP_TDSHP4_SOUT,
	 DISP_T_UNKNOWN,
	 "dmdp_tdshp4_sout",
	 1,
	 NULL,
	 {reg_magic,}
	},

	{DISP_MODULE_SPLIT0,
	 DISP_T_UNKNOWN,
	 "unknown",
	 0,
	 NULL,
	 {reg_magic,}
	 },

	{DISP_MODULE_UNKNOWN,
	 DISP_T_UNKNOWN,
	 "unknown",
	 0,
	 NULL,
	 {reg_magic,}
	 },
};

unsigned int is_ddp_module(enum DISP_MODULE_ENUM module)
{
	if (module >= 0 && module < DISP_MODULE_NUM)
		return 1;

	return 0;
}

unsigned int is_ddp_module_has_reg_info(enum DISP_MODULE_ENUM module)
{
	if (!is_ddp_module(module))
		return 0;

	if (strcmp(ddp_modules[module].reg_info.reg_dt_name, reg_magic))
		return 1;

	return 0;
}

const char *ddp_get_module_name(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module(module))
		return ddp_modules[module].module_name;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return "unknown";
}

unsigned int _can_connect(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module(module))
		return ddp_modules[module].can_connect;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return 0;
}

struct DDP_MODULE_DRIVER *ddp_get_module_driver(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module(module))
		return ddp_modules[module].module_driver;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return 0;
}

void ddp_set_module_driver(enum DISP_MODULE_ENUM module,
	struct DDP_MODULE_DRIVER *drv)
{
	if (!is_ddp_module(module))
		return;

	ddp_modules[module].module_driver = drv;
}

const char *ddp_get_module_dtname(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module(module))
		return ddp_modules[module].reg_info.reg_dt_name;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return "unknown";
}

unsigned int ddp_get_module_checkirq(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module_has_reg_info(module))
		return ddp_modules[module].reg_info.reg_irq_check;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return 0;
}

unsigned long ddp_get_module_pa(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module_has_reg_info(module))
		return ddp_modules[module].reg_info.reg_pa_check;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return 0;
}

unsigned int ddp_get_module_max_irq_bit(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module_has_reg_info(module))
		return ddp_modules[module].reg_info.irq_max_bit;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return 0;
}

unsigned int ddp_is_irq_enable(enum DISP_MODULE_ENUM module)
{
#if (defined(CONFIG_MTK_TEE_GP_SUPPORT) || \
	defined(CONFIG_TRUSTONIC_TEE_SUPPORT)) && \
	defined(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
	if (module == DISP_MODULE_WDMA0)
		return 0;
#endif

	if (ddp_get_module_max_irq_bit(module))
		return 1;

	return 0;
}

void ddp_module_irq_disable(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module_has_reg_info(module)) {
		ddp_modules[module].reg_info.irq_max_bit = 0;
		return;
	}

	DDPMSG("ddp_set_irq_enable: invalid module id=%d\n", module);
}

void ddp_set_module_va(enum DISP_MODULE_ENUM module, unsigned long va)
{
	if (is_ddp_module_has_reg_info(module)) {
		ddp_modules[module].reg_info.reg_va = va;
		return;
	}

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
}

unsigned long ddp_get_module_va(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module_has_reg_info(module))
		return ddp_modules[module].reg_info.reg_va;

	return 0;
}

void ddp_set_module_irq(enum DISP_MODULE_ENUM module, unsigned int irq)
{
	if (is_ddp_module_has_reg_info(module)) {
		ddp_modules[module].reg_info.reg_irq = irq;
		return;
	}

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
}

unsigned int ddp_get_module_irq(enum DISP_MODULE_ENUM module)
{
	if (is_ddp_module_has_reg_info(module))
		return ddp_modules[module].reg_info.reg_irq;

	DDPMSG("%s: invalid module id=%d\n", __func__, module);
	return 0;
}

unsigned int is_reg_addr_valid(unsigned int isVa, unsigned long addr)
{
	unsigned int i = 0;

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		if ((isVa == 1) && (addr >= ddp_get_module_va(i)) &&
			(addr < ddp_get_module_va(i) + DDP_MODULE_REG_RANGE))
			break;
		if ((isVa == 0) && (addr >= ddp_get_module_pa(i)) &&
			(addr < ddp_get_module_pa(i) + DDP_MODULE_REG_RANGE))
			break;
	}

	if (i < DISP_MODULE_NUM) {
		DDPMSG("addr valid, isVa=0x%x, addr=0x%lx, module=%s!\n",
		       isVa, addr, ddp_get_module_name(i));
		return i;
	}

	DDPERR("%s: return fail, isVa=0x%x, addr=0x%lx!\n",
		__func__, isVa, addr);
	return 0;
}

unsigned int ddp_get_module_num_by_t(enum DISP_MODULE_TYPE_ENUM module_t)
{
	int i;
	int cnt = 0;

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		if (ddp_modules[i].module_type == module_t)
			cnt++;
	}

	return cnt;
}

enum DISP_MODULE_ENUM ddp_get_module_id_by_idx(
	enum DISP_MODULE_TYPE_ENUM module_t, unsigned int idx)
{
	int i;
	int index = 0;

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		if (ddp_modules[i].module_type == module_t)
			index++;
		if (index == (idx + 1))
			return i;
	}

	return DISP_MODULE_UNKNOWN;
}

enum DISP_MODULE_ENUM disp_irq_to_module(unsigned int irq)
{
	int i;

	if (irq == 0)
		return DISP_MODULE_UNKNOWN;

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		if (irq == ddp_get_module_irq(i))
			return i;
	}

	DDPERR("cannot find module for irq %d\n", irq);
	WARN_ON(1);
	return DISP_MODULE_UNKNOWN;
}

const char *ddp_get_ioctl_name(enum DDP_IOCTL_NAME ioctl)
{
	switch (ioctl) {
	case DDP_SWITCH_DSI_MODE:
		return "DDP_SWITCH_DSI_MODE";
	case DDP_STOP_VIDEO_MODE:
		return "DDP_STOP_VIDEO_MODE";
	case DDP_BACK_LIGHT:
		return "DDP_BACK_LIGHT";
	case DDP_SWITCH_LCM_MODE:
		return "DDP_SWITCH_LCM_MODE";
	case DDP_DPI_FACTORY_TEST:
		return "DDP_DPI_FACTORY_TEST";
	case DDP_DSI_IDLE_CLK_CLOSED:
		return "DDP_DSI_IDLE_CLK_CLOSED";
	case DDP_DSI_IDLE_CLK_OPEN:
		return "DDP_DSI_IDLE_CLK_OPEN";
	case DDP_DSI_PORCH_CHANGE:
		return "DDP_DSI_PORCH_CHANGE";
	case DDP_PHY_CLK_CHANGE:
		return "DDP_PHY_CLK_CHANGE";
	case DDP_ENTER_ULPS:
		return "DDP_ENTER_ULPS";
	case DDP_EXIT_ULPS:
		return "DDP_EXIT_ULPS";
	case DDP_RDMA_GOLDEN_SETTING:
		return "DDP_RDMA_GOLDEN_SETTING";
	case DDP_OVL_GOLDEN_SETTING:
		return "DDP_OVL_GOLDEN_SETTING";
	case DDP_PARTIAL_UPDATE:
		return "DDP_PARTIAL_UPDATE";
	case DDP_UPDATE_PLL_CLK_ONLY:
		return "DDP_UPDATE_PLL_CLK_ONLY";
	case DDP_DSI_SW_INIT:
		return "DDP_DSI_SW_INIT";
	case DDP_DSI_MIPI_POWER_ON:
		return "DDP_DSI_MIPI_POWER_ON";
	default:
		return "unknown";
	}
}
