/* Copyright 2019-2020 Canaan Inc.
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
#pragma once
#include "datatypes.h"
#include <cassert>
#include <cmath>
#include <limits>

namespace nncase
{
namespace quant
{
    template <class TIt>
    value_range<float> get_range(TIt begin, TIt end)
    {
        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::min();
        while (begin != end)
        {
            auto value = *begin++;
            auto fc = std::fpclassify(value);
            if (fc == FP_NORMAL || fc == FP_SUBNORMAL || fc == FP_ZERO)
            {
                min = std::min(min, value);
                max = std::max(max, value);
            }
        }

        return { min, max };
    }

    inline value_range<float> fixup_range(value_range<float> range)
    {
        if (range.min < -1e3)
            range.min = -1e3;
        if (range.max > 1e3)
            range.max = 1e3;
        auto r = range.max - range.min;
        if (r == 0)
            r = 0.1f;
        else if (r < 0.01f)
            r = 0.01f;
        range.max = range.min + r;

        if (range.max < 0)
            range.max = 0;
        if (range.min > 0)
            range.min = 0;
        return range;
    }

    inline quant_param_t get_quant_param(value_range<float> range, int32_t bits)
    {
        range = fixup_range(range);
        auto r = range.max - range.min;
        auto scale = ((1LL << bits) - 1) / r;
        auto bias = std::round(-range.min * scale);
        assert(bias >= 0);
        return { static_cast<int32_t>(bias), scale };
    }

    inline fixed_mul get_fixed_mul(float value, int32_t max_bits, uint8_t max_shift, bool is_signed)
    {
        assert(!is_signed || value >= 0);

        auto bits = is_signed ? max_bits - 1 : max_bits;
        int32_t shift = 0;
        float mul = 0;

        if (std::abs(value) > 1)
        {
            int mul_shift;
            mul = std::frexp(value, &mul_shift);
            shift = std::min((int32_t)max_shift, bits - mul_shift);
            mul = mul * std::pow(2.f, shift + mul_shift);
        }
        else if (value == 0)
        {
            mul = 0;
            shift = 0;
        }
        else
        {
            int mul_shift;
            mul = std::frexp(value, &mul_shift);
            shift = std::min(max_shift + mul_shift, bits);
            mul = mul * std::pow(2.f, shift);
            shift -= mul_shift;
        }

        assert(std::abs(mul) < std::pow(2, bits));
        assert(shift >= 0 && shift <= max_shift);
        assert(std::abs(value - mul * std::pow(2, -shift)) <= std::numeric_limits<float>::epsilon());
        return { mul, static_cast<int8_t>(shift) };
    }
}
}
