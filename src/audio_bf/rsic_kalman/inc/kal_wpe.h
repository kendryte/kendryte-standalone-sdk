/*************************************************************************************
* Copy right:   2012-, canaan
* File Name:    kal_wpe.h
* Description:  header of kal_wpe.c
* History:      10/16/2018    Originated by xiexin
*************************************************************************************/

#ifndef INC_KAL_WPE_H_
#define INC_KAL_WPE_H_
//#include "nds32_basic_math.h"
//#include "nds32_complex_math.h"
//#include "nds32_utils_math.h"
//#include "nds32_transform_math.h"
//#include "nds32_matrix_math.h"
//#include "nds32_complex_matrix_math.h"
//#include "fastmath.h"
#include "fxp_define.h"
#include "complex_matrix_math.h"
#include <malloc.h>
#define BITWIDTH        (32)                // bit width
#define KALMAN_FWL      (25)                // fwl of kalman
#define INV_FWL         (28)                // fwl of inv matrix
#define FDNDLP_FREQ     (0)                 // input/output interface, 1:freqdata, 0:timedata
#define FDNDLP_K        (512)               // The number of subbands
#define NRE             ((FDNDLP_K/2)+1)    // number of process subbands
#define FDNDLP_N        (256)               // time shift
#define FDNDLP_D        (1)                 // prediction delay:seperate early and late delay
#define LOG2_NUMMIC     (0)                 // log2(FDNDLP_MIC)
#define FDNDLP_MIC      (1<<(LOG2_NUMMIC))  // Number of microphone
#define FDNDLP_LC       (10)                // order of frames
#define FDNDLP_EPS      (((1e-4)*((INT64)1<<(BITWIDTH-1))))              // lower bound of rho2
#define FDNDLP_ITER     (2)                 // number of iteration
#define FDNDLP_FRAME    (100)               // number of frames
#define FDNDLP_WINFLAG  (1)                 // 0:without hanning window, 1:with hanning window
#define FDNDLP_ALPHA    ((0.99)*( (INT64) 1<<(BITWIDTH-1)))    // alpha factor of kalman
#define ALPHA_REC       (0x014afd6a)        // fractional part of 1/alpha
#define FXPONE          (((long long)1<<(INV_FWL)))// 1 with config bit width
// total samples of a channel
#if FDNDLP_FREQ
#define FDNDLP_LEN      (FDNDLP_FRAME*FDNDLP_K)
#else
#define FDNDLP_LEN      (FDNDLP_FRAME*FDNDLP_N+FDNDLP_K-FDNDLP_N)
//#define FDNDLP_LEN      128337
#endif

typedef struct

{
    INT32 (*InvCovRe)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC*FDNDLP_LC];
    INT32 (*InvCovIm)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC*FDNDLP_LC];
    INT32 (*FiltTapsRe)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC];
    INT32 (*FiltTapsIm)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC];
    INT32 (*YSetpRe)[NRE][FDNDLP_MIC];
    INT32 (*YSetpIm)[NRE][FDNDLP_MIC];
    INT32 (*YWinRe)[FDNDLP_LC];
    INT32 (*YWinIm)[FDNDLP_LC];
    INT32 *PredRe;
    INT32 *PredIm;
    INT32 *KalRe;
    INT32 *KalIm;
    INT32 *TmpKal;
    INT32 *NormRe;
    INT32 *NormIm;
    INT32 *TmpInvRe;
    INT32 *TmpInvIm;
    INT32 *TmpMatRe;
    INT32 *TmpMatIm;
    UINT32 iFram;
    UINT32 CircFram;
    INT32 StartFlag;
}KAL_BUF_STR;


KAL_BUF_STR * kal_state_init();
void kalman_wpe(INT16 FreqDataRe[NRE][FDNDLP_MIC],      INT16 FreqDataIm[NRE][FDNDLP_MIC], KAL_BUF_STR *KalBufStr);
void kal_state_destroy(KAL_BUF_STR *KalBufStr);
#endif /* INC_KAL_WPE_H_ */
