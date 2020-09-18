/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "face.h"
// #include "iomem.h"
#include <stdlib.h>
// #include <cmath>
#include <math>

static inline int fast_roundf(float x) { return (int)(x); }

double cosine_similarity(double *A, double *B, unsigned int Vector_Length) {
    double dot = 0.0, denom_a = 0.0, denom_b = 0.0;
    for (unsigned int i = 0u; i < Vector_Length; ++i) {
        dot += A[i] * B[i];
        denom_a += A[i] * A[i];
        denom_b += B[i] * B[i];
    }
    return dot / (sqrt(denom_a) * sqrt(denom_b));
}

void crop_image(uint8_t *src, uint8_t* dst, int x1, int y1, int x2, int y2, int width, int height) {
    // uint8_t dst[3][(x2-x1) * (y2-y1)];
    int limit = (x2-x1) * (y2-y1);
    int count = 0;
    int fixed = 0;

    for (int i=0; i<3; i++) {
        count = 0;
        fixed = width*height*i;
        for (int j=y1; j<=y2; j++) {
            for (int k=x1; k<=x2; k++) {
                dst[i][count] = src[(j-1)*width + k + fixed];
                count++;
            }
        }
    }
}

void rgb_to_grayscale(uint8_t *rgb, uint8_t *grayscale) {
    float r_lin = xyz_table[(uint32_t)(rgb.addr)];
    float g_lin = xyz_table[(uint32_t)(rgb.addr + 96 * 96)];
    float b_lin = xyz_table[(uint32_t)(rgb.addr + 96 * 96 * 2)];
    float y =
        ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) / 100.0f;
    y = (y > 0.0031308f) ? ((1.055f * powf(y, 0.416666f)) - 0.055f)
                         : (y * 12.92f);
    grayscale = IM_MAX(IM_MIN(fast_roundf(y * 255), 255), 0);
}

static void svd22(const float a[4], float u[4], float s[2], float v[4]) {
    s[0] = (sqrt(pow(a[0] - a[3], 2) + pow(a[1] + a[2], 2)) +
            sqrt(pow(a[0] + a[3], 2) + pow(a[1] - a[2], 2))) /
           2;
    s[1] = fabs(s[0] - sqrt(pow(a[0] - a[3], 2) + pow(a[1] + a[2], 2)));
    v[2] = (s[0] > s[1]) ? sin((atan2(2 * (a[0] * a[1] + a[2] * a[3]),
                                      a[0] * a[0] - a[1] * a[1] + a[2] * a[2] -
                                          a[3] * a[3])) /
                               2)
                         : 0;
    v[0] = sqrt(1 - v[2] * v[2]);
    v[1] = -v[2];
    v[3] = v[0];
    u[0] = (s[0] != 0) ? -(a[0] * v[0] + a[1] * v[2]) / s[0] : 1;
    u[2] = (s[0] != 0) ? -(a[2] * v[0] + a[3] * v[2]) / s[0] : 0;
    u[1] = (s[1] != 0) ? (a[0] * v[1] + a[1] * v[3]) / s[1] : -u[2];
    u[3] = (s[1] != 0) ? (a[2] * v[1] + a[3] * v[3]) / s[1] : u[0];
    v[0] = -v[0];
    v[2] = -v[2];
}

// 2D affine
// #define MAX_POINT_CNT 10
void affine_getTansform(uint16_t *src, uint16_t *dst, uint16_t cnt, float *TT) {
    int i, j, k;
    float src_mean[2] = {0.0f};
    float dst_mean[2] = {0.0f};
    for (i = 0; i < cnt * 2; i += 2) {
        src_mean[0] += dst[i];
        src_mean[1] += dst[i + 1];
        dst_mean[0] += src[i];
        dst_mean[1] += src[i + 1];
    }
    src_mean[0] /= cnt;
    src_mean[1] /= cnt;
    dst_mean[0] /= cnt;
    dst_mean[1] /= cnt;

    float src_demean[MAX_POINT_CNT][2] = {0.0f};
    float dst_demean[MAX_POINT_CNT][2] = {0.0f};

    for (i = 0; i < cnt; i++) {
        src_demean[i][0] = dst[2 * i] - src_mean[0];
        src_demean[i][1] = dst[2 * i + 1] - src_mean[1];
        dst_demean[i][0] = src[2 * i] - dst_mean[0];
        dst_demean[i][1] = src[2 * i + 1] - dst_mean[1];
    }

    float A[2][2] = {0.0f};
    for (i = 0; i < 2; i++) {
        for (k = 0; k < 2; k++) {
            for (j = 0; j < cnt; j++) {
                A[i][k] += dst_demean[j][i] * src_demean[j][k];
            }
            A[i][k] /= cnt;
        }
    }

    float(*T)[3] = (float(*)[3])TT;
    T[0][0] = 1;
    T[0][1] = 0;
    T[0][2] = 0;
    T[1][0] = 0;
    T[1][1] = 1;
    T[1][2] = 0;
    T[2][0] = 0;
    T[2][1] = 0;
    T[2][2] = 1;

    float U[2][2] = {0};
    float S[2] = {0};
    float V[2][2] = {0};
    svd22(&A[0][0], &U[0][0], S, &V[0][0]);

    T[0][0] = U[0][0] * V[0][0] + U[0][1] * V[1][0];
    T[0][1] = U[0][0] * V[0][1] + U[0][1] * V[1][1];
    T[1][0] = U[1][0] * V[0][0] + U[1][1] * V[1][0];
    T[1][1] = U[1][0] * V[0][1] + U[1][1] * V[1][1];

    float scale = 1.0f;
    float src_demean_mean[2] = {0.0f};
    float src_demean_var[2] = {0.0f};
    for (i = 0; i < cnt; i++) {
        src_demean_mean[0] += src_demean[i][0];
        src_demean_mean[1] += src_demean[i][1];
    }
    src_demean_mean[0] /= cnt;
    src_demean_mean[1] /= cnt;

    for (i = 0; i < cnt; i++) {
        src_demean_var[0] += (src_demean_mean[0] - src_demean[i][0]) *
                             (src_demean_mean[0] - src_demean[i][0]);
        src_demean_var[1] += (src_demean_mean[1] - src_demean[i][1]) *
                             (src_demean_mean[1] - src_demean[i][1]);
    }
    src_demean_var[0] /= (cnt);
    src_demean_var[1] /= (cnt);
    scale = 1.0f / (src_demean_var[0] + src_demean_var[1]) * (S[0] + S[1]);
    T[0][2] =
        dst_mean[0] - scale * (T[0][0] * src_mean[0] + T[0][1] * src_mean[1]);
    T[1][2] =
        dst_mean[1] - scale * (T[1][0] * src_mean[0] + T[1][1] * src_mean[1]);
    T[0][0] *= scale;
    T[0][1] *= scale;
    T[1][0] *= scale;
    T[1][1] *= scale;
}

int affine_ai(uint8_t *src_buf, uint8_t *dst_buf, unsigned int hw, float *TT) {
    int step = hw;
    int color_step = hw * hw;
    int i, j, k;
    // uint8_t *src_buf = src_img->pix_ai;
    // uint8_t *dst_buf = dst_img->pix_ai;

    int dst_color_step = hw * hw;
    int dst_step = hw;

    memset(dst_buf, 0, hw * hw * 1);

    int pre_x, pre_y; //缩放前对应的像素点坐标
    int x, y;
    unsigned short color[2][2];
    float(*T)[3] = (float(*)[3])TT;
    /*printf("==========\r\n");
    printf("%.3f,%.3f,%.3f\r\n",T[0][0],T[0][1],T[0][2]);
    printf("%.3f,%.3f,%.3f\r\n",T[1][0],T[1][1],T[1][2]);
    printf("%.3f,%.3f,%.3f\r\n",T[2][0],T[2][1],T[2][2]);
    printf("##############\r\n");*/
    for (i = 0; i < hw; i++) {
        for (j = 0; j < hw; j++) {
            pre_x = (int)(T[0][0] * (j << 8) + T[0][1] * (i << 8) +
                          T[0][2] * (1 << 8));
            pre_y = (int)(T[1][0] * (j << 8) + T[1][1] * (i << 8) +
                          T[1][2] * (1 << 8));

            y = pre_y & 0xFF;
            x = pre_x & 0xFF;
            pre_x >>= 8;
            pre_y >>= 8;
            if (pre_x < 0 || pre_x > (hw - 1) || pre_y < 0 ||
                pre_y > (hw - 1))
                continue;
            for (k = 0; k < 1; k++) {
                color[0][0] = src_buf[pre_y * step + pre_x + k * color_step];
                color[1][0] =
                    src_buf[pre_y * step + (pre_x + 1) + k * color_step];
                color[0][1] =
                    src_buf[(pre_y + 1) * step + pre_x + k * color_step];
                color[1][1] =
                    src_buf[(pre_y + 1) * step + (pre_x + 1) + k * color_step];
                int final = (256 - x) * (256 - y) * color[0][0] +
                            x * (256 - y) * color[1][0] +
                            (256 - x) * y * color[0][1] + x * y * color[1][1];
                final = final >> 16;
                dst_buf[i * dst_step + j + k * dst_color_step] = final;
            }
        }
    }
    return 0;
}
