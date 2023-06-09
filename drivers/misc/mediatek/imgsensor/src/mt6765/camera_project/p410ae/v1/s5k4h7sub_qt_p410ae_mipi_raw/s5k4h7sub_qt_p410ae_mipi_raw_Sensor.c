/* *****************************************************************************
 *
 * Filename:
 * ---------
 *	 s5k4h7yxmipi_Sensor.c
 *
 * Project:
 * --------
 *	 ALPS
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "s5k4h7sub_qt_p410ae_mipi_raw_Sensor.h"

#ifdef CONFIG_TINNO_PRODUCT_INFO
#include <dev_info.h>
#endif

//unsigned int sensor_module_id = 0;

#define PFX "s5k4h7sub_qt_p410ae_camera_sensor"
#define LOG_INF(format, args...)	pr_debug(PFX "[%s] " format, __func__, ##args)
#define LOG_OTP(format, args...)    pr_err(PFX "[%s] " format, __func__, ##args)
#define READ_SENSOR_OTP

#define S5K4H7YX_TRULY_EEPROM_I2C_ADDR 0xA0
//static MUINT8 pOtp_data[24];

static DEFINE_SPINLOCK(imgsensor_drv_lock);


static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = S5K4H7SUB_QT_P410AE_SENSOR_ID,

	.checksum_value = 0xee4bdaab, //0x138daa55,

	.pre = {
		.pclk = 280000000,				/* record different mode's pclk */
		.linelength = 3688,				/* record different mode's linelength */
		.framelength = 2530,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width = 1632,		/* record different mode's width of grabwindow */
		.grabwindow_height = 1224,		/* record different mode's height of grabwindow */
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
		.mipi_pixel_rate = 280000000,
	},
	.cap = {
		.pclk = 280000000,
		.linelength = 3688,
		.framelength = 2530,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
		.mipi_pixel_rate = 292800000,
	},
	.cap1 = {
		.pclk = 280000000,
		.linelength = 3688,
		.framelength = 3150,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 240,
		.mipi_pixel_rate = 280000000,
	},
    .cap2 = {
	    .pclk = 280000000,
	    .linelength = 3688,
	    .framelength = 5060,
	    .startx = 0,
            .starty = 0,
	    .grabwindow_width = 3264,
	    .grabwindow_height = 2448,
	    .mipi_data_lp2hs_settle_dc = 85,
	    .max_framerate = 150,
		.mipi_pixel_rate = 280000000,
	},
	.normal_video = {
		.pclk = 280000000,
		.linelength = 3688,
		.framelength = 2530,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
		.mipi_pixel_rate = 280000000,
	},
	.hs_video = {	/* VGA120fps */
		.pclk = 280000000,
		.linelength = 3688,
		.framelength = 632,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 640,
		.grabwindow_height = 480,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 1200,
		.mipi_pixel_rate = 280000000,
	},
	.slim_video = {
			.pclk = 280000000,				/* record different mode's pclk */
			.linelength = 3688,			/* record different mode's linelength */
			.framelength = 2530,		   /* record different mode's framelength */
			.startx = 0,					/* record different mode's startx of grabwindow */
			.starty = 0,					/* record different mode's starty of grabwindow */
			.grabwindow_width = 1280,		/* record different mode's width of grabwindow */
			.grabwindow_height = 720,		/* record different mode's height of grabwindow */
			/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
			.mipi_data_lp2hs_settle_dc = 85,
			/*	 following for GetDefaultFramerateByScenario()	*/
			.max_framerate = 300,
			.mipi_pixel_rate = 280000000,
	},
	.margin = 8,
	.min_shutter = 5,
	.max_frame_length = 0xffff-5,
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,
	.ihdr_support = 0,	  /* 1, support; 0,not support */
	.ihdr_le_firstline = 0,  /* 1,le first ; 0, se first */
	.sensor_mode_num = 5,	  /* support sensor mode num */

	.cap_delay_frame = 3,
	.pre_delay_frame = 3,
	.video_delay_frame = 3,
	.hs_video_delay_frame = 3,
	.slim_video_delay_frame = 3,

	.isp_driving_current = ISP_DRIVING_6MA,
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	.mipi_sensor_type = MIPI_OPHY_NCSI2, /* 0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2 */
	.mipi_settle_delay_mode = 1,/* 0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL */
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gr,  //Gb
	.mclk = 24,
	.mipi_lane_num = SENSOR_MIPI_4_LANE,
	.i2c_addr_table = {0x5A, 0xff},
	.i2c_speed = 400,
};


static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,				/* mirrorflip information */
	.sensor_mode = IMGSENSOR_MODE_INIT, /* IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video,High Speed Video, Slim Video */
	.shutter = 0x3D0,					/* current shutter */
	.gain = 0x100,						/* current gain */
	.dummy_pixel = 0,					/* current dummypixel */
	.dummy_line = 0,					/* current dummyline */
	.current_fps = 0,  /* full size current fps : 24fps for PIP, 30fps for Normal or ZSD */
	.autoflicker_en = KAL_FALSE,  /* auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker */
	.test_pattern = KAL_FALSE,		/* test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output */
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,/* current scenario id */
	.ihdr_en = 0, /* sensor need support LE, SE with HDR feature */
	.i2c_write_id = 0x5A,
};


/* Sensor output window information */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[5] = {
 { 3280, 2464,   8,	  8, 3264, 2448, 1632, 1224, 0, 0, 1632, 1224, 0,	0, 1632, 1224}, /* Preview */
 { 3280, 2464,   8,	  8, 3264, 2448, 3264, 2448, 0, 0, 3264, 2448, 0,	0, 3264, 2448}, /* capture */
 { 3280, 2464,   8,	  8, 3264, 2448, 3264, 2448, 0, 0, 3264, 2448, 0,	0, 3264, 2448}, /* video */
 { 3280, 2464, 360, 272, 2560, 1920,  640,  480, 0, 0,  640,  480, 0, 0,  640,  480}, /* hight speed video */
 { 3280, 2464, 360, 512, 2560, 1440, 1280,  720, 0, 0, 1280,  720, 0, 0, 1280,  720}, /* slim video */
};/* slim video */


static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;

	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}
/*static kal_uint16 read_cmos_sensor_otp(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;

	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1, EEPROM_WRITE_ID);

	return get_byte;
}


kal_uint16 otp_4h7_read_cmos_sensor(kal_uint32 addr)
{
	char data = 0;
	data = read_cmos_sensor(addr);
	printk("s5k4h7 otp addr 0x%x data 0x%x \n", addr, data);
	return data;
}*/
static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};

	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}

static void write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para)
{
    char pusendcmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};

    iWriteRegI2C(pusendcmd, 3, imgsensor.i2c_write_id);
}

/*void otp_4h7_write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para)
{
	write_cmos_sensor_8(addr, para);
}
unsigned char S5K4H7_read_cmos_sensor(u32 addr)
{
	kal_uint16 get_byte = 0;

	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}

void S5K4H7_write_cmos_sensor(u16 addr, u32 para)
{
    char pusendcmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};

    iWriteRegI2C(pusendcmd, 3, imgsensor.i2c_write_id);
}*/

static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d\n", imgsensor.dummy_line, imgsensor.dummy_pixel);

	write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
	write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
	write_cmos_sensor_8(0x0342, imgsensor.line_length >> 8);
	write_cmos_sensor_8(0x0343, imgsensor.line_length & 0xFF);

}	/*	set_dummy  */
#if 1
static kal_uint32 return_sensor_id(void)
{
	kal_uint32 get_byte = 0;

	get_byte = (read_cmos_sensor(0x0000) << 8) | read_cmos_sensor(0x0001);
	return get_byte;

}
#endif
/*static kal_uint32 return_module_id(void)
{
	kal_uint32 get_byte = 0;

	get_byte = read_cmos_sensor_otp(0x000F);
	return get_byte;

}*/


static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{

	kal_uint32 frame_length = imgsensor.frame_length;


	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length;
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
	{
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */


static void write_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;

	/* if shutter bigger than frame_length, should extend frame length first */
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
		/* Extend frame length */
		write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
		}
	} else {
		/* Extend frame length */
		write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
	}

	/* Update Shutter */
	write_cmos_sensor_8(0x0202, shutter >> 8);
	write_cmos_sensor_8(0x0203, shutter & 0xFF);


	LOG_INF("shutter =%d, framelength =%d\n", shutter, imgsensor.frame_length);


}	/*	write_shutter  */



/*************************************************************************
* FUNCTION
*	set_shutter
*
* DESCRIPTION
*	This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*	iShutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	write_shutter(shutter);
}	/*	set_shutter */



static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0;

    reg_gain = gain/2;
    return (kal_uint16)reg_gain;
}

/*************************************************************************
* FUNCTION
*	set_gain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*	iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;

	/* 0x350A[0:1], 0x350B[0:7] AGC real gain */
	/* [0:3] = N meams N /16 X	*/
	/* [4:9] = M meams M X		 */
	/* Total gain = M + N /16 X   */

	if (gain < BASEGAIN || gain > 16 * BASEGAIN) {
	LOG_INF("Error gain setting");
	if (gain < BASEGAIN)
	    gain = BASEGAIN;
	else if (gain > 16 * BASEGAIN)
	    gain = 16 * BASEGAIN;
    }

    reg_gain = gain2reg(gain);


	/* reg_gain = gain>>1; */
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

    write_cmos_sensor_8(0x0204, (reg_gain>>8));
    write_cmos_sensor_8(0x0205, (reg_gain&0xff));

	return gain;
}	/*	set_gain  */



/* defined but not used */
static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
	LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n", le, se, gain);
	if (imgsensor.ihdr_en) {

		spin_lock(&imgsensor_drv_lock);
			if (le > imgsensor.min_frame_length - imgsensor_info.margin)
				imgsensor.frame_length = le + imgsensor_info.margin;
			else
				imgsensor.frame_length = imgsensor.min_frame_length;
			if (imgsensor.frame_length > imgsensor_info.max_frame_length)
				imgsensor.frame_length = imgsensor_info.max_frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (le < imgsensor_info.min_shutter) le = imgsensor_info.min_shutter;
			if (se < imgsensor_info.min_shutter) se = imgsensor_info.min_shutter;


				/* Extend frame length first */
				write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);


		set_gain(gain);
	}

}



static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

	/********************************************************
	   *
	   *   0x3820[2] ISP Vertical flip
	   *   0x3820[1] Sensor Vertical flip
	   *
	   *   0x3821[2] ISP Horizontal mirror
	   *   0x3821[1] Sensor Horizontal mirror
	   *
	   *   ISP and Sensor flip or mirror register bit should be the same!!
	   *
	   ********************************************************/

	switch (image_mirror) {
		case IMAGE_NORMAL:
			write_cmos_sensor_8(0x0101, 0x00);
			break;
		case IMAGE_H_MIRROR:
			write_cmos_sensor_8(0x0101, 0x01);
			break;
		case IMAGE_V_MIRROR:
			write_cmos_sensor_8(0x0101, 0x02);
			break;
		case IMAGE_HV_MIRROR:
			write_cmos_sensor_8(0x0101, 0x03);
			break;
		default:
			LOG_INF("Error image_mirror setting\n");
	}

}

/*************************************************************************
* FUNCTION
*	night_mode
*
* DESCRIPTION
*	This function night mode of sensor.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
#if 0
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/
}	/*	night_mode	*/
#endif
/* #define LSC_cal	1 */
static void sensor_init(void)
{
	LOG_INF("sensor_init() E\n");
	/* Base on S5K4H7_EVT0_Reference_setfile_v0.91 */
  write_cmos_sensor_8(0x0100, 0x00);
  write_cmos_sensor_8(0x0B05, 0x01);
  write_cmos_sensor_8(0x3074, 0x06);
  write_cmos_sensor_8(0x3075, 0x2F);
  write_cmos_sensor_8(0x308A, 0x20);
  write_cmos_sensor_8(0x308B, 0x08);
  write_cmos_sensor_8(0x308C, 0x0B);
  write_cmos_sensor_8(0x3081, 0x07);
  write_cmos_sensor_8(0x307B, 0x85);
  write_cmos_sensor_8(0x307A, 0x0A);
  write_cmos_sensor_8(0x3079, 0x0A);
  write_cmos_sensor_8(0x306E, 0x71);
  write_cmos_sensor_8(0x306F, 0x28);
  write_cmos_sensor_8(0x301F, 0x20);
  write_cmos_sensor_8(0x306B, 0x9A);
  write_cmos_sensor_8(0x3091, 0x1F);
  write_cmos_sensor_8(0x30C4, 0x06);
  write_cmos_sensor_8(0x3200, 0x09);
  write_cmos_sensor_8(0x306A, 0x79);
  write_cmos_sensor_8(0x30B0, 0xFF);
  write_cmos_sensor_8(0x306D, 0x08);
  write_cmos_sensor_8(0x3080, 0x00);
  write_cmos_sensor_8(0x3929, 0x3F);
  write_cmos_sensor_8(0x3084, 0x16);
  write_cmos_sensor_8(0x3070, 0x0F);
  write_cmos_sensor_8(0x3B45, 0x01);
  write_cmos_sensor_8(0x30C2, 0x05);
  write_cmos_sensor_8(0x3069, 0x87);
  write_cmos_sensor_8(0x3924, 0x7F);
  write_cmos_sensor_8(0x3925, 0xFD);
  write_cmos_sensor_8(0x3C08, 0xFF);
  write_cmos_sensor_8(0x3C09, 0xFF);
  write_cmos_sensor_8(0x3C31, 0xFF);
  write_cmos_sensor_8(0x3C32, 0xFF);
  write_cmos_sensor_8(0x0A02, 0x14);
  write_cmos_sensor_8(0x392F, 0x01);
  write_cmos_sensor_8(0x3930, 0x80);
}	/*	sensor_init  */


static void preview_setting(void)
{
	kal_uint8 framecnt;
	int i;

    LOG_INF("preview_setting() E\n");
	write_cmos_sensor_8(0x0100, 0x00);
	for (i = 0; i < 100; i++)
	{
	     framecnt = read_cmos_sensor(0x0005); /* waiting for sensor to  stop output  then  set the  setting */
	     if (framecnt == 0xFF)
	     {
		 LOG_INF("stream is off\n");
		  break;
	     }
	      else{
		  LOG_INF("stream is not off\n");
		 mdelay(1);
	      }
	}

  write_cmos_sensor_8(0x0136, 0x18);
  write_cmos_sensor_8(0x0137, 0x00);
  write_cmos_sensor_8(0x0305, 0x06);
  write_cmos_sensor_8(0x0306, 0x00);
  write_cmos_sensor_8(0x0307, 0x8C);
  write_cmos_sensor_8(0x030D, 0x04);
  write_cmos_sensor_8(0x030E, 0x00);
  write_cmos_sensor_8(0x030F, 0x7A);
  write_cmos_sensor_8(0x3C1F, 0x00);
  write_cmos_sensor_8(0x3C17, 0x00);
  write_cmos_sensor_8(0x3C1C, 0x04);
  write_cmos_sensor_8(0x3C1D, 0x15);
  write_cmos_sensor_8(0x0301, 0x04);
  write_cmos_sensor_8(0x0820, 0x02);
  write_cmos_sensor_8(0x0821, 0xDC);
  write_cmos_sensor_8(0x0822, 0x00);
  write_cmos_sensor_8(0x0823, 0x00);
  write_cmos_sensor_8(0x0112, 0x0A);
  write_cmos_sensor_8(0x0113, 0x0A);
  write_cmos_sensor_8(0x0114, 0x03);
  write_cmos_sensor_8(0x3906, 0x00);
  write_cmos_sensor_8(0x0344, 0x00);
  write_cmos_sensor_8(0x0345, 0x08);
  write_cmos_sensor_8(0x0346, 0x00);
  write_cmos_sensor_8(0x0347, 0x08);
  write_cmos_sensor_8(0x0348, 0x0C);
  write_cmos_sensor_8(0x0349, 0xC7);
  write_cmos_sensor_8(0x034A, 0x09);
  write_cmos_sensor_8(0x034B, 0x97);
  write_cmos_sensor_8(0x034C, 0x06);
  write_cmos_sensor_8(0x034D, 0x60);
  write_cmos_sensor_8(0x034E, 0x04);
  write_cmos_sensor_8(0x034F, 0xC8);
  write_cmos_sensor_8(0x0900, 0x01);
  write_cmos_sensor_8(0x0901, 0x22);
  write_cmos_sensor_8(0x0381, 0x01);
  write_cmos_sensor_8(0x0383, 0x01);
  write_cmos_sensor_8(0x0385, 0x01);
  write_cmos_sensor_8(0x0387, 0x03);
  write_cmos_sensor_8(0x0101, 0x00);
  write_cmos_sensor_8(0x0340, 0x09);
  write_cmos_sensor_8(0x0341, 0xE2);
  write_cmos_sensor_8(0x0342, 0x0E);
  write_cmos_sensor_8(0x0343, 0x68);
  write_cmos_sensor_8(0x0200, 0x0D);
  write_cmos_sensor_8(0x0201, 0xD8);
  write_cmos_sensor_8(0x0202, 0x02);
  write_cmos_sensor_8(0x0203, 0x08);
  write_cmos_sensor_8(0x3400, 0x00);
  write_cmos_sensor_8(0x0100, 0x01);
}	/*	preview_setting  */


static void capture_setting(kal_uint16 currefps)
{
	kal_uint8 framecnt;
	int i;
	LOG_INF("capture_setting() E! currefps:%d\n", currefps);

	write_cmos_sensor_8(0x0100, 0x00);

	 for (i = 0; i < 100; i++)
	{
	    framecnt = read_cmos_sensor(0x0005);
	     if (framecnt == 0xFF)
	     {
		 LOG_INF("stream is  off\\n");
		  break;
	     }
	      else{
		  LOG_INF("stream is not off\\n");
		 mdelay(1);
	      }
	}

  write_cmos_sensor_8(0x0136, 0x18);
  write_cmos_sensor_8(0x0137, 0x00);
  write_cmos_sensor_8(0x0305, 0x06);
  write_cmos_sensor_8(0x0306, 0x00);
  write_cmos_sensor_8(0x0307, 0x8C);
  write_cmos_sensor_8(0x030D, 0x04);
  write_cmos_sensor_8(0x030E, 0x00);
  write_cmos_sensor_8(0x030F, 0x7A);
  write_cmos_sensor_8(0x3C1F, 0x00);
  write_cmos_sensor_8(0x3C17, 0x00);
  write_cmos_sensor_8(0x3C1C, 0x04);
  write_cmos_sensor_8(0x3C1D, 0x15);
  write_cmos_sensor_8(0x0301, 0x04);
  write_cmos_sensor_8(0x0820, 0x02);
  write_cmos_sensor_8(0x0821, 0xDC);
  write_cmos_sensor_8(0x0822, 0x00);
  write_cmos_sensor_8(0x0823, 0x00);
  write_cmos_sensor_8(0x0112, 0x0A);
  write_cmos_sensor_8(0x0113, 0x0A);
  write_cmos_sensor_8(0x0114, 0x03);
  write_cmos_sensor_8(0x3906, 0x04);
  write_cmos_sensor_8(0x0344, 0x00);
  write_cmos_sensor_8(0x0345, 0x08);
  write_cmos_sensor_8(0x0346, 0x00);
  write_cmos_sensor_8(0x0347, 0x08);
  write_cmos_sensor_8(0x0348, 0x0C);
  write_cmos_sensor_8(0x0349, 0xC7);
  write_cmos_sensor_8(0x034A, 0x09);
  write_cmos_sensor_8(0x034B, 0x97);
  write_cmos_sensor_8(0x034C, 0x0C);
  write_cmos_sensor_8(0x034D, 0xC0);
  write_cmos_sensor_8(0x034E, 0x09);
  write_cmos_sensor_8(0x034F, 0x90);
  write_cmos_sensor_8(0x0900, 0x00);
  write_cmos_sensor_8(0x0901, 0x00);
  write_cmos_sensor_8(0x0381, 0x01);
  write_cmos_sensor_8(0x0383, 0x01);
  write_cmos_sensor_8(0x0385, 0x01);
  write_cmos_sensor_8(0x0387, 0x01);
  write_cmos_sensor_8(0x0101, 0x00);

	if (currefps == 300) {	
		write_cmos_sensor_8(0x0340, 0x09);
		write_cmos_sensor_8(0x0341, 0xE2);
	} else if(currefps==240){	//24fps
		write_cmos_sensor_8(0x0340, 0x0C);
		write_cmos_sensor_8(0x0341, 0x5B);
	} else{ //15fps
		write_cmos_sensor_8(0x0340, 0x13);
		write_cmos_sensor_8(0x0341, 0xC5);
	}

  write_cmos_sensor_8(0x0342, 0x0E);
  write_cmos_sensor_8(0x0343, 0x68);
  write_cmos_sensor_8(0x0200, 0x0D);
  write_cmos_sensor_8(0x0201, 0xD8);
  write_cmos_sensor_8(0x0202, 0x00);
  write_cmos_sensor_8(0x0203, 0x02);
  write_cmos_sensor_8(0x3400, 0x00);
  write_cmos_sensor_8(0x0100, 0x01);
}

static void normal_video_setting(kal_uint16 currefps)
{
	kal_uint8 framecnt;
	int i;
	LOG_INF("normal_video_setting() E! currefps:%d\n", currefps);

	write_cmos_sensor_8(0x0100, 0x00);

	for (i = 0; i < 100; i++)
	{
	    framecnt = read_cmos_sensor(0x0005);
	     if (framecnt == 0xFF)
	     {
		 LOG_INF("stream is off\\n");
		  break;
	     }
	      else{
		 LOG_INF("stream is not off\\n");
		 mdelay(1);
	      }
	}

  write_cmos_sensor_8(0x0136, 0x18);
  write_cmos_sensor_8(0x0137, 0x00);
  write_cmos_sensor_8(0x0305, 0x06);
  write_cmos_sensor_8(0x0306, 0x00);
  write_cmos_sensor_8(0x0307, 0x8C);
  write_cmos_sensor_8(0x030D, 0x06);
  write_cmos_sensor_8(0x030E, 0x00);
  write_cmos_sensor_8(0x030F, 0xAF);
  write_cmos_sensor_8(0x3C1F, 0x00);
  write_cmos_sensor_8(0x3C17, 0x00);
  write_cmos_sensor_8(0x3C1C, 0x05);
  write_cmos_sensor_8(0x3C1D, 0x15);
  write_cmos_sensor_8(0x0301, 0x04);
  write_cmos_sensor_8(0x0820, 0x02);
  write_cmos_sensor_8(0x0821, 0xBC);
  write_cmos_sensor_8(0x0822, 0x00);
  write_cmos_sensor_8(0x0823, 0x00);
  write_cmos_sensor_8(0x0112, 0x0A);
  write_cmos_sensor_8(0x0113, 0x0A);
  write_cmos_sensor_8(0x0114, 0x03);
  write_cmos_sensor_8(0x3906, 0x04);
  write_cmos_sensor_8(0x0344, 0x00);
  write_cmos_sensor_8(0x0345, 0x08);
  write_cmos_sensor_8(0x0346, 0x00);
  write_cmos_sensor_8(0x0347, 0x08);
  write_cmos_sensor_8(0x0348, 0x0C);
  write_cmos_sensor_8(0x0349, 0xC7);
  write_cmos_sensor_8(0x034A, 0x09);
  write_cmos_sensor_8(0x034B, 0x97);
  write_cmos_sensor_8(0x034C, 0x0C);
  write_cmos_sensor_8(0x034D, 0xC0);
  write_cmos_sensor_8(0x034E, 0x09);
  write_cmos_sensor_8(0x034F, 0x90);
  write_cmos_sensor_8(0x0900, 0x00);
  write_cmos_sensor_8(0x0901, 0x00);
  write_cmos_sensor_8(0x0381, 0x01);
  write_cmos_sensor_8(0x0383, 0x01);
  write_cmos_sensor_8(0x0385, 0x01);
  write_cmos_sensor_8(0x0387, 0x01);
  write_cmos_sensor_8(0x0101, 0x00);
	write_cmos_sensor_8(0x0340, 0x09);
	write_cmos_sensor_8(0x0341, 0xE2);
  write_cmos_sensor_8(0x0342, 0x0E);
  write_cmos_sensor_8(0x0343, 0x68);
  write_cmos_sensor_8(0x0200, 0x0D);
  write_cmos_sensor_8(0x0201, 0xD8);
  write_cmos_sensor_8(0x0202, 0x00);
  write_cmos_sensor_8(0x0203, 0x02);
  write_cmos_sensor_8(0x3400, 0x00);
  write_cmos_sensor_8(0x0100, 0x01);

}
static void hs_video_setting(void)
{
	kal_uint8 framecnt;
	int i;
	LOG_INF("hs_video_setting() E\n");

	write_cmos_sensor_8(0x0100, 0x00);
	for (i = 0; i < 100; i++)
	{
	     framecnt = read_cmos_sensor(0x0005); /* waiting for sensor to  stop output  then  set the  setting */
	     if (framecnt == 0xFF)
	     {
		 LOG_INF("stream is off\\n");
		  break;
	     }
	      else{
		  LOG_INF("stream is not off\\n");
		 mdelay(1);
	      }
	}

  write_cmos_sensor_8(0x0136, 0x18);
  write_cmos_sensor_8(0x0137, 0x00);
  write_cmos_sensor_8(0x0305, 0x06);
  write_cmos_sensor_8(0x0306, 0x00);
  write_cmos_sensor_8(0x0307, 0x8C);
  write_cmos_sensor_8(0x030D, 0x06);
  write_cmos_sensor_8(0x030E, 0x00);
  write_cmos_sensor_8(0x030F, 0xAF);
  write_cmos_sensor_8(0x3C1F, 0x00);
  write_cmos_sensor_8(0x3C17, 0x00);
  write_cmos_sensor_8(0x3C1C, 0x05);
  write_cmos_sensor_8(0x3C1D, 0x15);
  write_cmos_sensor_8(0x0301, 0x04);
  write_cmos_sensor_8(0x0820, 0x02);
  write_cmos_sensor_8(0x0821, 0xBC);
  write_cmos_sensor_8(0x0822, 0x00);
  write_cmos_sensor_8(0x0823, 0x00);
  write_cmos_sensor_8(0x0112, 0x0A);
  write_cmos_sensor_8(0x0113, 0x0A);
  write_cmos_sensor_8(0x0114, 0x03);
  write_cmos_sensor_8(0x3906, 0x00);
  write_cmos_sensor_8(0x0344, 0x01);
  write_cmos_sensor_8(0x0345, 0x68);
  write_cmos_sensor_8(0x0346, 0x01);
  write_cmos_sensor_8(0x0347, 0x10);
  write_cmos_sensor_8(0x0348, 0x0B);
  write_cmos_sensor_8(0x0349, 0x67);
  write_cmos_sensor_8(0x034A, 0x08);
  write_cmos_sensor_8(0x034B, 0x8F);
  write_cmos_sensor_8(0x034C, 0x02);
  write_cmos_sensor_8(0x034D, 0x80);
  write_cmos_sensor_8(0x034E, 0x01);
  write_cmos_sensor_8(0x034F, 0xE0);
  write_cmos_sensor_8(0x0900, 0x01);
  write_cmos_sensor_8(0x0901, 0x44);
  write_cmos_sensor_8(0x0381, 0x01);
  write_cmos_sensor_8(0x0383, 0x01);
  write_cmos_sensor_8(0x0385, 0x01);
  write_cmos_sensor_8(0x0387, 0x07);
  write_cmos_sensor_8(0x0101, 0x00);
  write_cmos_sensor_8(0x0340, 0x02);
  write_cmos_sensor_8(0x0341, 0x78);
  write_cmos_sensor_8(0x0342, 0x0E);
  write_cmos_sensor_8(0x0343, 0x68);
  write_cmos_sensor_8(0x0200, 0x0D);
  write_cmos_sensor_8(0x0201, 0xD8);
  write_cmos_sensor_8(0x0202, 0x02);
  write_cmos_sensor_8(0x0203, 0x08);
  write_cmos_sensor_8(0x3400, 0x00);
  write_cmos_sensor_8(0x0100, 0x01);

}

static void slim_video_setting(void)
{
	kal_uint8 framecnt;
	int i;
	LOG_INF("slim_video_setting() E\n");

	write_cmos_sensor_8(0x0100, 0x00);


	for (i = 0; i < 100; i++)
	{
	     framecnt = read_cmos_sensor(0x0005); /* waiting for sensor to  stop output  then  set the  setting */
	     if (framecnt == 0xFF)
	     {
		 LOG_INF("stream is  off\\n");
		  break;
	     }
	      else{
		  LOG_INF("stream is not off\\n");
		 mdelay(1);
	      }
	}

  write_cmos_sensor_8(0x0136, 0x18);
  write_cmos_sensor_8(0x0137, 0x00);
  write_cmos_sensor_8(0x0305, 0x06);
  write_cmos_sensor_8(0x0306, 0x00);
  write_cmos_sensor_8(0x0307, 0x8C);
  write_cmos_sensor_8(0x030D, 0x06);
  write_cmos_sensor_8(0x030E, 0x00);
  write_cmos_sensor_8(0x030F, 0xAF);
  write_cmos_sensor_8(0x3C1F, 0x00);
  write_cmos_sensor_8(0x3C17, 0x00);
  write_cmos_sensor_8(0x3C1C, 0x05);
  write_cmos_sensor_8(0x3C1D, 0x15);
  write_cmos_sensor_8(0x0301, 0x04);
  write_cmos_sensor_8(0x0820, 0x02);
  write_cmos_sensor_8(0x0821, 0xBC);
  write_cmos_sensor_8(0x0822, 0x00);
  write_cmos_sensor_8(0x0823, 0x00);
  write_cmos_sensor_8(0x0112, 0x0A);
  write_cmos_sensor_8(0x0113, 0x0A);
  write_cmos_sensor_8(0x0114, 0x03);
  write_cmos_sensor_8(0x3906, 0x00);
  write_cmos_sensor_8(0x0344, 0x01);
  write_cmos_sensor_8(0x0345, 0x68);
  write_cmos_sensor_8(0x0346, 0x02);
  write_cmos_sensor_8(0x0347, 0x00);
  write_cmos_sensor_8(0x0348, 0x0B);
  write_cmos_sensor_8(0x0349, 0x67);
  write_cmos_sensor_8(0x034A, 0x07);
  write_cmos_sensor_8(0x034B, 0x9F);
  write_cmos_sensor_8(0x034C, 0x05);
  write_cmos_sensor_8(0x034D, 0x00);
  write_cmos_sensor_8(0x034E, 0x02);
  write_cmos_sensor_8(0x034F, 0xD0);
  write_cmos_sensor_8(0x0900, 0x01);
  write_cmos_sensor_8(0x0901, 0x22);
  write_cmos_sensor_8(0x0381, 0x01);
  write_cmos_sensor_8(0x0383, 0x01);
  write_cmos_sensor_8(0x0385, 0x01);
  write_cmos_sensor_8(0x0387, 0x03);
  write_cmos_sensor_8(0x0101, 0x00);
  write_cmos_sensor_8(0x0340, 0x09);
  write_cmos_sensor_8(0x0341, 0xE2);
  write_cmos_sensor_8(0x0342, 0x0E);
  write_cmos_sensor_8(0x0343, 0x68);
  write_cmos_sensor_8(0x0200, 0x0D);
  write_cmos_sensor_8(0x0201, 0xD8);
  write_cmos_sensor_8(0x0202, 0x02);
  write_cmos_sensor_8(0x0203, 0x08);
  write_cmos_sensor_8(0x3400, 0x00);
  write_cmos_sensor_8(0x0100, 0x01);

}
/*
static kal_uint16 read_eeprom_module(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };
	iReadRegI2C(pusendcmd, 2, (u8 *) &get_byte, 1, S5K4H7YX_TRULY_EEPROM_I2C_ADDR);

	return get_byte;
}
static int get_eeprom_data(MUINT8 *data)
{
    MUINT8 i =0x0;
    u8 *otp_data = (u8 *)data;

    for (i = 0x07;i <= 0x1E; i++, otp_data++){
        *otp_data = read_eeprom_module(i);
        pr_err("pOtp_data s5k4h7_truly otpdata[0x%x]=0x%x \n", i, *otp_data);
    }
    return 0;
}*/

#ifdef READ_SENSOR_OTP
#define MODULE_ID_START_ADD 0x0a0c
#define MODULE_ID_LENGTH 0x04
#define getbit(x,y)   ((x) >> (y)&1)
#define DUMP_DATA_PATH "/data/vendor/camera_dump/s5k4h7sub_otp_data.bin"

 typedef struct otp_data_sub {
 	unsigned char page_flag;
 	unsigned char module_id[4];
 	unsigned char awb_gloden[8];
 	unsigned char awb_unint[8];
 	unsigned char lscdata[1871];
	unsigned char serial_number[8];
	unsigned char name[2];
 } OTP_DATA_SUB;

OTP_DATA_SUB otp_data_sub;

static void write_cmos_sensor_16(kal_uint16 addr, kal_uint16 para)
{
    char pusendcmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para >> 8), (char)(para & 0xFF)};

    iWriteRegI2C(pusendcmd, 4, imgsensor.i2c_write_id);
}
static int read_otp_page_data(int page, int start_add, unsigned char *Buff, int size)
 {
 	unsigned short stram_flag = 0;
 	unsigned short val = 0;
 	int i = 0;
 	if (NULL == Buff) return 0;
 
 	stram_flag = read_cmos_sensor(0x0100); //3
 	if (stram_flag == 0) {
 		write_cmos_sensor(0x0100,0x01);   //3
 		mdelay(50);
 	}
 	write_cmos_sensor_8(0x0a02,page);    //3
 	write_cmos_sensor_8(0x3b41,0x01);
 	write_cmos_sensor_8(0x3b42,0x03);
 	write_cmos_sensor_8(0x3b40,0x01);
 	write_cmos_sensor_16(0x0a00,0x0100); //4 otp enable and read start
 
 	for (i = 0; i <= 100; i++)
 	{
 		mdelay(1);
 		val = read_cmos_sensor(0x0A01);
 		if (val == 0x01)
 			break;
 	}
 
 	for ( i = 0; i < size; i++ ) {
 		Buff[i] = read_cmos_sensor(start_add+i); //3
 		LOG_OTP("cur page = %d, Buff[%d] = 0x%x\n",page,i,Buff[i]);
 	}
 	write_cmos_sensor_16(0x0a00,0x0400);
 	write_cmos_sensor_16(0x0a00,0x0000); //4 //otp enable and read end
 
 	return 0;
 }
 
static bool read_valid_data(int page, int start_add)
 {
 	unsigned char page_flag[2] = {0};
 	read_otp_page_data(page,start_add,&page_flag[0],1);
 	read_otp_page_data(page,start_add,&page_flag[1],1);
 	LOG_INF("page = %d,page_flag0 = 0x%x,f1 = 0x%x\n",page,page_flag[0],page_flag[1]);
 	if (page_flag[0] != 0 || page_flag[1] != 0) {
 		otp_data_sub.page_flag = ((page_flag[0] > page_flag[1])? page_flag[0] : page_flag[1]);
 		if (!((getbit(otp_data_sub.page_flag,6)) && !(getbit(otp_data_sub.page_flag,7)))) {
 			LOG_INF("valid bit 7 = %d,bit 6 = %d\n",getbit(otp_data_sub.page_flag,7),getbit(otp_data_sub.page_flag,6));
 			LOG_OTP("4h7sub error data not valid\n");
 			return false;
 		}else{
 			return true;
 		}
 	}else{
 		otp_data_sub.page_flag = 0;
 		return false;
 	}
 }
 
static int get_vaild_data_page(int start_add)
 {
 	unsigned short page = 21;

 	for (page = 21;page <= 31; page+=5) {
 		if(read_valid_data(page, start_add)){
 			break;
 		}else if (31 == page){
 			return 0;
 		}
 	}
 
 	return page;
 }
 
static int get_vaild_lsc_data_page(int start_add)
 {
 	unsigned short page = 36;
 
	if(read_valid_data(page, start_add))
		return page;

	page += 29;
	if(read_valid_data(page, start_add + 15))
		return page;

	page += 29;
	if(read_valid_data(page, start_add + 30))
		return page;
 
 	return 0;
 }
 
static int read_otp_data(void)
 {
 	unsigned int page = 0;
 	unsigned int i = 0;
 	unsigned int sum = 0;
 	unsigned int lsc_addr = 0x0a04;
 
 	page = get_vaild_data_page(0x0a04);
 	if(!page) return -1;
 	LOG_OTP("moduleid vaild page = %d", page);
 
	read_otp_page_data(page, 0x0a1a, otp_data_sub.serial_number, 8);
	read_otp_page_data(page, 0x0a12, otp_data_sub.name, 2);
 	//read module id
 
 	read_otp_page_data(page, MODULE_ID_START_ADD, otp_data_sub.module_id, MODULE_ID_LENGTH);
 	for (i = 0;i < MODULE_ID_LENGTH ; i++ ) {
 		LOG_INF ("+++4h7 6 page = %d modulue_id[%d] = 0x%x",page,i,otp_data_sub.module_id[i]);
 		sum += otp_data_sub.module_id[i];
 	}
 	if (!sum) return -1;
 
 	page = get_vaild_data_page(0x0a3c); //read awb flag vaild
 	if(!page) return -1;
 	LOG_INF("awb vaild page = %d", page);
 
 	read_otp_page_data(page, 0x0a3d, otp_data_sub.awb_gloden, 7);
 	read_otp_page_data(page + 1, 0x0a04, otp_data_sub.awb_gloden + 7, 1);
 	read_otp_page_data(page + 1, 0x0a19, otp_data_sub.awb_unint, 8);
 
 	page = get_vaild_lsc_data_page(0x0a04);
 	if(!page) return -1;
 	LOG_INF("lsc vaild page = %d", page);

	if(page == 36) {
 		for (i = 0; i < 30 ; i++ ) {
 			if (i == 29) {
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (64*i), 15);
 			} else {
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (64*i), 64);
 			}
 		}
	} else if(page == 65) {
		lsc_addr = 0x0a13;
 		for (i = 0; i < 30 ; i++ ) {
			if( (i!=0) && (i!=29)) {
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (64*i), 64);
			} else if (i == 0) {
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (64*i), 49);
 			} else if(i == 29){
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (49 + 64*(i-1)), 30);
 			}
 		}
	} else if(page == 94) {
		lsc_addr = 0x0a22;
 		for (i = 0; i < 30 ; i++ ) {
			if( (i!=0) && (i!=29)) {
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (64*i), 64);
			} else if (i == 0) {
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (64*i), 34);
 			} else if(i == 29){
 				read_otp_page_data(page + i, lsc_addr, otp_data_sub.lscdata + (34 + 64*(i-1)), 45);
 			}
 		}
	}
 	return 0;
 }

  static int read_otp_data_all(unsigned char *data, unsigned int size)
 {

 	int page_num;
 	int cnt;
 	int i;

 	page_num = (int)(size/64);
 	cnt = (int) (size % 64);

 	if (page_num > 100) return -1;

 	for (i = 0; i < page_num ; i++ ) {
 		read_otp_page_data(21 + i, 0x0A04, data + (64*i), 64);
 	}
 	if (cnt > 0) {
 		read_otp_page_data(21 + page_num, 0x0A04, data + (64*page_num), cnt);
 	}
 	return size;
 }

 
unsigned int s5k4h7sub_read_region(struct i2c_client *client,unsigned int addr,unsigned char *data, unsigned int size)
 {
 	if (size == 4) { //read module id
 		memcpy(data, otp_data_sub.module_id, size);
 		LOG_OTP("add = 0x%x,read module id\n",addr);
 	} else if (size == 8) { //read single awb data
 		if (addr == 0x05) {
 			memcpy(data, (otp_data_sub.awb_gloden), size);
 			LOG_OTP("add = 0x%x, read golden\n",addr);
 		} else {
 			memcpy(data,(otp_data_sub.awb_unint), size);
 			LOG_OTP("add = 0x%x, read awb_unint\n",addr);
 		}
 	} else if (size >=1868 && size < 2048){
 		if (addr == 0x15) {
 			memcpy(data, (otp_data_sub.lscdata), size);
 		}else {
 			memcpy(data, (otp_data_sub.lscdata + 1), size);
 		}
 	} else if (size > 2048 && size < 6400){ //read all data
 			read_otp_data_all(data, size);
 	}else{
 		LOG_OTP("add = 0x%x, size = %d ,read error !!!\n",addr,size); 		
 	}
 
 	return size;
 }

/*
static void otp_dump_bin(const char *file_name, uint32_t size, const void *data)
 {
 	struct file *fp = NULL;
 	mm_segment_t old_fs;
 	int ret = 0;

 	old_fs = get_fs();
 	set_fs(KERNEL_DS);

 	fp = filp_open(file_name, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0666);
 	if (IS_ERR_OR_NULL(fp)) {
 		ret = PTR_ERR(fp);
 		LOG_INF("open file error(%s), error(%d)\n",  file_name, ret);
 		goto p_err;
 	}

 	ret = vfs_write(fp, (const char *)data, size, &fp->f_pos);
 	if (ret < 0) {
 		LOG_INF("file write fail(%s) to EEPROM data(%d)", file_name, ret);
 		goto p_err;
 	}

 	LOG_INF("wirte to file(%s)\n", file_name);
 p_err:
 	if (!IS_ERR_OR_NULL(fp))
 		filp_close(fp, NULL);

 	set_fs(old_fs);
 	LOG_INF(" end writing file");
 }

 */
#endif

/*************************************************************************
* FUNCTION
*	get_imgsensor_id
*
* DESCRIPTION
*	This function get the sensor ID
*
* PARAMETERS
*	*sensorID : return the sensor ID
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 module_id = 0;
	/* sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address */
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_sensor_id();
			pr_err("4h7truly sensor id = 0x%x.\n", *sensor_id);
			if (*sensor_id == imgsensor_info.sensor_id) {
#ifdef READ_SENSOR_OTP
				read_otp_data();
#endif				
				//get_eeprom_data(pOtp_data);
#ifdef CONFIG_TINNO_PRODUCT_INFO
				//FULL_PRODUCT_DEVICE_INFO(ID_SUB_CAM_SN, pOtp_data);
				pr_err("quhongjin read otp data name %c %c \n", otp_data_sub.name[0],otp_data_sub.name[1]);
				if('T' ==  otp_data_sub.name[0] && 'S' == otp_data_sub.name[1]){
					FULL_PRODUCT_DEVICE_INFO_CAMERA(S5K4H7SUB_QT_P410AE_SENSOR_ID,1,"s5k4h7sub_tsp_p411be_mipi_raw",imgsensor_info.cap.grabwindow_width, imgsensor_info.cap.grabwindow_height);
				}else if('Q' == otp_data_sub.name[0] && 'T' == otp_data_sub.name[1]){
					FULL_PRODUCT_DEVICE_INFO_CAMERA(S5K4H7SUB_QT_P410AE_SENSOR_ID,1,"s5k4h7sub_qt_p411be_mipi_raw",imgsensor_info.cap.grabwindow_width, imgsensor_info.cap.grabwindow_height);
				}
#endif
                   break;
			}
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}
  
	if (*sensor_id != imgsensor_info.sensor_id) {

		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	//module_id = return_module_id();
	  pr_err("4h7truly module id = 0x%x.\n", module_id);

   // if(sensor_module_id == 0x06) {
//add camera info for v770
/*#ifdef CONFIG_TINNO_PRODUCT_INFO
        FULL_PRODUCT_DEVICE_INFO_CAMERA(S5K4H7SUB_QT_P410_SENSOR_ID, 1, "s5k4h7sub_qt_p410ae_mipi_raw",
                imgsensor_info.cap.grabwindow_width, imgsensor_info.cap.grabwindow_height);
#endif*/
//	pr_err("4h7truly module id = 0x%x.\n", sensor_module_id);
        return ERROR_NONE;
  /*  } else {
        pr_err("4h7 sensor module id = 0x%x.\n", sensor_module_id);
        *sensor_id = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }*/
}


/*************************************************************************
* FUNCTION
*	open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{

	/* const kal_uint8 i2c_addr[] = {IMGSENSOR_WRITE_ID_1, IMGSENSOR_WRITE_ID_2}; */
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0;


	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id();
			LOG_INF("s5k4h7yxmipiraw open sensor_id = %x\r\n", sensor_id);
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF("Read sensor id fail, id: 0x%x\n", sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;


	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.shutter = 0x3D0;
	imgsensor.gain = 0x100;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}	/*	open  */



/*************************************************************************
* FUNCTION
*	close
*
* DESCRIPTION
*
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	/*No Need to implement this function*/

	return ERROR_NONE;
}	/*	close  */


/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();
	set_mirror_flip(imgsensor.mirror);
	return ERROR_NONE;
}	/*	preview   */

/*************************************************************************
* FUNCTION
*	capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	if (imgsensor.current_fps == imgsensor_info.cap.max_framerate) {
		imgsensor.pclk = imgsensor_info.cap.pclk;
		imgsensor.line_length = imgsensor_info.cap.linelength;
		imgsensor.frame_length = imgsensor_info.cap.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
		}
	else if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {/* PIP capture:15fps */
		imgsensor.pclk = imgsensor_info.cap1.pclk;
		imgsensor.line_length = imgsensor_info.cap1.linelength;
		imgsensor.frame_length = imgsensor_info.cap1.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	} else {/* PIP capture: 24fps */
		imgsensor.pclk = imgsensor_info.cap2.pclk;
		imgsensor.line_length = imgsensor_info.cap2.linelength;
		imgsensor.frame_length = imgsensor_info.cap2.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap2.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	}
	spin_unlock(&imgsensor_drv_lock);

	capture_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);


	return ERROR_NONE;
}	/* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	LOG_INF("E\n");


	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	slim_video	 */



static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;



	sensor_resolution->SensorHighSpeedVideoWidth	 = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight	 = imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth	 = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight	 = imgsensor_info.slim_video.grabwindow_height;
	return ERROR_NONE;
}	/*	get_resolution	*/

static kal_uint32 get_info(
			enum MSDK_SCENARIO_ID_ENUM scenario_id,
			MSDK_SENSOR_INFO_STRUCT *sensor_info,
			MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);




	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; /* inverse with datasheet */
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;
	//strncpy(sensor_info->TinnoModuleSn,pOtp_data,sizeof(pOtp_data));
	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;		 /* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;	/* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;

	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  /* 0 is default 1x */
	sensor_info->SensorHightSampling = 0;	/* 0 is default 1x */
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

			sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

			break;
		default:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}

	return ERROR_NONE;
}	/*	get_info  */


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			preview(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			capture(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			normal_video(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			hs_video(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			slim_video(image_window, sensor_config_data);
			break;
		default:
			LOG_INF("Error ScenarioId setting");
			preview(image_window, sensor_config_data);
			return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}	/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);
	if (framerate == 0)
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d\n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) /* enable auto flicker */
		imgsensor.autoflicker_en = KAL_TRUE;
	else /* Cancel Auto flick */
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(
			enum MSDK_SCENARIO_ID_ENUM scenario_id,
			MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter) set_dummy();
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if (framerate == 0)
				return ERROR_NONE;
			frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter) set_dummy();
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		  if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
		frame_length = imgsensor_info.cap1.pclk / framerate * 10 / imgsensor_info.cap1.linelength;
		spin_lock(&imgsensor_drv_lock);
			    imgsensor.dummy_line = (frame_length > imgsensor_info.cap1.framelength) ? (frame_length - imgsensor_info.cap1.framelength) : 0;
			    imgsensor.frame_length = imgsensor_info.cap1.framelength + imgsensor.dummy_line;
			    imgsensor.min_frame_length = imgsensor.frame_length;
			    spin_unlock(&imgsensor_drv_lock);
	    } else if (imgsensor.current_fps == imgsensor_info.cap2.max_framerate) {
		 frame_length = imgsensor_info.cap2.pclk / framerate * 10 / imgsensor_info.cap2.linelength;
		spin_lock(&imgsensor_drv_lock);
			    imgsensor.dummy_line = (frame_length > imgsensor_info.cap2.framelength) ? (frame_length - imgsensor_info.cap2.framelength) : 0;
			    imgsensor.frame_length = imgsensor_info.cap2.framelength + imgsensor.dummy_line;
			    imgsensor.min_frame_length = imgsensor.frame_length;
			    spin_unlock(&imgsensor_drv_lock);
		}
			else{
				if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
                    LOG_INF("Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n", framerate, imgsensor_info.cap.max_framerate/10);
			frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
	    }
			if (imgsensor.frame_length > imgsensor.shutter) set_dummy();
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter) set_dummy();
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter) set_dummy();
			break;
		default:  /* coding with  preview scenario by default */
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter) set_dummy();
			LOG_INF("error scenario_id = %d, we use preview scenario\n", scenario_id);
			break;
	}
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(
			enum MSDK_SCENARIO_ID_ENUM scenario_id,
			MUINT32 *framerate)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*framerate = imgsensor_info.pre.max_framerate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*framerate = imgsensor_info.normal_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*framerate = imgsensor_info.cap.max_framerate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*framerate = imgsensor_info.hs_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*framerate = imgsensor_info.slim_video.max_framerate;
			break;
		default:
			break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);
	if (enable) {
		write_cmos_sensor_8(0x0601, 0x01);
		write_cmos_sensor_8(0x0602, 0x00);
		write_cmos_sensor_8(0x0604, 0x00);
		write_cmos_sensor_8(0x0606, 0x00);
		write_cmos_sensor_8(0x0608, 0x00);
	} else{
		write_cmos_sensor_8(0x0601, 0x00);
		write_cmos_sensor_8(0x0602, 0x00);
		write_cmos_sensor_8(0x0604, 0x00);
		write_cmos_sensor_8(0x0606, 0x00);
		write_cmos_sensor_8(0x0608, 0x00);
	}
#if 0
/* enable = false; */
	if (enable) {
		/* 0x5E00[8]: 1 enable,  0 disable */
		/* 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK */
    write_cmos_sensor_8(0x3200, 0x00);
    write_cmos_sensor_8(0x3462, 0x00);
    write_cmos_sensor_8(0x3230, 0x01);
    write_cmos_sensor_8(0x3290, 0x01);
    write_cmos_sensor_8(0x3201, 0x01);
    write_cmos_sensor_8(0x0b05, 0x00);
    write_cmos_sensor_8(0x0b00, 0x00);
    write_cmos_sensor_8(0x3400, 0x00);
    write_cmos_sensor_8(0x3C0F, 0x01);
    write_cmos_sensor_8(0x020E, 0x01);
    write_cmos_sensor_8(0x020F, 0x00);
    write_cmos_sensor_8(0x0210, 0x01);
    write_cmos_sensor_8(0x0211, 0x00);
    write_cmos_sensor_8(0x0212, 0x01);
    write_cmos_sensor_8(0x0213, 0x00);
    write_cmos_sensor_8(0x0214, 0x01);
    write_cmos_sensor_8(0x0215, 0x00);
    write_cmos_sensor_8(0x3c60, 0x00);
    write_cmos_sensor_8(0x0601, 0x01);
	} else {
		/* 0x5E00[8]: 1 enable,  0 disable */
		/* 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK */
		write_cmos_sensor_8(0x0601, 0x00);
	}
#endif
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static void check_streamoff(void)
{
	unsigned int i = 0;
	int timeout = (10000 / imgsensor.current_fps) + 1;

	mdelay(3);
	for (i = 0; i < timeout; i++) {
		if (read_cmos_sensor(0x0005) != 0xFF)
			mdelay(1);
		else
			break;
	}
	LOG_INF("%s exit!\n", __func__);
}
static kal_uint32 streaming_control(kal_bool enable)
{
	LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);

	if (enable) {	//Streaming on
		write_cmos_sensor_8(0x0100, 0x01);
	} else {
		// streaming OFF
		write_cmos_sensor_8(0x0100, 0x00);
		check_streamoff();
	}
	return ERROR_NONE;
}
static kal_uint32 feature_control(
			MSDK_SENSOR_FEATURE_ENUM feature_id,
			UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
    unsigned long long *feature_data = (unsigned long long *) feature_para;
    kal_uint32 rate;
    /* unsigned long long *feature_return_para=(unsigned long long *) feature_para; */

	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data = (MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	LOG_INF("feature_id = %d", feature_id);
	switch (feature_id) {
		case SENSOR_FEATURE_GET_PERIOD:
			*feature_return_para_16++ = imgsensor.line_length;
			*feature_return_para_16 = imgsensor.frame_length;
			*feature_para_len = 4;
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            LOG_INF("feature_Control imgsensor.pclk = %d,imgsensor.current_fps = %d\n", imgsensor.pclk, imgsensor.current_fps);
			*feature_return_para_32 = imgsensor.pclk;
			*feature_para_len = 4;
			break;
		case SENSOR_FEATURE_SET_ESHUTTER:
	    set_shutter(*feature_data);
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			break;
		case SENSOR_FEATURE_SET_GAIN:
	    set_gain((UINT16) *feature_data);
			break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
			break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			if ((sensor_reg_data->RegData>>8) > 0)
			   write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
			else
				write_cmos_sensor_8(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
			break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			/* get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE */
			/* if EEPROM does not exist in camera module. */
			*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
			*feature_para_len = 4;
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
	    set_video_mode(*feature_data);
			break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			get_imgsensor_id(feature_return_para_32);
			break;
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			set_auto_flicker_mode((BOOL)*feature_data_16, *(feature_data_16+1));
			break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		set_max_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM)*feature_data,
			*(feature_data+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		get_default_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM)*(feature_data),
			(MUINT32 *)(uintptr_t)(*(feature_data+1)));
			break;
		case SENSOR_FEATURE_SET_TEST_PATTERN:
	    set_test_pattern_mode((BOOL)*feature_data);
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: /* for factory mode auto testing */
			*feature_return_para_32 = imgsensor_info.checksum_value;
			*feature_para_len = 4;
			break;
		case SENSOR_FEATURE_SET_FRAMERATE:
			spin_lock(&imgsensor_drv_lock);
	    imgsensor.current_fps = *feature_data;
			spin_unlock(&imgsensor_drv_lock);
			break;
		case SENSOR_FEATURE_SET_HDR:
			LOG_INF("Warning! Not Support IHDR Feature");
			spin_lock(&imgsensor_drv_lock);
	    imgsensor.ihdr_en = KAL_FALSE;
			spin_unlock(&imgsensor_drv_lock);
			break;
		case SENSOR_FEATURE_GET_CROP_INFO:
		wininfo = (struct SENSOR_WINSIZE_INFO_STRUCT *)
			(uintptr_t)(*(feature_data+1));

			switch (*feature_data_32) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					memcpy((void *)wininfo,
						(void *)&imgsensor_winsize_info[1],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					memcpy((void *)wininfo,
						(void *)&imgsensor_winsize_info[2],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					memcpy((void *)wininfo,
						(void *)&imgsensor_winsize_info[3],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
					memcpy((void *)wininfo,
						(void *)&imgsensor_winsize_info[4],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				default:
					memcpy((void *)wininfo,
						(void *)&imgsensor_winsize_info[0],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			}
	    break;
		case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
            LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n", (UINT16)*feature_data, (UINT16)*(feature_data+1), (UINT16)*(feature_data+2));
            ihdr_write_shutter_gain((UINT16)*feature_data, (UINT16)*(feature_data+1), (UINT16)*(feature_data+2));
			break;
		case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
			streaming_control(KAL_FALSE);
			break;
		case SENSOR_FEATURE_SET_STREAMING_RESUME:
			if (*feature_data != 0)
				set_shutter(*feature_data);
			streaming_control(KAL_TRUE);
			break;
		case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
			switch (*feature_data) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					rate = imgsensor_info.cap.mipi_pixel_rate;
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					rate = imgsensor_info.normal_video.mipi_pixel_rate;
					break;
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					rate = imgsensor_info.hs_video.mipi_pixel_rate;
					break;
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
					rate = imgsensor_info.slim_video.mipi_pixel_rate;
					break;
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				default:
					rate = imgsensor_info.pre.mipi_pixel_rate;
					break;
			}
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = rate;
			break;
		default:
			break;
	}

	return ERROR_NONE;
}	/*	feature_control()  */

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 S5K4H7SUB_QT_P410AE_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc =  &sensor_func;
	return ERROR_NONE;
}
