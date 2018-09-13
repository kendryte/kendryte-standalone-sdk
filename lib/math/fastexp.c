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

#include "fastexp.h"
#include <stdint.h>

static inline float _fast_exp2f(float in)
{
    static const float p1 = 0.0f;
    static const float p2 = 0.0f;
    static const float p3 = 0.0f;
    static const float p4 = 0.0f;
    static const float p5 = 0.0f;

    float x1 = in;
    float x2 = x1 * in;
    float x3 = x2 * in;
    float x4 = x3 * in;

    return x4 * p1 + x3 * p2 + x2 * p3 + x1 * p4 + p5;
}


float fast_exp2f(float x)
{
    union _float {
        float f;
        struct {
            uint32_t frac : 23;
            uint32_t expo : 8;
            uint32_t sign : 1;
        } data;
    };

    if (x < 0)
        return 1.0f / fast_exp2f(-x);

    union _float f = {.f = 1.0f};

    f.data.expo = (int)x + 127;

    return _fast_exp2f(x - (int)x) * f.f;
}


float fast_expf(float x)
{
    extern float __ieee754_expf(float x);

    return __ieee754_expf(x);
}


float fast_powf(float base, float expo)
{
    extern float __ieee754_powf(float base, float expo);

    return __ieee754_powf(base, expo);
}
