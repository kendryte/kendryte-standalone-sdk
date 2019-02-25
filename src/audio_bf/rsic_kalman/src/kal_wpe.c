/**
* @file         kal_wpe.c
* @brief        kalman wpe alg.
* @author       xiexin
* @date         10/16/2018
* @copyright    canaan
*/
#include <string.h>
#include "../inc/kal_wpe.h"

void filter_update(INT32 *FiltTapsRe, 
                          INT32 *FiltTapsIm, 
                          INT32 *KalRe,     // Q(32,KALMAN_FWL)
                          INT32 *KalIm, 
                          INT32 *PredRe, 
                          INT32 *PredIm, 
                          INT32 *TmpInvRe, 
                          INT32 *TmpInvIm)
{
    UINT32 iRow;
    // (a+bj)*conj(x+yj)=(ax+by)+(bx-ay)j -> (b+aj)*(x+yj)=(bx-ay)+(ax+by)j
    //cmat_mul_q31(KalIm, KalRe, PredRe, PredIm, TmpInvIm, TmpInvRe, FDNDLP_MIC*FDNDLP_LC, 1, FDNDLP_MIC);
    for (iRow = 0; iRow < FDNDLP_MIC*FDNDLP_LC; ++iRow)
    {
        cmat_cscale_q31(PredRe, PredIm, KalIm[iRow], KalRe[iRow], 0, \
            &TmpInvIm[iRow*FDNDLP_MIC], &TmpInvRe[iRow*FDNDLP_MIC], \
            1, FDNDLP_MIC);
    }
    sl_shift_q31(TmpInvRe, BITWIDTH-1-KALMAN_FWL, TmpInvRe, FDNDLP_MIC*FDNDLP_LC*FDNDLP_MIC);// Q(32,KALMAN_FWL)->Q(32,31)
    sl_shift_q31(TmpInvIm, BITWIDTH-1-KALMAN_FWL, TmpInvIm, FDNDLP_MIC*FDNDLP_LC*FDNDLP_MIC);
    cmat_add_q31(FiltTapsRe, FiltTapsIm, TmpInvRe, TmpInvIm, FiltTapsRe, FiltTapsIm, FDNDLP_MIC*FDNDLP_LC, FDNDLP_MIC);
}


void inv_update(INT32 *InvCovRe, 
                     INT32 *InvCovIm, 
                     INT32 *KalRe,      // Q(32,KALMAN_FWL)
                     INT32 *KalIm, 
                     INT32 *YWinRe, 
                     INT32 *YWinIm, 
                     INT32 *TmpInvRe,   // Q(32,INV_FWL)
                     INT32 *TmpInvIm, 
                     INT32 *TmpMatRe, 
                     INT32 *TmpMatIm)
{
    //INT32 AlphaRec = div_q31(((long long)1<<(BITWIDTH-1))-(INT32)FDNDLP_ALPHA, (INT32)FDNDLP_ALPHA);//fractional part
    UINT32 iRow;
    // conj(a+bj)*(x+yj)=(ax+by)+(ay-bx)j -> (a+bj)*(y+xj)=(ay-bx)+(ax+by)j
    cmat_mul_q31(YWinRe, YWinIm, InvCovIm, InvCovRe, TmpMatIm, TmpMatRe, 1, FDNDLP_MIC*FDNDLP_LC, FDNDLP_MIC*FDNDLP_LC);// Q(32,INV_FWL)
    cmat_mul_q31(KalRe, KalIm, TmpMatRe, TmpMatIm, TmpInvRe, TmpInvIm, FDNDLP_MIC*FDNDLP_LC, 1, FDNDLP_MIC*FDNDLP_LC);
    // for (iRow = 0; iRow < FDNDLP_MIC*FDNDLP_LC; ++iRow)
    // {
    //     cmat_cscale_q31(TmpMatRe, TmpMatIm, KalRe[iRow], KalIm[iRow], 0, 
    //         &TmpInvRe[iRow*FDNDLP_MIC*FDNDLP_LC], &TmpInvIm[iRow*FDNDLP_MIC*FDNDLP_LC], 
    //         1, FDNDLP_MIC*FDNDLP_LC);
    // }// Q(32,KALMAN_FWL+INV_FWL-31)
    
    // TmpMatRe&TmpMatIm: Q(32,KALMAN_FWL+INV_FWL-31)->Q(32,INV_FWL)
    sl_shift_q31(TmpInvRe, BITWIDTH-1-KALMAN_FWL, TmpInvRe, FDNDLP_MIC*FDNDLP_LC*FDNDLP_MIC*FDNDLP_LC);
    sl_shift_q31(TmpInvIm, BITWIDTH-1-KALMAN_FWL, TmpInvIm, FDNDLP_MIC*FDNDLP_LC*FDNDLP_MIC*FDNDLP_LC);
    
    cmat_sub_q31(InvCovRe, InvCovIm, TmpInvRe, TmpInvIm, InvCovRe, InvCovIm, FDNDLP_MIC*FDNDLP_LC, FDNDLP_MIC*FDNDLP_LC);
    cmat_scale_q31(InvCovRe, InvCovIm, ALPHA_REC, 0, TmpInvRe, TmpInvIm, FDNDLP_MIC*FDNDLP_LC, FDNDLP_MIC*FDNDLP_LC);
    cmat_add_q31(InvCovRe, InvCovIm, TmpInvRe, TmpInvIm, InvCovRe, InvCovIm,FDNDLP_MIC*FDNDLP_LC, FDNDLP_MIC*FDNDLP_LC);
}

void kalman_update(INT32 *KalRe,            // Q(32,KALMAN_FWL)
                         INT32 *KalIm, 
                         INT32 *TmpKal,
                         INT32 *InvCovRe, 	// Q(32,INV_FWL)
                         INT32 *InvCovIm, 
                         INT32 *YWinRe, 
                         INT32 *YWinIm, 
                         INT32 *NormRe,
                         INT32 *NormIm,
                         INT32 YPow)
{
    INT32 DNormRe,DNormIm,TmpData,TmpData1;
    INT64 Tmp63D;
    UINT32 iOrd;
    INT32 Msb,ShiftB;
    // norm calc NormRe,NormIm->Q(32,INV_FWL)
    cmat_mul_q31(InvCovRe, InvCovIm, YWinRe, YWinIm, NormRe, NormIm, FDNDLP_MIC*FDNDLP_LC, FDNDLP_MIC*FDNDLP_LC, 1);
    // Q(32,INV_FWL)->Q(32,31)
    sl_shift_q31(NormRe, 31-INV_FWL, NormRe, FDNDLP_MIC*FDNDLP_LC);
    sl_shift_q31(NormIm, 31-INV_FWL, NormIm, FDNDLP_MIC*FDNDLP_LC);
    // denorm calc
    // DNormRe = FDNDLP_ALPHA * YPow;
    DNormRe = (INT32)FDNDLP_ALPHA;
    DNormRe = mul_q31_inline(DNormRe, YPow);
    DNormIm = 0;
    // (a-bj)*(x+yj)=(ax+by)+(ay-bx)j
    Tmp63D = dprod_q31(YWinRe, NormRe, FDNDLP_MIC*FDNDLP_LC);//Q(64,48)
    TmpData = (INT32)SAT(Tmp63D >> (48-31), MAX_S32, MIN_S32);
    DNormRe += TmpData;
    Tmp63D = dprod_q31(YWinIm, NormIm, FDNDLP_MIC*FDNDLP_LC);
    TmpData = (INT32)SAT(Tmp63D >> (48-31), MAX_S32, MIN_S32);
    DNormRe += TmpData;
    Tmp63D = dprod_q31(YWinRe, NormIm, FDNDLP_MIC*FDNDLP_LC);
    TmpData = (INT32)SAT(Tmp63D >> (48-31), MAX_S32, MIN_S32);
    DNormIm += TmpData;
    Tmp63D = dprod_q31(YWinIm, NormRe, FDNDLP_MIC*FDNDLP_LC);
    TmpData = (INT32)SAT(Tmp63D >> (48-31), MAX_S32, MIN_S32);
    DNormIm -= TmpData;
    // kalman calc
//    TmpData = DNormRe*DNormRe + DNormIm*DNormIm;
//    TmpData = 1/TmpData;
//    DNormRe = DNormRe * TmpData;
//    DNormIm = -DNormIm * TmpData;
    Msb = calc_max_msb(&DNormRe, 1);
    Msb = MAX(calc_max_msb(&DNormIm, 1),Msb);
    ShiftB = BITWIDTH - 1 - Msb;
    DNormRe = (DNormRe << ShiftB);         // Q(32,31)->Q(32,31+ShiftB)
    DNormIm = (DNormIm << ShiftB);
    // TmpData = DNormRe*DNormRe + DNormIm*DNormIm;
    TmpData = mul_q31_inline(DNormRe, DNormRe);     // Q(32,31+ShiftB)*Q(32,31+ShiftB)=Q(32,31+2*ShiftB)
    TmpData1 = mul_q31_inline(DNormIm, DNormIm);
    TmpData = add_q31_inline(TmpData, TmpData1);
    // TmpData = 1/TmpData;
    // 1<<(BITWIDTH-3): Q(32,(BITWIDTH-3))=Q(32,x)
    // In TmpData: Q(32,31+2*ShiftB)=Q(32,y)
    // Out TmpData: Q(32,29-ShiftB)=Q(32,31+x-y)
    
    // TmpData = div_q31(1<<(BITWIDTH-3),TmpData);   // Q(32,29-2*ShiftB)
    TmpData = (((INT64)1<<(2*BITWIDTH-4))/TmpData);   // Q(32,29-2*ShiftB)
    // DNormRe = DNormRe * TmpData;
    DNormRe = mul_q31_inline(DNormRe, TmpData);     // Q(32,31+ShiftB)*Q(32,29-2*ShiftB)=Q(32,29-ShiftB)
    // DNormIm = DNormIm * TmpData;
    DNormIm = mul_q31_inline(DNormIm, TmpData);
    
    
// Q(32,31)*Q(32,29-ShiftB)=Q(32,29-ShiftB)
//    for (iOrd = 0; iOrd < FDNDLP_MIC*FDNDLP_LC; ++iOrd)
//    {
//        KalRe[iOrd] = NormRe[iOrd] * DNormRe;
//        KalRe[iOrd] -= NormIm[iOrd] * DNormIm;
//        KalIm[iOrd] = NormRe[iOrd] * DNormIm;
//        KalIm[iOrd] += NormIm[iOrd] * DNormRe;
//    }
    scale_q31(NormRe, DNormRe, 0, KalRe, FDNDLP_MIC*FDNDLP_LC);
    scale_q31(NormIm, -DNormIm, 0, TmpKal, FDNDLP_MIC*FDNDLP_LC);
    sub_q31(KalRe, TmpKal, KalRe, FDNDLP_MIC*FDNDLP_LC);
    scale_q31(NormRe, -DNormIm, 0, KalIm, FDNDLP_MIC*FDNDLP_LC);
    scale_q31(NormIm, DNormRe, 0, TmpKal, FDNDLP_MIC*FDNDLP_LC);
    add_q31(KalIm, TmpKal, KalIm, FDNDLP_MIC*FDNDLP_LC);
    // Q(32,29-ShiftB)->Q(32,KALMAN_FWL)
    ShiftB = KALMAN_FWL - (BITWIDTH - 3 -ShiftB);
    sl_shift_q31(KalRe, ShiftB, KalRe, FDNDLP_MIC*FDNDLP_LC);
    sl_shift_q31(KalIm, ShiftB, KalIm, FDNDLP_MIC*FDNDLP_LC);
    
}
 void wpe_filt(INT32 *PredRe,
                  INT32 *PredIm,
                  INT32 *YSetpRe,
                  INT32 *YSetpIm,
                  INT32 *YWinRe,
                  INT32 *YWinIm,
                  INT32 *FiltTapsRe,
                  INT32 *FiltTapsIm)
{
    // (a+bj)*conj(x+yj)=(ax+by)+(bx-ay)j -> (b+aj)*(x+yj)=(ax+by)j+(bx-ay)
    cmat_mul_q31(YWinIm, YWinRe, FiltTapsRe, FiltTapsIm, PredIm, PredRe, 1, FDNDLP_MIC*FDNDLP_LC, FDNDLP_MIC);
    cmat_sub_q31(YSetpRe, YSetpIm, PredRe, PredIm, PredRe, PredIm, FDNDLP_MIC, 1);
}


void y_win_get(INT32 YWinRe[FDNDLP_MIC][FDNDLP_LC],
                   INT32 YWinIm[FDNDLP_MIC][FDNDLP_LC],
                   INT32 YSetpRe[FDNDLP_LC+FDNDLP_D+1][NRE][FDNDLP_MIC],
                   INT32 YSetpIm[FDNDLP_LC+FDNDLP_D+1][NRE][FDNDLP_MIC],
                   UINT32 CircFram,UINT32 iRe)
{
    UINT32 iMic,iLc;
    INT32 iFram;

    iFram = CircFram-FDNDLP_D-1;
    if(iFram<0)
        iFram += FDNDLP_LC+FDNDLP_D+1;
    for (iLc = 0; iLc < FDNDLP_LC; ++iLc)
    {
        for (iMic = 0; iMic < FDNDLP_MIC; ++iMic)
        {
            YWinRe[iMic][iLc] = YSetpRe[iFram][iRe][iMic];
            YWinIm[iMic][iLc] = YSetpIm[iFram][iRe][iMic];
        }
        iFram--;
        if(iFram<0)
            iFram = FDNDLP_LC+FDNDLP_D;
    }
}


KAL_BUF_STR * kal_state_init()
{
    UINT32 iRe,ii;
    KAL_BUF_STR *pBuf;

    pBuf = (KAL_BUF_STR *)malloc(sizeof(KAL_BUF_STR));
    pBuf->InvCovRe = (INT32 (*)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC*FDNDLP_LC])malloc(sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC*FDNDLP_LC));
    memset(&pBuf->InvCovRe[0][0][0], 0, sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC*FDNDLP_LC));
    for (iRe = 0; iRe < NRE; ++iRe)
    {
        for (ii = 0; ii < FDNDLP_MIC*FDNDLP_LC; ++ii)
        {
            pBuf->InvCovRe[iRe][ii][ii] = FXPONE;
        }
    }
    pBuf->FiltTapsRe = (INT32 (*)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC])malloc(sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC));
    memset(&pBuf->FiltTapsRe[0][0][0], 0, sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC));
    pBuf->YSetpRe = (INT32 (*)[NRE][FDNDLP_MIC])malloc(sizeof(INT32)*(FDNDLP_LC+FDNDLP_D+1)*NRE*FDNDLP_MIC);

    pBuf->InvCovIm = (INT32 (*)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC*FDNDLP_LC])malloc(sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC*FDNDLP_LC));
    memset(&pBuf->InvCovIm[0][0][0], 0, sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC*FDNDLP_LC));
    
    pBuf->FiltTapsIm = (INT32 (*)[FDNDLP_MIC*FDNDLP_LC][FDNDLP_MIC])malloc(sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC));
    memset(&pBuf->FiltTapsIm[0][0][0], 0, sizeof(INT32)*NRE*(FDNDLP_MIC*FDNDLP_LC)*(FDNDLP_MIC));
    pBuf->YSetpIm = (INT32 (*)[NRE][FDNDLP_MIC])malloc(sizeof(INT32)*(FDNDLP_LC+FDNDLP_D+1)*NRE*FDNDLP_MIC);
    pBuf->YWinRe = (INT32 (*)[FDNDLP_LC])malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->YWinIm = (INT32 (*)[FDNDLP_LC])malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->PredRe = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC);
    pBuf->PredIm = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC);
    pBuf->KalRe = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->KalIm = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->TmpKal = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->NormRe = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->NormIm = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->TmpInvRe = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC*FDNDLP_MIC*FDNDLP_LC);
    pBuf->TmpInvIm = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC*FDNDLP_MIC*FDNDLP_LC);
    pBuf->TmpMatRe = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->TmpMatIm = (INT32 *)malloc(sizeof(INT32)*FDNDLP_MIC*FDNDLP_LC);
    pBuf->iFram = 0;
    pBuf->CircFram = 0;
    pBuf->StartFlag = 0;
    return pBuf;
}

void kal_state_destroy(KAL_BUF_STR *KalBufStr)
{
    free(KalBufStr->InvCovRe);
    free(KalBufStr->InvCovIm);
    free(KalBufStr->FiltTapsRe);
    free(KalBufStr->FiltTapsIm);
    free(KalBufStr->YSetpRe);
    free(KalBufStr->YSetpIm);
    free(KalBufStr->YWinRe);
    free(KalBufStr->YWinIm);
    free(KalBufStr->PredRe);
    free(KalBufStr->PredIm);
    free(KalBufStr->KalRe);
    free(KalBufStr->KalIm);
    free(KalBufStr->TmpKal);
    free(KalBufStr->NormRe);
    free(KalBufStr->NormIm);
    free(KalBufStr->TmpInvRe);
    free(KalBufStr->TmpInvIm);
    free(KalBufStr->TmpMatRe);
    free(KalBufStr->TmpMatIm);
    free(KalBufStr);
}

void kal_state_update(KAL_BUF_STR *KalBufStr)
{
    KalBufStr->iFram++;
    KalBufStr->CircFram++;
    if (KalBufStr->CircFram==FDNDLP_LC+FDNDLP_D+1)
    {
        KalBufStr->CircFram = 0;
    }
    if (KalBufStr->iFram > FDNDLP_LC+FDNDLP_D-1)
    {
        KalBufStr->StartFlag = 1;
    }
}

void kalman_wpe(INT16 FreqDataRe[NRE][FDNDLP_MIC],
                  INT16 FreqDataIm[NRE][FDNDLP_MIC],
                  KAL_BUF_STR *KalBufStr)
{
    INT32 YPow;
    INT32 TmpPow;

    
    UINT32 iFram,iRe,iMic,ii,CircFram;

    CircFram = KalBufStr->CircFram;
    // read freq data to buffer
    for (iRe = 0; iRe < NRE; ++iRe)
    {
        for (iMic = 0; iMic < FDNDLP_MIC; ++iMic)
        {
            KalBufStr->YSetpRe[CircFram][iRe][iMic] = FreqDataRe[iRe][iMic] << 16;
            KalBufStr->YSetpIm[CircFram][iRe][iMic] = FreqDataIm[iRe][iMic] << 16;
        }
    }

    if (KalBufStr->StartFlag)
    {
        for (iRe = 0; iRe < NRE; ++iRe)
        {
            // YPow calc
            YPow = 0;
            for (iMic = 0; iMic < FDNDLP_MIC; ++iMic)
            {
                //YPow = YPow + YSetpRe[CircFram][iRe][iMic] * YSetpRe[CircFram][iRe][iMic];
                //YPow = YPow + YSetpIm[CircFram][iRe][iMic] * YSetpIm[CircFram][iRe][iMic];
                TmpPow = mul_q31_inline(KalBufStr->YSetpRe[CircFram][iRe][iMic], \
                    KalBufStr->YSetpRe[CircFram][iRe][iMic]);
                YPow = add_q31_inline(YPow, TmpPow);
                TmpPow = mul_q31_inline(KalBufStr->YSetpIm[CircFram][iRe][iMic], \
                    KalBufStr->YSetpIm[CircFram][iRe][iMic]);
                YPow = add_q31_inline(YPow, TmpPow);
            }
            YPow = (YPow >> LOG2_NUMMIC);
            YPow = (YPow<FDNDLP_EPS)? (FDNDLP_EPS) : (YPow);
            // YPow = YPow/FDNDLP_MIC;
            // YWin
            y_win_get(KalBufStr->YWinRe,KalBufStr->YWinIm,KalBufStr->YSetpRe,KalBufStr->YSetpIm,CircFram,iRe);
            // filter
            wpe_filt(KalBufStr->PredRe,KalBufStr->PredIm,KalBufStr->YSetpRe[CircFram][iRe],KalBufStr->YSetpIm[CircFram][iRe],\
                &KalBufStr->YWinRe[0][0],&KalBufStr->YWinIm[0][0],&KalBufStr->FiltTapsRe[iRe][0][0],&KalBufStr->FiltTapsIm[iRe][0][0]);
            // kalman update
            kalman_update(KalBufStr->KalRe, KalBufStr->KalIm, KalBufStr->TmpKal, \
                &KalBufStr->InvCovRe[iRe][0][0], &KalBufStr->InvCovIm[iRe][0][0], &KalBufStr->YWinRe[0][0], &KalBufStr->YWinIm[0][0], \
                KalBufStr->NormRe, KalBufStr->NormIm, YPow);
            // inv update
            inv_update(&KalBufStr->InvCovRe[iRe][0][0], &KalBufStr->InvCovIm[iRe][0][0], KalBufStr->KalRe, KalBufStr->KalIm, \
                &KalBufStr->YWinRe[0][0], &KalBufStr->YWinIm[0][0], KalBufStr->TmpInvRe, KalBufStr->TmpInvIm, \
                KalBufStr->TmpMatRe, KalBufStr->TmpMatIm);
            // filter update
            filter_update(&KalBufStr->FiltTapsRe[iRe][0][0], &KalBufStr->FiltTapsIm[iRe][0][0], KalBufStr->KalRe, KalBufStr->KalIm, \
                KalBufStr->PredRe, KalBufStr->PredIm, KalBufStr->TmpInvRe, KalBufStr->TmpInvIm);
            // kalman-wpe output
            FreqDataRe[iRe][iMic] = KalBufStr->PredRe[iMic] >> 16;
            FreqDataIm[iRe][iMic] = KalBufStr->PredIm[iMic] >> 16;
        }
    }

    // update state
    kal_state_update(KalBufStr);

    
}



