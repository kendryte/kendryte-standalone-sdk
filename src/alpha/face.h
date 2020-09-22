// #include <cmath>
#ifndef _FACE_H
#define _FACE_H
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>


#define MAX_POINT_CNT 10
#define IM_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define IM_DIV(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
#define IM_MOD(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })


static inline int fast_roundf(float x);
double cosine_similarity(double *A, double *B, unsigned int Vector_Length);
void crop_image(uint8_t *src, uint8_t* dst, uint32_t* xy, int width, int height);
void image_resize(uint8_t* in, int w0, int h0, uint8_t* out, int w, int h);
void rgb_to_grayscale(uint8_t r, uint8_t g, uint8_t b, uint8_t* gray, int idx);
static void svd22(const float a[4], float u[4], float s[2], float v[4]);
void affine_getTansform(uint16_t *src, uint16_t *dst, uint16_t cnt, float *TT);
int affine_ai(uint8_t *src_buf, uint8_t *dst_buf, unsigned int hw, float *TT);

#endif /* _IMAGE_PROCESS_H */
