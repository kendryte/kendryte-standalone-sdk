#include "../inc/kal_wpe_float.h"
#include <stdio.h>

typedef struct {
    z_t y_hist[WPE_L];
    z_t K[WPE_L];
    z_t R1[WPE_L * WPE_L];
    z_t W[WPE_L];
} state_freq_t;

state_freq_t states[NRE];
void kalman_wpe_float_init(){
    for(int f=0; f<NRE; f++){
        state_freq_t* cs = &states[f];
        for(int i=0; i<WPE_L; i++){
            cs->y_hist[i] = (z_t){.re=0, .im=0};
            cs->K[i] = (z_t){.re=0, .im=0};
            cs->W[i] = (z_t){.re=0, .im=0};
        }
        for(int i=0; i<WPE_L*WPE_L; i++){
            cs->R1[i] = (z_t){.re=0, .im=0};
        }
    }
}
void kalman_wpe_float(z_t data[NRE]){

    // for each freq
    for(int f=0; f<NRE; f++){
        state_freq_t* cs = &states[f];
        z_t cur = data[f];
        // float eng = |data[f]|
        float eng = 0;
        for(int i=0; i<WPE_L; i++){
            eng += z_eng(cs->y_hist[i]);
            printf("eng[%d] %f %f\n", i, eng, z_eng(cs->y_hist[i]));
        }
        eng = ((eng<(1e-4f)) ? (1e-4f) : eng);
        printf("eng %f \n", eng);
        printf("eng %f \n", eng);
        printf("eng %f \n", eng);
        // z_t y_est = data[f] - H(W) * y_hist // todo: W or H(W)
        data[f] = z_sub(data[f], z_vec_dot_conj(cs->y_hist, cs->W, WPE_L));
        printf("data[%d] %f %f\n", f, data[f].re, data[f].im);

        // y_hist_H = H(y_hist)
        z_t y_hist_H[WPE_L];
        for(int i=0; i<WPE_L; i++){
            y_hist_H[i] = z_conj(cs->y_hist[i]);
        }

        printf("eng %f \n", eng);
        // z_t[] K = (R1 * y_hist)/(a*eng + H(y_hist)*R1*y_hist)
        // nmtr = R1 * y_hist
        z_t nmtr[WPE_L];
        z_mat_mul(cs->R1, cs->y_hist, nmtr, WPE_L, WPE_L, 1);
        // dnmt2 = H(y_hist)*R1*y_hist = H(y_hist)*nmtr
        z_t dnmt2 = z_vec_dot(nmtr, y_hist_H, WPE_L);
        z_t dnmt = z_add((z_t){.re=KAL_ALPHA*eng, .im=0}, dnmt2);
        z_t inv_dnmt = (z_t){.re=1/dnmt.re, .im=1/dnmt.im};
        // K = nmtr*inv_dnmt
        for(int i=0; i<WPE_L; i++){
            cs->K[i] = z_mul(nmtr[i], inv_dnmt);
        }
        printf("nmtr %f %f\n", nmtr[4].re, nmtr[4].im);
        printf("dnmt %f %f  eng %f\n", dnmt.re, dnmt.im, eng);
        printf("K %f %f\n", cs->K[4].re, cs->K[4].im);

        // z_t[][] R1 = 1/a*(R1-K*H(y_hist)*R1)
        // z_t[] mat_mul_tmp = H(y_hist)*R1
        z_t mat_mul_tmp[WPE_L], mat_mul_tmp2[WPE_L * WPE_L];
        z_mat_mul(y_hist_H, cs->R1, mat_mul_tmp, 1, WPE_L, WPE_L);
        // z_t[][] mat_mul_tmp2 = K*H(y_hist)*R1 = K*mat_mul_tmp
        z_mat_mul(cs->K, mat_mul_tmp, mat_mul_tmp2, WPE_L, 1, WPE_L);
        // R1 = (R1 - mat_mul_tmp2)/a
        for(int i=0; i<WPE_L*WPE_L; i++){
            cs->R1[i] = z_sub(cs->R1[i], z_scale(mat_mul_tmp2[i], KAL_ALPHA));
        }
        printf("R1 %f %f\n", cs->R1[4].re, cs->R1[4].im);

        // z_t[] W = W + K*H(y_est)
        for(int i=0; i<WPE_L; i++){
            cs->W[i] = z_add(cs->W[i], z_mul(cs->K[i], data[f]));
        }
        printf("w %f %f\n", cs->W[4].re, cs->W[4].im);

        // y_hist.push(y)
        for(int i=1; i<WPE_L; i++){
            cs->y_hist[i] = cs->y_hist[i-1];
        }
        cs->y_hist[0] = cur;
    }
}           

