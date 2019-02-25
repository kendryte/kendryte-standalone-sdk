/**
* @file         fxp_define.c
* @brief        fixed point function definition
* @author       xiexin
* @date         10/16/2018
* @copyright    canaan
*/

#include "../inc/fxp_define.h"

/** 
* @brief:            Calculate the maximum MSB in a vector
* @param[in/out]     INT32 *in      
* @param[in/out]     INT32 dataLen  
* @return            maximum MSB
*/
INT32 calc_max_msb(INT32 *in, INT32 dataLen)
{               
    INT32 maxMsb=0, msb;
    INT32 i;
    INT32 temp;
    
    for(i=0;i<dataLen;i++)
    {
        msb=0;
        /* Convert in to abs(in). bit inversion for minus number */
        temp=in[i]<0 ? ~in[i] : in[i];	  	
       
        /* Compute MSB */
        while(temp>0)
        {
            temp>>=1;
            msb++;
        }
        
        /* updat the maximum MSB */
        maxMsb=MAX(maxMsb,msb);
    }
        
    return(maxMsb);
}

/** 
* @brief:            vector add for INT16
* @param[in/out]     INT16 *src1  
* @param[in/out]     INT16 *src2  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 len   
*/
void add_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 len)
{
    UINT32 addr;

    for ( addr = 0 ; addr < len ; addr++ )
    {
        //dst[addr] = SAT((INT32)src1[addr] + (INT32)src2[addr], MAX_S16, MIN_S16);
        dst[addr] = add_q15_inline(src1[addr],src2[addr]);
    }
}

/** 
* @brief:            vector add for INT32
* @param[in/out]     INT32 *src1  
* @param[in/out]     INT32 *src2  
* @param[in/out]     INT32 *dst   
* @param[in/out]     UINT32 len   
*/
void add_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 len)
{
    UINT32 addr;

    for ( addr = 0 ; addr < len ; addr++ )
    {
        dst[addr] = add_q31_inline(src1[addr] , src2[addr]);
    }
}

/** 
* @brief:            vector sub for INT16
* @param[in/out]     INT16 *src1  
* @param[in/out]     INT16 *src2  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 len   
*/
void sub_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 len)
{
    UINT32 addr;

    for ( addr = 0 ; addr < len ; addr++ )
    {
        dst[addr] = sub_q15_inline(src1[addr], src2[addr]);
    }
}

void sub_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 len)
{
    UINT32 addr;

    for ( addr = 0 ; addr < len ; addr++ )
    {
        dst[addr] = sub_q31_inline(src1[addr], src2[addr]);
    }
}

/** 
* @brief:            vector scale for INT16
* @param[in/out]     INT16 *src   
* @param[in/out]     INT16 scale  
* @param[in/out]     INT32 shift  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 len   
*/
void scale_q15(INT16 *src, INT16 scale, INT32 shift, INT16 *dst, UINT32 len)
{
    UINT32 addr;
    INT32 Tmp32;

    shift = shift + 15;
    if(shift>0)
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = mul_r_shift_q15_inline(src[addr], scale, shift);
        }
    }
    else
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = mul_l_shift_q15_inline(src[addr], scale, shift);
        }
    }
}


/** 
* @brief:            vector scale for INT32
* @param[in/out]     INT32 *src   
* @param[in/out]     INT32 scale  
* @param[in/out]     INT32 shift  
* @param[in/out]     INT32 *dst   
* @param[in/out]     UINT32 len   
*/
void scale_q31(INT32 *src, INT32 scale, INT32 shift, INT32 *dst, UINT32 len)
{
    UINT32 addr;
    INT64 Tmp32;

    shift = shift + 31;
    if(shift>0)
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = mul_r_shift_q31_inline(src[addr], scale, shift);
        }
    }
    else
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = mul_l_shift_q31_inline(src[addr], scale, shift);
        }
    }
}

/** 
* @brief:            matrix add for INT16
* @param[in/out]     INT16 *src1  
* @param[in/out]     INT16 *src2  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
*/
void mat_add_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 row, UINT32 col)
{
    UINT32 RowXCol,addr;

    RowXCol = row * col;
    for ( addr = 0 ; addr < RowXCol ; addr++ )
    {
        //dst[addr] = SAT((INT32)src1[addr] + (INT32)src2[addr], MAX_S16, MIN_S16);
        dst[addr] = add_q15_inline(src1[addr],src2[addr]);
    }
}

/** 
* @brief:            matrix add for INT32
* @param[in/out]     INT32 *src1  
* @param[in/out]     INT32 *src2  
* @param[in/out]     INT32 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
*/
void mat_add_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 row, UINT32 col)
{
    UINT32 RowXCol,addr;

    RowXCol = row * col;
    for ( addr = 0 ; addr < RowXCol ; addr++ )
    {
        dst[addr] = add_q31_inline(src1[addr] , src2[addr]);
    }
}

/** 
* @brief:            matrix sub for INT16
* @param[in/out]     INT16 *src1  
* @param[in/out]     INT16 *src2  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
*/
void mat_sub_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 row, UINT32 col)
{
    UINT32 RowXCol,addr;

    RowXCol = row * col;
    for ( addr = 0 ; addr < RowXCol ; addr++ )
    {
        dst[addr] = sub_q15_inline(src1[addr], src2[addr]);
    }
}

/** 
* @brief:            matrix sub for INT32
* @param[in/out]     INT32 *src1  
* @param[in/out]     INT32 *src2  
* @param[in/out]     INT32 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
*/
void mat_sub_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 row, UINT32 col)
{
    UINT32 RowXCol,addr;

    RowXCol = row * col;
    for ( addr = 0 ; addr < RowXCol ; addr++ )
    {
        dst[addr] = sub_q31_inline(src1[addr], src2[addr]);
    }
}

/** 
* @brief:            matrix scale for INT16
* @param[in/out]     INT16 *src   
* @param[in/out]     INT16 scale  
* @param[in/out]     INT32 shift  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
*/
void mat_scale_q15(INT16 *src, INT16 scale, INT32 shift, INT16 *dst, UINT32 row, UINT32 col)
{
    UINT32 RowXCol,addr;
    INT32 Tmp32;
    
    RowXCol = row * col;
    shift = shift + 15;
    if(shift>0)
    {
        for ( addr = 0 ; addr < RowXCol ; addr++ )
        {
            dst[addr] = mul_r_shift_q15_inline(src[addr], scale, shift);
        }
    }
    else
    {
        for ( addr = 0 ; addr < RowXCol ; addr++ )
        {
            dst[addr] = mul_l_shift_q15_inline(src[addr], scale, shift);
        }
    }
}

/** 
* @brief:            matrix scale for INT32
* @param[in/out]     INT32 *src   
* @param[in/out]     INT32 scale  
* @param[in/out]     INT32 shift  
* @param[in/out]     INT32 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
*/
void mat_scale_q31(INT32 *src, INT32 scale, INT32 shift, INT32 *dst, UINT32 row, UINT32 col)
{
    UINT32 RowXCol,addr;
    INT64 Tmp64;

    RowXCol = row * col;
    shift = shift + 31;
    if(shift>0)
    {
        for ( addr = 0 ; addr < RowXCol ; addr++ )
        {
            dst[addr] = mul_r_shift_q31_inline(src[addr], scale, shift);
        }
    }
    else
    {
        for ( addr = 0 ; addr < RowXCol ; addr++ )
        {
            dst[addr] = mul_l_shift_q31_inline(src[addr], scale, shift);
        }
    }
}


/** 
* @brief:            matrix multiplication for INT16
* @param[in/out]     INT16 *src1  
* @param[in/out]     INT16 *src2  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
* @param[in/out]     UINT32 col2  
*/
void mat_mul_q15(INT16 *src1, INT16 *src2, INT16 *dst, UINT32 row, UINT32 col, UINT32 col2)
{
    INT32 Tmp32;
    UINT32 addr1,addr2,AddrDst;
    UINT32 irow,icol,icol2;

    for ( irow = 0 ; irow < row ; irow++ )
    {
        for ( icol2 = 0 ; icol2 < col2 ; icol2++ )
        {
            Tmp32 = 0;
            for ( icol = 0 ; icol < col ; icol++ )
            {
                addr1 = irow*col + icol;
                addr2 = icol*col2 + icol2;
                Tmp32 = Tmp32 + mul_q15_inline(src1[addr1], src2[addr2]);
                //Tmp32 = add_q31_inline(Tmp32, (INT32)src1[addr1] * (INT32)src2[addr2]);
            }
            AddrDst = irow*col2 + icol2;
            dst[AddrDst] = SAT(Tmp32, MAX_S16, MIN_S16);
            //dst[AddrDst] = SAT(Tmp32>>15, MAX_S16, MIN_S16);
        }
    }
}

/** 
* @brief:            matrix multiplication for INT32
* @param[in/out]     INT32 *src1  
* @param[in/out]     INT32 *src2  
* @param[in/out]     INT32 *dst   
* @param[in/out]     UINT32 row   
* @param[in/out]     UINT32 col   
* @param[in/out]     UINT32 col2  
*/
void mat_mul_q31(INT32 *src1, INT32 *src2, INT32 *dst, UINT32 row, UINT32 col, UINT32 col2)
{
    INT64 Tmp64;
    UINT32 addr1,addr2,AddrDst;
    UINT32 irow,icol,icol2;

    for ( irow = 0 ; irow < row ; irow++ )
    {
        for ( icol2 = 0 ; icol2 < col2 ; icol2++ )
        {
            Tmp64 = 0;
            for ( icol = 0 ; icol < col ; icol++ )
            {
                addr1 = irow*col + icol;
                addr2 = icol*col2 + icol2;
                Tmp64 = Tmp64 + mul_q31_inline(src1[addr1], src2[addr2]);
            }
            AddrDst = irow*col2 + icol2;
            dst[AddrDst] = SAT(Tmp64, MAX_S32, MIN_S32);
        }
    }
}

/** 
* @brief:            signed left shift for INT16
* @param[in/out]     INT16 *src   
* @param[in/out]     INT32 shift  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 len   
*/
void sl_shift_q15(INT16 *src, INT32 shift, INT16 *dst, UINT32 len)
{
    UINT32 addr;

    if(shift>=0)
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = SAT( (INT32)src[addr] << shift, MAX_S16, MIN_S16);
        }
    }
    else
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = SAT( (INT32)src[addr] >> (-shift), MAX_S16, MIN_S16);
        }
    }
}

/** 
* @brief:            signed left shift for INT32
* @param[in/out]     INT16 *src   
* @param[in/out]     INT32 shift  
* @param[in/out]     INT16 *dst   
* @param[in/out]     UINT32 len   
*/
void sl_shift_q31(INT32 *src, INT32 shift, INT32 *dst, UINT32 len)
{
    UINT32 addr;

    if(shift>=0)
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = SAT( (INT64)src[addr] << shift, MAX_S32, MIN_S32);
        }
    }
    else
    {
        for ( addr = 0 ; addr < len ; addr++ )
        {
            dst[addr] = SAT( (INT64)src[addr] >> (-shift), MAX_S32, MIN_S32);
        }
    }
}

/** 
* @brief:            dot product of the two input vectors for INT16
* @param[in/out]     INT16 *src1  
* @param[in/out]     INT16 *src2  
* @param[in/out]     UINT32 len   
* @return            dot result Q(64,48)
*/
INT64 dprod_q15(INT16 *src1, INT16 *src2, UINT32 len)
{
    INT64 Tmp64=0;
    UINT32 addr;

    for ( addr = 0 ; addr < len ; addr++ )
    {
        Tmp64 = SAT(Tmp64 + (INT64)src1[addr] * (INT64)src2[addr], MAX_S64, MIN_S64);//Q(64,30)
    }
    return SAT(Tmp64<<18, MAX_S64, MIN_S64);
}

/** 
* @brief:            dot product of the two input vectors for INT32
* @param[in/out]     INT16 *src1  
* @param[in/out]     INT16 *src2  
* @param[in/out]     UINT32 len   
* @return            dot result Q(64,48)
*/
INT64 dprod_q31(INT32 *src1, INT32 *src2, UINT32 len)
{
    INT64 Tmp64=0;
    UINT32 addr;

    for ( addr = 0 ; addr < len ; addr++ )
    {
        Tmp64 = SAT(Tmp64 + (((INT64)src1[addr] * (INT64)src2[addr])>>14), MAX_S64, MIN_S64);//Q(64,62)->Q(64,48)
    }
    return Tmp64;
}



inline INT16 add_q15_inline(INT16 a, INT16 b)
{
    INT32 Tmp32;
    Tmp32 = (INT32)a + (INT32)b;
    return SAT(Tmp32, MAX_S16, MIN_S16);
}
inline INT32 add_q31_inline(INT32 a, INT32 b)
{
    INT64 Tmp64;
    Tmp64 = (INT64)a + (INT64)b;
    return SAT(Tmp64, MAX_S32, MIN_S32);
}
inline INT16 sub_q15_inline(INT16 a, INT16 b)
{
    INT32 Tmp32;
    Tmp32 = (INT32)a - (INT32)b;
    return SAT(Tmp32, MAX_S16, MIN_S16);
}
inline INT32 sub_q31_inline(INT32 a, INT32 b)
{
    INT64 Tmp64;
    Tmp64 = (INT64)a - (INT64)b;
    return SAT(Tmp64, MAX_S32, MIN_S32);
}
inline INT16 mul_r_shift_q15_inline(INT16 a, INT16 b, INT32 shift)
{
    INT32 Tmp32;
    Tmp32 = (INT32)a * (INT32)b;
    return SAT(Tmp32>>shift, MAX_S16, MIN_S16);
}
inline INT32 mul_r_shift_q31_inline(INT32 a, INT32 b, INT32 shift)
{
    INT64 Tmp64;
    Tmp64 = (INT64)a * (INT64)b;
    return SAT(Tmp64>>shift, MAX_S32, MIN_S32);
}
inline INT16 mul_l_shift_q15_inline(INT16 a, INT16 b, INT32 shift)
{
    INT32 Tmp32;
    Tmp32 = (INT32)a * (INT32)b;
    return SAT(Tmp32<<shift, MAX_S16, MIN_S16);
}
inline INT32 mul_l_shift_q31_inline(INT32 a, INT32 b, INT32 shift)
{
    INT64 Tmp64;
    Tmp64 = (INT64)a * (INT64)b;
    return SAT(Tmp64<<shift, MAX_S32, MIN_S32);
}
inline INT16 mul_q15_inline(INT16 a, INT16 b)
{
    INT32 Tmp32;
    Tmp32 = (INT32)a * (INT32)b;
    return SAT(Tmp32>>15, MAX_S16, MIN_S16);
}
inline INT32 mul_q31_inline(INT32 a, INT32 b)
{
    INT64 Tmp64;
    Tmp64 = (INT64)a * (INT64)b;
    return SAT(Tmp64>>31, MAX_S32, MIN_S32);
}
