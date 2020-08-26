#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#define  OV5640             1
#define  OV2640             0

#define  BOARD_KD233        1
#define  BOARD_LICHEEDAN    0
#define	 BOARD_PADDLEPI		0

#if OV5640 + OV2640 != 1
#error ov sensor only choose one
#endif

#if BOARD_KD233 + BOARD_LICHEEDAN + BOARD_PADDLEPI!= 1
#error board only choose one
#endif

#endif
