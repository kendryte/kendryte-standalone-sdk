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
#ifndef _FACE_H
#define _FACE_H
#include <stdint.h>


#define MAX_POINT_CNT 10
#define IM_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define IM_DIV(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
#define IM_MOD(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })


static inline int fast_roundf(float x);
double cosine_similarity(double *A, double *B, unsigned int Vector_Length);
void crop_image(uint8_t *src, uint8_t* dst, int x1, int y1, int x2, int y2, int width, int height);
void image_resize(uint8_t* in, int w0, int h0, uint8_t* out, int w, int h);
uint8_t rgb_to_grayscale(uint8_t r, uint8_t g, uint8_t b);
static void svd22(const float a[4], float u[4], float s[2], float v[4]);
void affine_getTansform(uint16_t *src, uint16_t *dst, uint16_t cnt, float *TT);
int affine_ai(uint8_t *src_buf, uint8_t *dst_buf, unsigned int hw, float *TT);

#endif /* _IMAGE_PROCESS_H */
