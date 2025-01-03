/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#define LOG_TAG "ORADT"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#include <mt-plat/mt_gpio.h>
#include <mt-plat/upmu_common.h>
#include <mach/gpio_const.h>
/*#include <mach/mt_pm_ldo.h>*/
#else
#include <mt-plat/mt_gpio.h>
#include <mt-plat/upmu_common.h>
#include <mach/gpio_const.h>
#endif

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_info("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_info("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

static const unsigned int BL_MIN_LEVEL = 20;
static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define MDELAY(n)       (lcm_util.mdelay(n))

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */

#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
        lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
        lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
      lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
        lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static unsigned int need_set_lcm_addr = 1;

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
/* #include <linux/jiffies.h> */
/* #include <linux/delay.h> */
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>

/*****************************************************************************
 * Define
 *****************************************************************************/
#define I2C_I2C_LCD_BIAS_CHANNEL 0
#define I2C_ID_NAME "tps65132"
#define TPS_ADDR 0x3E
/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
static struct i2c_board_info tps65132_board_info __initdata = { I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR) };

static struct i2c_client *tps65132_i2c_client;

/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tps65132_remove(struct i2c_client *client);
/*****************************************************************************
 * Data Structure
 *****************************************************************************/

struct tps65132_dev {
	struct i2c_client *client;

};

static const struct i2c_device_id tps65132_id[] = {
	{I2C_ID_NAME, 0},
	{}
};

/* #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)) */
/* static struct i2c_client_address_data addr_data = { .forces = forces,}; */
/* #endif */
static struct i2c_driver tps65132_iic_driver = {
	.id_table = tps65132_id,
	.probe = tps65132_probe,
	.remove = tps65132_remove,
	/* .detect               = mt6605_detect, */
	.driver = {
		.owner = THIS_MODULE,
		.name = "tps65132",
	},
};

/*****************************************************************************
 * Function
 *****************************************************************************/
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	LCM_LOGI("tps65132_iic_probe\n");
	LCM_LOGI("TPS: info==>name=%s addr=0x%x\n", client->name, client->addr);
	tps65132_i2c_client = client;
	return 0;
}

static int tps65132_remove(struct i2c_client *client)
{
	LCM_LOGI("tps65132_remove\n");
	tps65132_i2c_client = NULL;
	i2c_unregister_device(client);
	return 0;
}
/*
static int tps65132_write_bytes(unsigned char addr, unsigned char value)
{
	int ret = 0;
	struct i2c_client *client = tps65132_i2c_client;
	char write_data[2] = { 0 };
	write_data[0] = addr;
	write_data[1] = value;
	ret = i2c_master_send(client, write_data, 2);
	if (ret < 0)
		LCM_LOGI("tps65132 write data fail !!\n");
	return ret;
}*/

#define TPS_I2C_BUSNUM  I2C_I2C_LCD_BIAS_CHANNEL

static int __init tps65132_iic_init(void)
{
	LCM_LOGI("tps65132_iic_init\n");
	i2c_register_board_info(TPS_I2C_BUSNUM, &tps65132_board_info, 1);
	LCM_LOGI("tps65132_iic_init2\n");
	i2c_add_driver(&tps65132_iic_driver);
	LCM_LOGI("tps65132_iic_init success\n");
	return 0;
}

static void __exit tps65132_iic_exit(void)
{
	LCM_LOGI("tps65132_iic_exit\n");
	i2c_del_driver(&tps65132_iic_driver);
}


module_init(tps65132_iic_init);
module_exit(tps65132_iic_exit);

MODULE_AUTHOR("Xiaokuan Shi");
MODULE_DESCRIPTION("MTK TPS65132 I2C Driver");
#endif

/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define LCM_DSI_CMD_MODE                                    1
#define FRAME_WIDTH                                     (1200)
#define FRAME_HEIGHT                                    (720)
#define LCM_ID (144)
#define REGFLAG_DELAY       0xFE
#define REGFLAG_END_OF_TABLE    0xFD

static LCM_DSI_MODE_SWITCH_CMD lcm_switch_mode_cmd;

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
        {0x28, 1, {0x00}},
        {0x10, 1, {0x00}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_vdo_initialization_setting[] = {
        {0xff, 3, {0x78, 0x10, 0x06}},
        {0x45, 1, {0x00}},
        {0xff, 3, {0x78, 0x10, 0x05}},
        {0x01, 1, {0x10}},
        {0x02, 1, {0x00}},
        {0xff, 3, {0x78, 0x10, 0x03}},
        {0x83, 1, {0x20}},
        {0x84, 1, {0x00}},
        {0xff, 3, {0x78, 0x10, 0x00}},
        {0x35, 1, {0x00}},
        {0x36, 1, {0x00}},
        {0x2a, 4, {0x00, 0x00, 0x04, 0xaf}},
        {0x2b, 4, {0x00, 0x00, 0x02, 0xcf}},
        {0x11, 1, {0x00}},
        {REGFLAG_DELAY, 120, {}},
        {0x29, 1, {0x00}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_off_setting[] = {
        {0x55, 1, {0x00}},
        {0x53, 1, {0x00}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};
/*
static struct LCM_setting_table lcm_backlight_mode_setting[] = {
        {0x51, 2, {0x0f, 0xff}},
        {0x53, 1, {0x24}},
        {0x55, 1, {0x01}},
        {0x5e, 2, {0x00, 0x00}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};*/

static struct LCM_setting_table lcm_backlight_level_setting[] = {
        {0x51, 2, {0x0f, 0xff}},
        {0x53, 1, {0x24}},
        {0x55, 1, {0x01}},
        {0x5e, 2, {0x00, 0x00}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(void *cmdq, struct LCM_setting_table *table,
        unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {

			case REGFLAG_DELAY:
				MDELAY(table[i].count);
				break;

			case REGFLAG_END_OF_TABLE:
				break;

			default:
				dsi_set_cmdq_V22(cmdq, cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_THREE_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size = 256;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.vertical_sync_active = 3;
	params->dsi.vertical_backporch = 5;
	params->dsi.vertical_frontporch = 10;
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 6;
	params->dsi.horizontal_backporch = 10;
	params->dsi.horizontal_frontporch = 10;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.ssc_disable = 1;
	params->dsi.PLL_CLOCK = 420;	/* this value must be in MTK suggested table */
	params->dsi.CLK_HS_POST = 26;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 0;
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	params->dsi.lcm_ext_te_monitor = 1;
}

#ifdef BUILD_LK
#define TPS65132_SLAVE_ADDR_WRITE  0x7C
static struct mt_i2c_t TPS65132_i2c;
#define I2C_I2C_LCD_BIAS_CHANNEL 0
static int TPS65132_write_byte(kal_uint8 addr, kal_uint8 value)
{
	kal_uint32 ret_code = I2C_OK;
	kal_uint8 write_data[2];
	kal_uint16 len;

	write_data[0] = addr;
	write_data[1] = value;

	TPS65132_i2c.id = I2C_I2C_LCD_BIAS_CHANNEL; /* I2C2; */
	/* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
	TPS65132_i2c.addr = (TPS65132_SLAVE_ADDR_WRITE >> 1);
	TPS65132_i2c.mode = ST_MODE;
	TPS65132_i2c.speed = 100;
	len = 2;

	ret_code = i2c_write(&TPS65132_i2c, write_data, len);
	/* printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code); */

	return ret_code;
}
#endif

/*
static void lcm_power_off(void)
{
	pmic_set_register_value(PMIC_RG_VGP3_EN, 0);
        MDELAY(10);
        mt_set_gpio_mode(GPIO3,0);
        mt_set_gpio_dir(GPIO3,1);
        mt_set_gpio_out(GPIO3,0);
        MDELAY(2);
	pmic_set_register_value(PMIC_RG_VGP2_EN, 0);
        mt_set_gpio_mode(GPIO1,0);
        mt_set_gpio_dir(GPIO1,1);
        mt_set_gpio_out(GPIO1,0);
}*/

static void lcm_power_on(void)
{
	pmic_set_register_value(0x3f1,3);
	pmic_set_register_value(0x38f,1);
        mt_set_gpio_mode(GPIO1,0);
        mt_set_gpio_dir(GPIO1,1);
        mt_set_gpio_out(GPIO1,1);
        MDELAY(2);
        mt_set_gpio_mode(GPIO3,0);
        mt_set_gpio_dir(GPIO3,1);
        mt_set_gpio_out(GPIO3,1);
        MDELAY(4);
	pmic_set_register_value(0x3f6,3);
	pmic_set_register_value(0x394,1);
        mt_set_gpio_mode(GPIO2,0);
        mt_set_gpio_dir(GPIO2,1);
        mt_set_gpio_out(GPIO2,1);
        MDELAY(1);
        mt_set_gpio_mode(GPIO46,0);
        mt_set_gpio_dir(GPIO46,1);
        mt_set_gpio_out(GPIO46,1);
        MDELAY(1);
        mt_set_gpio_out(GPIO46,0);
        MDELAY(1);
        mt_set_gpio_out(GPIO46,1);
        MDELAY(10);
}
static void lcm_init_power(void)
{
	lcm_power_on();
}

static void lcm_init(void)
{
	LCM_LOGI("LCM_INIT++");
	lcm_power_on();
        push_table(0, lcm_vdo_initialization_setting, sizeof(lcm_vdo_initialization_setting) / sizeof(struct LCM_setting_table), 1);
        need_set_lcm_addr = 1;
	LCM_LOGI("LCM_INIT--");
}

static void lcm_suspend(void)
{
	push_table(0, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	mt_set_gpio_mode(GPIO46,0);
	mt_set_gpio_dir(GPIO46,1);
	mt_set_gpio_out(GPIO46,0);
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
        unsigned int x0 = x;
        unsigned int y0 = y;
        unsigned int x1 = x0 + width - 1;
        unsigned int y1 = y0 + height - 1;

        unsigned char x0_MSB = ((x0>>8)&0xFF);
        unsigned char x0_LSB = (x0&0xFF);
        unsigned char x1_MSB = ((x1>>8)&0xFF);
        unsigned char x1_LSB = (x1&0xFF);
        unsigned char y0_MSB = ((y0>>8)&0xFF);
        unsigned char y0_LSB = (y0&0xFF);
        unsigned char y1_MSB = ((y1>>8)&0xFF);
        unsigned char y1_LSB = (y1&0xFF);

        unsigned int data_array[16];

        // need update at the first time
        if(need_set_lcm_addr)
        {
                data_array[0]= 0x00053902;
                data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
                data_array[2]= (x1_LSB);
                dsi_set_cmdq(data_array, 3, 1);

                data_array[0]= 0x00053902;
                data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
                data_array[2]= (y1_LSB);
                dsi_set_cmdq(data_array, 3, 1);
                need_set_lcm_addr = 0;
        }

        data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
        LCM_LOGI("[LCD] lcm_update \n");
}

static unsigned int lcm_compare_id(void)
{
/*	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[17];

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(20);//Must over 6 ms

	array[0]=0x00023700;
	dsi_set_cmdq(&array, 1, 1);
//	MDELAY(10);
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0];
	LCM_LOGI("%s, id = 0x%08x\n", __func__, id);*/
	return 1;
}


/* return TRUE: need recovery */
/* return FALSE: No need recovery */
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
	char buffer[3];
	int array[4];

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x53, buffer, 1);

	if (buffer[0] != 0x24) {
		LCM_LOGI("[LCM ERROR] [0x53]=0x%02x\n", buffer[0]);
		return 1;
	} else {
		LCM_LOGI("[LCM NORMAL] [0x53]=0x%02x\n", buffer[0]);
		return 0;
	}
#else
	return 0;
#endif

}

static void lcm_setbacklight(void *handle, unsigned int level)
{
	LCM_LOGI("%s,oradt backlight: level = %d\n", __func__, level);
	if (level == 1000) { // ???
		lcm_backlight_level_setting[0].para_list[0] = 0x0F;
		lcm_backlight_level_setting[0].para_list[1] = 0xFF;
		push_table(handle, lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
	} else if (level == 0) {
		push_table(handle, lcm_backlight_off_setting, sizeof(lcm_backlight_off_setting) / sizeof(struct LCM_setting_table), 1);
	} else if (level < 256) {
		unsigned int esd_level = (level * 0xFFF) / 0xFF;
		lcm_backlight_level_setting[0].para_list[0] = (esd_level >> 8) & 0xFF;
		lcm_backlight_level_setting[0].para_list[1] = esd_level & 0xFF;
		push_table(handle, lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
	}
}

static void *lcm_switch_mode(int mode)
{
#ifndef BUILD_LK
	/* customization: 1. V2C config 2 values, C2V config 1 value; 2. config mode control register */
	if (mode == 0) {    /* V2C */
		lcm_switch_mode_cmd.mode = CMD_MODE;
		lcm_switch_mode_cmd.addr = 0xBB;    /* mode control addr */
		lcm_switch_mode_cmd.val[0] = 0x13;  /* enabel GRAM firstly, ensure writing one frame to GRAM */
		lcm_switch_mode_cmd.val[1] = 0x10;  /* disable video mode secondly */
	} else {        /* C2V */
		lcm_switch_mode_cmd.mode = SYNC_PULSE_VDO_MODE;
		lcm_switch_mode_cmd.addr = 0xBB;
		lcm_switch_mode_cmd.val[0] = 0x03;  /* disable GRAM and enable video mode */
	}
	return (void *)(&lcm_switch_mode_cmd);
#else
	return NULL;
#endif
}


LCM_DRIVER oradt_hd720_dsi_cmd_lcm_drv = {
	.name = "oradt_hd720_dsi_cmd_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_init,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
	.esd_check = lcm_esd_check,
	.set_backlight_cmdq = lcm_setbacklight,
	.update = lcm_update,
	.switch_mode = lcm_switch_mode,
};
