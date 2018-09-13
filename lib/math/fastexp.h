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

#ifndef KENDRYTE_FAST_EXP
#define KENDRYTE_FAST_EXP

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * calc exp2f(x)
 **/
float fast_exp2f(float x);

/**
 * calc expf(x)
 **/
float fast_expf(float x);

/**
 * calc powf(base,expo)
 **/
float fast_powf(float base, float expo);

/**
 * calc exp2(x)
 **/
static inline double fast_exp2(double x)
{
    return fast_exp2f((float)x);
}

/**
 * calc exp(x)
 **/
static inline double fast_exp(double x)
{
    return fast_expf((float)x);
}

/**
 * calc pow(base,expo)
 **/
static inline double fast_pow(double base, double expo)
{
    return fast_powf((float)base, (float)expo);
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <forward_list>
namespace std{
    template < typename... Args >
    auto fast_pow(Args && ... args)->decltype(::fast_pow(std::forward < Args > (args)...)) {
        return ::fast_pow(std::forward < Args > (args)...);
    }

    template < typename... Args >
    auto fast_powf(Args && ... args)->decltype(::fast_powf(std::forward < Args > (args)...)) {
        return ::fast_powf(std::forward < Args > (args)...);
    }

    template < typename... Args >
    auto fast_exp(Args && ... args)->decltype(::fast_exp(std::forward < Args > (args)...)) {
        return ::fast_exp(std::forward < Args > (args)...);
    }

    template < typename... Args >
    auto fast_expf(Args && ... args)->decltype(::fast_expf(std::forward < Args > (args)...)) {
        return ::fast_expf(std::forward < Args > (args)...);
    }

    template < typename... Args >
    auto fast_exp2(Args && ... args)->decltype(fast_exp2(std::forward < Args > (args)...)) {
        return ::fast_exp2(std::forward < Args > (args)...);
    }

    template < typename... Args >
    auto fast_exp2f(Args && ... args)->decltype(::fast_exp2f(std::forward < Args > (args)...)) {
        return ::fast_exp2f(std::forward < Args > (args)...);
    }
}
#endif



#ifdef CONFIG_FAST_EXP_OVERRIDE
#define pow fast_pow
#define powf fast_powf
#define exp fast_exp
#define expf fast_expf
#define exp2 fast_exp2
#define exp2f fast_exp2f


#endif

#endif
