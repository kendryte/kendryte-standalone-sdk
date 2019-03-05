#include "../config.h"
#include <fft.h>
#include <stdio.h>
#include "kal_wpe_float.h"

typedef struct {
    z_t y_hist[KAL_WPE_L];
    z_t K[KAL_WPE_L];
    z_t R1[KAL_WPE_L * KAL_WPE_L];
    z_t W[KAL_WPE_L];
} state_freq_t;

state_freq_t states[KAL_NRE];
void kalman_wpe_float_init(){
    for(int f=0; f<KAL_NRE; f++){
        state_freq_t* cs = &states[f];
        for(int i=0; i<KAL_WPE_L; i++){
            cs->y_hist[i] = (z_t){.re=0, .im=0};
            cs->K[i] = (z_t){.re=0, .im=0};
            cs->W[i] = (z_t){.re=0, .im=0};
        }
        for(int i=0; i<KAL_WPE_L*KAL_WPE_L; i++){
            cs->R1[i] = (z_t){.re=0, .im=0};
        }
    }
}
void kalman_wpe_float(z_t data[KAL_NRE]){

    // for each freq
    for(int f=0; f<KAL_NRE; f++){
        state_freq_t* cs = &states[f];
        z_t cur = data[f];
        // float eng = |data[f]|
        float eng = 0;
        for(int i=0; i<KAL_WPE_L; i++){
            eng += z_eng(cs->y_hist[i]);
        }
        eng = ((eng<(1e-4f)) ? (1e-4f) : eng);
        // z_t y_est = data[f] - H(W) * y_hist // todo: W or H(W)
        data[f] = z_sub(data[f], z_vec_dot_conj(cs->y_hist, cs->W, KAL_WPE_L));
        // printf("data[%d] %f %f\n", f, data[f].re, data[f].im);

        // y_hist_H = H(y_hist)
        z_t y_hist_H[KAL_WPE_L];
        for(int i=0; i<KAL_WPE_L; i++){
            y_hist_H[i] = z_conj(cs->y_hist[i]);
        }

        // z_t[] K = (R1 * y_hist)/(a*eng + H(y_hist)*R1*y_hist)
        // nmtr = R1 * y_hist
        z_t nmtr[KAL_WPE_L];
        z_mat_mul(cs->R1, cs->y_hist, nmtr, KAL_WPE_L, KAL_WPE_L, 1);
        // dnmt2 = H(y_hist)*R1*y_hist = H(y_hist)*nmtr
        z_t dnmt2 = z_vec_dot(nmtr, y_hist_H, KAL_WPE_L);
        z_t inv_dnmt = z_inv(z_add((z_t){.re=KAL_ALPHA*eng, .im=0}, dnmt2));
        // K = nmtr*inv_dnmt
        for(int i=0; i<KAL_WPE_L; i++){
            cs->K[i] = z_mul(nmtr[i], inv_dnmt);
        }
        // printf("nmtr %f %f\n", nmtr[4].re, nmtr[4].im);
        // z_t dnmt = z_add((z_t){.re=KAL_ALPHA*eng, .im=0}, dnmt2);
        // printf("dnmt %f %f  eng %f\n", dnmt.re, dnmt.im, eng);
        // printf("K %f %f\n", cs->K[4].re, cs->K[4].im);

        // z_t[][] R1 = 1/a*(R1-K*H(y_hist)*R1)
        // z_t[] mat_mul_tmp = H(y_hist)*R1
        z_t mat_mul_tmp[KAL_WPE_L], mat_mul_tmp2[KAL_WPE_L * KAL_WPE_L];
        z_mat_mul(y_hist_H, cs->R1, mat_mul_tmp, 1, KAL_WPE_L, KAL_WPE_L);
        // z_t[][] mat_mul_tmp2 = K*H(y_hist)*R1 = K*mat_mul_tmp
        z_mat_mul(cs->K, mat_mul_tmp, mat_mul_tmp2, KAL_WPE_L, 1, KAL_WPE_L);
        // R1 = (R1 - mat_mul_tmp2)/a
        for(int i=0; i<KAL_WPE_L*KAL_WPE_L; i++){
            cs->R1[i] = z_sub(cs->R1[i], z_scale(mat_mul_tmp2[i], KAL_ALPHA));
        }
        // printf("R1 %f %f\n", cs->R1[4].re, cs->R1[4].im);

        // z_t[] W = W + K*H(y_est)
        for(int i=0; i<KAL_WPE_L; i++){
            cs->W[i] = z_add(cs->W[i], z_mul(cs->K[i], data[f]));
        }
        // printf("w %f %f\n", cs->W[4].re, cs->W[4].im);

        // y_hist.push(y)
        for(int i=1; i<KAL_WPE_L; i++){
            cs->y_hist[i] = cs->y_hist[i-1];
        }
        cs->y_hist[0] = cur;
    }
}           

fft_data_t fft_buf_tx[KAL_FRAME_LEN];
fft_data_t fft_buf_rx[KAL_FRAME_LEN];
void process_frame_float(int16_t frame_in[KAL_FRAME_LEN], int16_t frame_acc[KAL_FRAME_LEN]){
    z_t data[KAL_NRE];
    for(int i=0; i<KAL_FRAME_LEN/2; i++){
        uint32_t idx = i*2;
        fft_buf_tx[i].I1 = 0;
        fft_buf_tx[i].R1 = (int16_t)frame_in[idx];
        fft_buf_tx[i].I2 = 0;
        fft_buf_tx[i].R2 = (int16_t)frame_in[idx+1];
    }
    fft_complex_uint16_dma(DMA_CH_FFT_TX, DMA_CH_FFT_RX, 0x4a, FFT_DIR_FORWARD, fft_buf_tx, 512, fft_buf_rx);
    for(int i=0; i<KAL_FRAME_LEN/4; i++){
        data[i*2].im = fft_buf_rx[i].I1;
        data[i*2].re = fft_buf_rx[i].R1;
        data[i*2+1].im = fft_buf_rx[i].I2;
        data[i*2+1].re = fft_buf_rx[i].R2;
    }
    data[KAL_FRAME_LEN/2].im = fft_buf_rx[KAL_FRAME_LEN/4].I1;
    data[KAL_FRAME_LEN/2].re = fft_buf_rx[KAL_FRAME_LEN/4].R1;

    kalman_wpe_float(data);

    for(int i=0; i<KAL_FRAME_LEN/4; i++){
        fft_buf_tx[i].I1 = data[i*2].im;
        fft_buf_tx[i].R1 = data[i*2].re;
        fft_buf_tx[i].I2 = data[i*2+1].im;
        fft_buf_tx[i].R2 = data[i*2+1].re;
        fft_buf_tx[KAL_FRAME_LEN/2-1-i].I1 = -data[i*2+2].im;
        fft_buf_tx[KAL_FRAME_LEN/2-1-i].R1 = data[i*2+2].re;
        fft_buf_tx[KAL_FRAME_LEN/2-1-i].I2 = -data[i*2+1].im;
        fft_buf_tx[KAL_FRAME_LEN/2-1-i].R2 = data[i*2+1].re;
    }

    fft_complex_uint16_dma(DMA_CH_FFT_TX, DMA_CH_FFT_RX, 0x4a, FFT_DIR_BACKWARD, fft_buf_tx, 512, fft_buf_rx);
    for(int i=0; i<KAL_FRAME_LEN/2; i++){
        uint32_t idx = i*2;
        frame_acc[idx] += (int32_t)fft_buf_rx[i].R1;
        frame_acc[idx+1] += (int32_t)fft_buf_rx[i].R2;
    }
}
