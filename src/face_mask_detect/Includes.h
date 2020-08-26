#ifndef		__INCLUDES_H
#define		__INCLUDES_H

#include	"board_config.h"

/* spi > WiFi */
#define		SPI_DEVICE					SPI_DEVICE_1
#define		SPI_CS_SELECT				SPI_CHIP_SELECT_0
#define		SPI_DMA_TX_CHL				DMAC_CHANNEL3
#define		SPI_DMA_RX_CHL				DMAC_CHANNEL4

/* i2s > Led */
#define		LED_DEVICE					I2S_DEVICE_1
#define		LED_DEVICE_CHL				I2S_CHANNEL_3
#define		LED_DMA_TX_CHL				DMAC_CHANNEL2

/* IO map */
#if	 BOARD_KD233
#define 	W600_INTERRUPT_PIN   		27
#define 	W600_RESET_PIN       		24
#define		W600_SPI_CS_PIN				30
#define		W600_SPI_CLK_PIN			32
#define		W600_SPI_MOSI_PIN			34
#define		W600_SPI_MISO_PIN			31

#define		OV_RST_PIN					11
#define		OV_VSYNC_PIN				12
#define		OV_PWDN_PIN					13
#define		OV_HREF_PIN					17
#define		OV_PCLK_PIN					15
#define		OV_XCLK_PIN					14
#define		OV_SCCB_SCLK_PIN			10
#define		OV_SCCB_SDA_PIN				9	

#define		LCD_CS_PIN					6
#define		LCD_DC_PIN					8
#define		LCD_RW_PIN					7

#define		KEY_PIN						26
#define		BUZZ_PIN					46
#define		LED_PIN						40
#define		RELAY_PIN					42

#elif BOARD_GUARD
/* W600 */
#define 	W600_INTERRUPT_PIN   		11
#define 	W600_RESET_PIN       		6
#define		W600_SPI_CS_PIN				9
#define		W600_SPI_CLK_PIN			10
#define		W600_SPI_MOSI_PIN			7
#define		W600_SPI_MISO_PIN			8
/* OV2640 */
#define		OV_RST_PIN					18
#define		OV_VSYNC_PIN				19
#define		OV_PWDN_PIN					20
#define		OV_HREF_PIN					33
#define		OV_PCLK_PIN					34
#define		OV_XCLK_PIN					35
#define		OV_SCCB_SCLK_PIN			21
#define		OV_SCCB_SDA_PIN				23	
/* LCD */
#define		LCD_CS_PIN					28
#define		LCD_DC_PIN					27
#define		LCD_RW_PIN					26
#define		LCD_RST_PIN					29
/* KEY */
#define		KEY_PIN						44
/* BUZZ*/
#define		BUZZ_PIN					46
/* LED*/
#define		LED_PIN						45
/* RELAY*/
#define		RELAY_PIN					43

#elif  BOARD_PADDLEPI
/* W600 */
#define 	W600_INTERRUPT_PIN   		19
#define 	W600_RESET_PIN       		23
#define		W600_SPI_CS_PIN				20
#define		W600_SPI_CLK_PIN			18
#define		W600_SPI_MOSI_PIN			21
#define		W600_SPI_MISO_PIN			22

/* OV2640 */
#define		OV_RST_PIN					43
#define		OV_VSYNC_PIN				42
#define		OV_PWDN_PIN					45
#define		OV_HREF_PIN					44
#define		OV_PCLK_PIN					47
#define		OV_XCLK_PIN					46
#define		OV_SCCB_SCLK_PIN			41
#define		OV_SCCB_SDA_PIN				40	
/* LCD */
#define		LCD_CS_PIN					38
#define		LCD_DC_PIN					37
#define		LCD_RW_PIN					36
#define		LCD_RST_PIN					39
/* KEY */
#define		KEY_PIN						13 //35
#define		KEY_IR_PIN					32
/* BUZZ*/
#define		BUZZ_PIN					25
/* LED*/
#define		LED_PIN						10
/* RELAY*/
#define		RELAY_PIN					30
/* IR-CUT*/
#define		IR_CUT1_PIN					6
#define		IR_CUT2_PIN					7
#endif

/* IO */
#define 	W600_INTERRUPT_IO   		4
#define 	W600_RESET_IO        		3
#define		LCD_DC_IO					2
#define		LCD_RST_IO					7
#define		KEY_IO						8
#define		BUZZ_IO						9
#define		RELAY_IO					10
#define		KEY_IR_IO					11
#define		IR_CUT1_IO					12
#define		IR_CUT2_IO					13				

/* Tick*/
#define		TCIK_MS						10
#define		TICK_NANOSECONDS			(TCIK_MS*1000000)

/* FLASH */
#define		FLASH_TOTAL_SIZE			0x01000000
#define		DOOR_INFO_TOTAL_SIZE		0x00200000
#define		DEVICE_INFO_START_ADDR		0x00600000

#define		WLAN_INFO_START_ADDR		(DEVICE_INFO_START_ADDR + 0x1000)
#define		SERVER_INFO_START_ADDR		(WLAN_INFO_START_ADDR   + 0x1000)
#define		SYSTEM_MISC_START_ADDR		(SERVER_INFO_START_ADDR + 0x1000)
#define		FEATURE_INDEX_START_ADDR	(SYSTEM_MISC_START_ADDR + 0x1000)
#define		DOOR_INFO_START_ADDR		(FEATURE_INDEX_START_ADDR + 0x1000)
#define		FEATURE_INFO_START_ADDR		(DOOR_INFO_START_ADDR + DOOR_INFO_TOTAL_SIZE)

/*Device Information. */
#define		DEFAULT_FLAG_FLASH_VALID	0xA55A5AA5
#define		DEFALUT_AUTH_MODE			0x01
#define		DEFAULT_LENGTH				0x40
#define		DEFAULT_OPEN_D_SECOND		0x02
#define		DEFAULT_DID_VALUE			0x01
#define		DEFAULT_WLAN_SSID			"DJF5939"
#define		DEFAULT_WLAN_PASSWORD		"12345678"
#define		DEFAULT_SERVER_IP_ADDR		"192.168.137.1"
#define		DEFAULT_SERVER_PORT_ADDR	30001
#define		DEFAULT_READ_COUNT			0x10
#define		DEFAULT_DEVICE_NAME			"K210 AI-FDM"
#define		DEFALUT_ALIVE_CHECK			0x00
#define		DEFAULT_FIRMWARE_VERSION	"v1.0.0"

#define		ZBAR_INFORMATION_SSID		"{ssid:"
#define		ZBAR_INFORMATION_PASSWD		"{password:"
#define		ZBAR_INFORMATION_SERVER		"{server:"
#define		ZBAR_INFORMATION_PORT		"{prot:"
#define		ZBAR_INFORMATION_DID		"{did:"
#define		ZBAR_INFORMATION_NAME		"{name:"
#define		ZBAR_INFORMATION_END_MARK 	"}"

/*RTC Init time*/
#define		RTC_YEAR_BASE				2000
#define		RTC_YEAR					19
#define		RTC_MONTH					3
#define		RTC_DAY						5
#define		RTC_HOUR					9
#define		RTC_MINUTE					34
#define		RTC_SECOND					0

/* debug switch. */
//#define		DEBUG_CORE1_ONLY
//#define		DEBUG_FACE_RECOGNITION_EXCLUDE

#include	"stdio.h"
#include	"entry.h"
#include 	"fpioa.h"
#include 	"sysctl.h"
#include 	"gpiohs.h"
#include 	"gpio.h"
#include 	"spi.h"
#include 	"uarths.h"
#include	"timer.h"
#include 	"w25qxx.h"
#include 	"rtc.h"

#define		ASSERT(ex)				while(!(ex));


#define	FIRMWARE_VERSION_NUMBER		((uint64_t)0)
#endif
