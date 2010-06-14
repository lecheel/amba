#define VS_CLASS_TX		0x21
#define VS_CLASS_RX		0xA1

#define SET_CUR			0x01
#define GET_CUR			0x81

//get_chip_version
#define V_CHIP_VERSION 		0x0300
#define I_CHIP_VERSION		0xF000
#define	L_CHIP_VERSION		4

//get_firmware_version
#define V_FIRMWARE_VERSION 	0x0400
#define I_FIRMWARE_VERSION	0xF000

// encode_format
#define V_ENCODE_FORMAT 	0x0B00
#define I_ENCODE_FORMAT		0xF000

#define	L_ENCODE_FORMAT sizeof(ENCODE_FORMAT)

#define H264_FORMAT		0
#define MJPEG_FORMAT		1
#define H264_MJPEG_FORMAT	2
#define H264_TS_FORMAT		3
#define H264_AAC_FORMAT		4
#define H264_PCM_FORMAT		5
#define H264_MJPEG_AAC_FORMAT   6
#define MJPEG_AAC_FORMAT	7
#define MJPEG_PCM_FORMAT	8


// encode_size
#define V_ENCODE_SIZE 		0x1500
#define I_ENCODE_SIZE		0xF000

#define	L_ENCODE_SIZE sizeof(ENCODE_SIZE)

#define H264_SIZE_1920_1080	0
#define H264_SIZE_1440_1080	1
#define H264_SIZE_1280_720	2
#define H264_SIZE_640_480	3
#define H264_SIZE_352_288	10
#define MJPEG_SIZE_640_480	0
#define MJPEG_SIZE_352_288	5


//boot_dsp
#define V_BOOT_DSP	 	0x0600
#define I_BOOT_DSP		0xF000
#define	L_BOOT_DSP 		0
                        	
//vin_device            	
#define V_VIN_DEVICE		0x1600
#define I_VIN_DEVICE		0xF000
#define	L_VIN_DEVICE 		4
                        	
#define SENSOR			0
#define TV_DECODER		1
#define HD_SDI			2
                        	
//h264_bitrate          	
#define V_H264_BITRATE		0x1000
#define I_H264_BITRATE		0xF000
#define	L_H264_BITRATE 		4		//Integer number in kbps.

//Set/Get current frame rate control.          	
#define V_H264_FRAMERATE	0x0C00
#define I_H264_FRAMERATE	0xF000
#define	L_H264_FRAMERATE 	4		//0: default (*).
									//10 to 30 for Micron sensor
//vout_device           	
#define V_VOUT_DEVICE		0x1900
#define I_VOUT_DEVICE		0xF000
#define	L_VOUT_DEVICE 		4		
                        	
#define TV			0
#define LCD			1

//reboot_system          	
#define V_REBOOT_SYSTEM		0x0500
#define I_REBOOT_SYSTEM		0xF000
#define	L_REBOOT_SYSTEM 	0	
                        	
//start_encode          	
#define V_START_ENCODE		0x0700
#define I_START_ENCODE		0xF000
#define	L_START_ENCODE 		4		
                        	
#define H264_ENCODING		0
#define MJPEG_ENCODING		1
                        	
//stop_encode           	
#define V_STOP_ENCODE		0x0800
#define I_STOP_ENCODE		0xF000
#define	L_STOP_ENCODE 		4		
                        	
                        	
//image_brightness      	
#define V_BRIGHTNESS		0x2900
#define I_BRIGHTNESS		0xF000
#define	L_BRIGHTNESS 		4	

#define MAX_BRIGHTNESS		256
#define MIN_BRIGHTNESS		-256
#define DEFAULT_BRIGHTNESS	0

//image_contrast
#define V_CONTRAST		0x2A00
#define I_CONTRAST		0xF000
#define	L_CONTRAST 		4	

#define MAX_CONTRAST		256
#define MIN_CONTRAST		0
#define DEFAULT_CONTRAST	64

//image_hue
#define V_HUE			0x2B00
#define I_HUE			0xF000
#define	L_HUE 			4	
                        	
#define MAX_HUE			128
#define MIN_HUE			-128
#define DEFAULT_HUE		0

//image_sharpness
#define V_SHARPNESS		0x2D00
#define I_SHARPNESS		0xF000
#define	L_SHARPNESS 		4	

#define MAX_SHARPNESS		5
#define MIN_SHARPNESS		0
#define DEFAULT_SHARPNESS	0

//image_saturation
#define V_SATURATION		0x2C00
#define I_SATURATION		0xF000
#define	L_SATURATION 		4	

#define MAX_SATURATION		256
#define MIN_SATURATION		0
#define DEFAULT_SATURATION	64

//get_image_para
#define V_IMAGE_PARA		0x3100
#define I_IMAGE_PARA		0xF000

#define	L_IMAGE_PARA 		sizeof(AMBA_PARA)

//append_header
#define V_APPEND_HEADER		0x3900
#define I_APPEND_HEADER		0xF000
#define	L_APPEND_HEADER 	sizeof(APPEND_HEADER)

#define	ID_MAGIC_NUMBER		0xCC
#define	FORMAT_AUDIO 		0x00
#define FORMAT_H264_VIDEO	0x01
#define FORMAT_MJPG_VIDEO   	0x02
#define IDR_FRAME		0x01
#define I_FRAME			0x02
#define P_FRAME			0x03
#define B_FRAME			0x04
