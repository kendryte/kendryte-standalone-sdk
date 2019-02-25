/****************************************************************
*  COPYRIGHT LeadCoreTech CO.,LTD	                            *
*****************************************************************/
/****************************************************************
* FileName:    <lib_typedef_N.h>
* version:     <1.0.0>
* Purpose:     <define the data type and macros>
* Authors:     <Yan Dong>
* Notes:       <None>		 
****************************************************************/

/******************************************************************************
*  HISTORY OF CHANGES
*   <Date>          <Author>        <Version>       <DESCRIPTION>
*   2006-06         Yan Dong         V1.0.0           original
******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

#undef FLTPNT

#ifndef LIB_TYPEDEF_N_H
#define LIB_TYPEDEF_N_H
	
/*-----------including external files -----------------------------*/
	
/*-----------macro declaration-------------------------------------*/
	
/*-----------global variable declaration---------------------------*/
	
/*-----------constant and type declaration-------------------------*/
	
/*-----------file-local variable declaration-----------------------*/
	
/*-----------Function or task prototype declaration-----------------*/
	
#if !defined(_LANGUAGE_ASM) /* Added to Bypass the C typedefs for assembler */

/*
* Target dependent macros.
*
* The macros __LITTLE_END__  and __BIGEND__ are set here
* based on target platform, and indicate if the smallest indexed
* byte comes first, or last.
* 
*/

#if defined (__WIN32__) || (__linux__) || (i386) || (i486) || (__ADSPBLACKFIN__) ||  (_MSC_VER)
#define __LITTLE_END__
#undef __BIG_END__
#elif defined (__sun__) || (sun) || (SUN)
#define __BIG_END__
#undef __LITTLE_END__
#else
#pragma "ERROR: cannot determine platform"
#endif

/* When switch FLTPNT is on, all types are mapped to
* floating point. This is for floating point simulations.
*/

#ifdef FLTPNT
typedef float fractN;
typedef float fractM;
typedef double fract40;
typedef double fractLXM;

typedef struct {
	float hi, lo;
} fract2xN;

typedef struct {
	fractN re, im;
} cmplx_frN;

typedef struct {
	fractM re, im;
} cmplx_frM;

/* When switch FLTPNT is off, the library does
* fixed point simulations
*/
#else
typedef short  fractN;
typedef  fractN * fractN_ptr;

typedef int   fractM;
typedef  fractM * fractM_ptr;

#ifndef WIN32
typedef long long _int64;
typedef long long __int64;
#endif
typedef struct {
	fractN re, im;
} cmplx_frN;

typedef cmplx_frN *	cmplx_frN_ptr;

typedef struct {
	fractM re, im;
} cmplx_frM;

typedef cmplx_frM *	cmplx_frM_ptr;

typedef long   fract2xN;

typedef fract2xN * fract2xN_ptr;

typedef  struct {
	long	 accw32;	
	unsigned char accx8;	
}fract40;

typedef fract40 * fract40_ptr;

typedef  struct {
	fractM	accLo;
	long	accHi;
}fractLXM;

typedef fractLXM * fractLXM_ptr;

#endif

/*
* The following types are ints for both fixed
* and float systems
*/
//modify by dengruinan@2010-10-20
//#ifndef  void_ptr
//typedef void *                void_ptr;
//#define NULL_PTR             (void_ptr)0
//#endif
//
//#ifndef  uint8
//typedef unsigned char         uint8;
//#endif
//
//#ifndef  uint8_ptr
//typedef unsigned char *       uint8_ptr;
//#endif
//
//#ifndef  uint16
//typedef unsigned short int    uint16;
//#endif
//
//#ifndef  uint16_ptr
//typedef unsigned short int *  uint16_ptr;
//#endif
//
//#ifndef  uint32
//typedef unsigned long int     uint32;
//#endif
//
//#ifndef  uint32_ptr
//typedef unsigned long int *   uint32_ptr;
//#endif
//
//#ifndef  int8
//typedef char           int8;
//#endif
//
//#ifndef  int8_ptr
//typedef char *         int8_ptr;
//#endif
//
//#ifndef  int16
//typedef signed short int      int16;
//#endif
//
//#ifndef  int16_ptr
//typedef signed short int *    int16_ptr;
//#endif
//
//#ifndef  int32
//typedef signed long int       int32;
//#endif
//
//#ifndef  int32_ptr
//typedef signed long int *     int32_ptr;
//#endif
//
//#ifndef  boolean
//typedef uint8                 boolean;
//#endif
//
//#ifndef  AL_TRUE
//#define AL_TRUE             ((boolean)1)
//#endif
//
//#ifndef  AL_FALSE
//#define AL_FALSE            ((boolean)0)
//#endif

#ifndef  LIB_TYPEDEF_F_H
typedef void *                void_ptr;
#define NULL_PTR              (void_ptr)0

typedef unsigned char         uint8;

typedef unsigned char *       uint8_ptr;

typedef unsigned short int    uint16;

typedef unsigned short int *  uint16_ptr;

typedef unsigned long int     uint32;

typedef unsigned long int *   uint32_ptr;

typedef char                  int8;

typedef char *                int8_ptr;

typedef signed short int      int16;

typedef signed short int *    int16_ptr;

typedef signed long int       int32;

typedef signed long int *     int32_ptr;

typedef uint8                 boolean;

// PRINT TV 
typedef struct {
	fractN im, re ;
} cmplx_frN_HimLre ;


#define AL_TRUE              ((boolean)1)

#define AL_FALSE             ((boolean)0)

#endif//end of #ifndef  LIB_TYPEDEF_F_H
//modify by dengruinan@2010-10-20

#endif /* _LANGUAGE_ASM */

#endif /* __LIB_TYPEDEF__ */

#ifdef __cplusplus
}
#endif

/*$Log: lib_typedef_N.h,v $
/*Revision 1.4.2.1  2012/10/31 02:54:25  kangguoqing
/* (bug Inc00000314)
/*Committed on the Free edition of March Hare Software CVSNT Server.
/*Upgrade to CVS Suite for more features and support:
/*http://march-hare.com/cvsnt/
/**/
