/***************************************************************************
 * complex_matrix_math.c                                                  *
 ***************************************************************************/
#include "../inc/complex_matrix_math.h"
#include <stdlib.h>

#define cmat_add(name, type) \
void cmat_add_##name(type* src1_real, type* src1_imag,	 	 \
	type* src2_real, type* src2_imag,						 		 \
	type* dst_real, type* dst_imag,									 			 \
	UINT32 row, UINT32 col)													 \
{																				 \
	mat_add_##name(src1_real, src2_real, dst_real, row, col);				 \
	mat_add_##name(src1_imag, src2_imag, dst_imag, row, col);				 \
}
cmat_add(q15, INT16);
cmat_add(q31, INT32);
#define cmat_mul(name, type) \
static type cmat_mul_tmp_##name[MAX_CMAT_ROW * MAX_CMAT_COL];								   \
void cmat_mul_##name(type* src1_real, type* src1_imag,					   \
	type* src2_real, type* src2_imag,											   \
	type* dst_real, type* dst_imag,															   \
	UINT32 row, UINT32 col, UINT32 col2)												   \
{																							   \
	mat_mul_##name(src1_real, src2_real, dst_real, row, col, col2);					   \
	mat_mul_##name(src1_imag, src2_imag, cmat_mul_tmp_##name, row, col, col2);		   \
	sub_##name(dst_real, cmat_mul_tmp_##name, dst_real, row * col2);					   \
																							   \
	mat_mul_##name(src1_imag, src2_real, dst_imag, row, col, col2);					   \
	mat_mul_##name(src1_real, src2_imag, cmat_mul_tmp_##name, row, col, col2);		   \
	add_##name(dst_imag, cmat_mul_tmp_##name, dst_imag, row * col2);					   \
}
cmat_mul(q15, INT16);
cmat_mul(q31, INT32);

#define cmat_qscale(name, type) \
void cmat_scale_##name(type* src_real, type* src_imag,	   \
	type scale_fract, INT32 shift, type* dst_real, type* dst_imag,\
	UINT32 row, UINT32 col)												   \
{																			   \
	mat_scale_##name(src_real, scale_fract, shift, dst_real, row, col);  \
	mat_scale_##name(src_imag, scale_fract, shift, dst_imag, row, col);  \
}
cmat_qscale(q15, INT16);
cmat_qscale(q31, INT32);

#define cmat_qcscale(name, type)                                                    \
void cmat_cscale_##name(type* src_real, type* src_imag,	        \
    type scale_real, type scale_imag, INT32 shift, type* dst_real, type* dst_imag,\
    UINT32 row, UINT32 col)                                                     \
{                                                                                   \
    mat_scale_##name(src_real, scale_real, shift, dst_real, row, col);        \
    mat_scale_##name(src_imag, scale_imag, shift, cmat_mul_tmp_##name, row, col); \
    mat_sub_##name(dst_real, cmat_mul_tmp_##name, dst_real, row, col);            \
    mat_scale_##name(src_real, scale_imag, shift, dst_imag, row, col);            \
    mat_scale_##name(src_imag, scale_real, shift, cmat_mul_tmp_##name, row, col); \
    mat_add_##name(dst_imag, cmat_mul_tmp_##name, dst_imag, row, col);            \
}
cmat_qcscale(q15, INT16);
cmat_qcscale(q31, INT32);


#define cmat_sub(name, type) \
void cmat_sub_##name(type* src1_real, type* src1_imag,	 	 \
	type* src2_real, type* src2_imag,						 		 \
	type* dst_real, type* dst_imag,									 			 \
	UINT32 row, UINT32 col)													 \
{																				 \
	mat_sub_##name(src1_real, src2_real, dst_real, row, col);				 \
	mat_sub_##name(src1_imag, src2_imag, dst_imag, row, col);				 \
}
cmat_sub(q15, INT16);
cmat_sub(q31, INT32);
