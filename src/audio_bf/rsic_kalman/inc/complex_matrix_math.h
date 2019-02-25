#ifndef __COMPLEX_MATRIX_MATH_H__
#define __COMPLEX_MATRIX_MATH_H__
/***************************************************************************
 * complex_matrix_math.h                                                      *
 ***************************************************************************/
/**
 * @defgroup complexMatrix Complex Matrix Functions
 */
#ifdef  __cplusplus
extern "C"
{
#endif
#include "fxp_define.h"
#define MAX_CMAT_ROW 20
#define MAX_CMAT_COL 20
// Matrix Addition
void cmat_add_q15(INT16* src1_real, INT16* src1_imag,
	    				INT16* src2_real, INT16* src2_imag,
						INT16* dst_real, INT16* dst_imag,
						UINT32 row, UINT32 col);
void cmat_add_q31(INT32* src1_real, INT32* src1_imag,
	    				INT32* src2_real, INT32* src2_imag,
						INT32* dst_real, INT32* dst_imag,
						UINT32 row, UINT32 col);
// Matrix Multiplication
void cmat_mul_q31(INT32* src1_real, INT32* src1_imag,
					    INT32* src2_real, INT32* src2_imag,
						INT32* dst_real, INT32* dst_imag,
					    UINT32 row, UINT32 col, UINT32 col2);
void cmat_mul_q15(INT16* src1_real, INT16* src1_imag,
					    INT16* src2_real, INT16* src2_imag,
						INT16* dst_real, INT16* dst_imag,
					    UINT32 row, UINT32 col, UINT32 col2);
// Matrix Scale
void cmat_scale_q31(INT32* src_real, INT32* src_imag,
						  INT32 scale_fract, INT32 shift, INT32* dst_real, INT32* dst_imag,
						  UINT32 row, UINT32 col);
void cmat_scale_q15(INT16* src_real, INT16* src_imag,
						  INT16 scale_fract, INT32 shift, INT16* dst_real, INT16* dst_imag,
						  UINT32 row, UINT32 col);
// Matrix complex Scale
void cmat_cscale_q31(INT32* src_real, INT32* src_imag,
						  INT32 scale_real, INT32 scale_imag, INT32 shift, INT32* dst_real, INT32* dst_imag,
						  UINT32 row, UINT32 col);
void cmat_cscale_q15(INT16* src_real, INT16* src_imag,
						  INT16 scale_real, INT16 scale_imag, INT32 shift, INT16* dst_real, INT16* dst_imag,
						  UINT32 row, UINT32 col);
// Matrix Subtraction
void cmat_sub_q15(INT16* src1_real, INT16* src1_imag,
	    				INT16* src2_real, INT16* src2_imag,
						INT16* dst_real, INT16* dst_imag,
						UINT32 row, UINT32 col);
void cmat_sub_q31(INT32* src1_real, INT32* src1_imag,
	    				INT32* src2_real, INT32* src2_imag,
						INT32* dst_real, INT32* dst_imag,
						UINT32 row, UINT32 col);
#ifdef  __cplusplus
}
#endif
#endif
