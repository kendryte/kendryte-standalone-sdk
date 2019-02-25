/**
* @file         fxp_define.h
* @brief        header of fxp_define.c
* @author       xiexin
* @date         10/16/2018
* @copyright    canaan
*/
#ifndef _FXP_DEFINE_H
#define _FXP_DEFINE_H

#define VC_VER  0   /* 1: for window, 0: for unix or linux */

#if  VC_VER
typedef char                    INT8;
typedef short int               INT16;
typedef int                     INT32;
typedef __int64                 INT64;
typedef unsigned char           UINT8;
typedef unsigned short int      UINT16;
typedef unsigned int            UINT32;
typedef unsigned __int64        UINT64;

#else
typedef char                    INT8;
typedef short int               INT16;
typedef int                     INT32;
typedef long long               INT64;
typedef unsigned char           UINT8;
typedef unsigned short int      UINT16;
typedef unsigned int            UINT32;
typedef unsigned long long      UINT64;
#endif

typedef struct {
        int Re;
        int Im;
}CPLX_DATA;    

#define MAX_S16                                 ((INT16)0x7FFF)
#define MAX_S32                                 ((INT32)0x7FFFFFFF)
#define MAX_S64                                 ((INT64)0x7FFFFFFFFFFFFFFF)
#define MIN_S16                                 ((INT16)0x8000)
#define MIN_S32                                 ((INT32)0x80000000)
#define MIN_S64                                 ((INT64)0x8000000000000000)
#define MAX_U16                                 ((UINT16)0xFFFF)
#define MAX_U32                                 ((UINT32)0xFFFFFFFF)
#define MAX_U64                                 ((UINT64)0xFFFFFFFFFFFFFFFF)
#define MIN_U                                   (0)
#define SAT(a, max, min)                        ((a) > (max) ? (max) : ((a) < (min) ? (min) : (a)))
#define SAT_US(a, max)                          ((a) > (max) ? (max) : (a))
#define OVER_FLOW(a, max, min, sign)            (((sign)==0) ? ((a)&(max)) : ((a)|(min)) )
#define OVER_FLOW_US(a, max)                    (((a)>(max)) ? ((a)&(max)) : (a))

/* Get bit in b posstion b=1,2,...,Wl*/
#define BIT_GET(a, b)                           (((a)>>((b)-1))&1)
/* Set 1 in position b  b=1,2,...,Wl*/
#define BIT_SET_1(a, b)                         ((a)|(1<<((b)-1)))
/* Set 0 in position b  b=1,2,...,Wl*/
#define BIT_SET_0(a, b)                         (BIT_SET_1(a, b)-(1<<((b)-1)))
/* Set Bool c in b posstion b=1,2,...,Wl*/
#define BIT_SET(a, b, c)                        ((c)==0 ? BIT_SET_0(a, b): BIT_SET_1(a, b))

#define SIGN(a)                                 ((a) >= 0 ? (1) : (-1))
#define CIRC_INCR(a,bound)                      ((a) <= ((bound)-2) ? ((a)+1) : 0)
#define CIRC_DECR(a,bound)                      ((a) > 0  ? ((a)-1) : ((bound)-1))
#define MAX(a,b)                                ((a) > (b) ? (a) : (b))
#define MIN(a,b)                                ((a) < (b) ? (a) : (b)) 
#define ABS(a)                                  (((a)>=0) ? (a) : (-(a)))
#define ROUND_LAST_KBIT(a,k)                    ((k)==0 ? (a) : (((a)+(1<<((k)-1)))>>(k)))
#define POINT_ALIGN(a,srcFwl,desFwl,round)      (((srcFwl)>(desFwl)) ? (((a)+((round)<<((srcFwl)-(desFwl)-1))) >> ((srcFwl)-(desFwl))) : ((a) << ((desFwl)-(srcFwl))) )

/* Z=X*Y=(Xr+jXi)*(Yr+jYi)=XrYr-XiYi+j(XiYr+XrYi) */
#define CPLX_MUL(zr,zi,xr,xi,yr,yi)             do {*(zr)=(xr)*(yr)-(xi)*(yi); *(zi)=(xi)*(yr)+(xr)*(yi);} while(0)
/* Z=X*conj(Y)=(Xr+jXi)*(Yr-jYi)=XrYr+XiYi+j(XiYr-XrYi) */
#define CPLX_CONJMUL(zr,zi,xr,xi,yr,yi)         do {*(zr)=(xr)*(yr)+(xi)*(yi); *(zi)=(xi)*(yr)-(xr)*(yi);} while(0)

/*Function prototype*/
INT32 calc_max_msb(INT32 *in, INT32 dataLen);
void add_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 len);
void add_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 len);
void sub_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 len);
void sub_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 len);
void scale_q15(INT16 *src, INT16 scale, INT32 shift, INT16 *dst, UINT32 len);
void scale_q31(INT32 *src, INT32 scale, INT32 shift, INT32 *dst, UINT32 len);

void mat_add_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 row, UINT32 col);
void mat_add_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 row, UINT32 col);
void mat_sub_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 row, UINT32 col);
void mat_sub_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 row, UINT32 col);
void mat_scale_q15(INT16 *src, INT16 scale, INT32 shift, INT16 *dst, UINT32 row, UINT32 col);
void mat_scale_q31(INT32 *src, INT32 scale, INT32 shift, INT32 *dst, UINT32 row, UINT32 col);
void mat_mul_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 row, UINT32 col, UINT32 col2);
void mat_mul_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 row, UINT32 col, UINT32 col2);
void sl_shift_q15(INT16 *src, INT32 shift, INT16 *dst, UINT32 len);
void sl_shift_q31(INT32 *src, INT32 shift, INT32 *dst, UINT32 len);
INT64 dprod_q15(INT16 *src1, INT16 *src2, UINT32 len);
INT64 dprod_q31(INT32 *src1, INT32 *src2, UINT32 len);


INT16 add_q15_inline(INT16 a, INT16 b);
INT32 add_q31_inline(INT32 a, INT32 b);
INT16 sub_q15_inline(INT16 a, INT16 b);
INT32 sub_q31_inline(INT32 a, INT32 b);
INT16 mul_r_shift_q15_inline(INT16 a, INT16 b, INT32 shift);
INT32 mul_r_shift_q31_inline(INT32 a, INT32 b, INT32 shift);
INT16 mul_l_shift_q15_inline(INT16 a, INT16 b, INT32 shift);
INT32 mul_l_shift_q31_inline(INT32 a, INT32 b, INT32 shift);
INT16 mul_q15_inline(INT16 a, INT16 b);
INT32 mul_q31_inline(INT32 a, INT32 b);


#endif