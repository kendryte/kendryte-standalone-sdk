#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#define  OV5640             1
#define  OV2640             0

#define  BOARD_KD233        1
#define  BOARD_LICHEEDAN    0

#if OV5640 + OV2640 != 1
#error ov sensor only choose one
#endif

#if BOARD_KD233 + BOARD_LICHEEDAN != 1
#error board only choose one
#endif

#define LCD_RST_PIN                         (37)
#define LCD_DCX_PIN                         (38)
#define LCD_WRX_PIN                         (36)
#define LCD_SCK_PIN                         (39)

#define LCD_DCX_HS_NUM                      (5)
#define LCD_RST_HS_NUM                      (6)


#define CAM_W                       (320)
#define CAM_H                       (240)

#endif
