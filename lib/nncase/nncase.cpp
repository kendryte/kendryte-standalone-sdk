/* Copyright 2019 Canaan Inc.
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
#include "v0/nncase_v0.h"
#include "v1/nncase_v1.h"
#include <cstring>
#include <nncase.h>
#include <stdio.h>
#include <utils.h>

extern "C"
{
    struct model_header
    {
        uint32_t identifier;
        uint32_t version;
    };

    int nncase_load_kmodel(kpu_model_context_t *ctx, const uint8_t *buffer)
    {
        auto header = reinterpret_cast<const model_header *>(buffer);
        if (header->version == 4)
            return nncase_v0_load_kmodel(ctx, buffer);
        else
            return nncase_v1_load_kmodel(ctx, buffer);
    }

    int nncase_get_output(kpu_model_context_t *ctx, uint32_t index, uint8_t **data, size_t *size)
    {
        if (ctx->nncase_version == 0)
            return nncase_v0_get_output(ctx, index, data, size);
        else
            return nncase_v1_get_output(ctx, index, data, size);
    }

    void nncase_model_free(kpu_model_context_t *ctx)
    {
        if (ctx->nncase_version == 0)
            return nncase_v0_model_free(ctx);
        else
            return nncase_v1_model_free(ctx);
    }

    int nncase_run_kmodel(kpu_model_context_t *ctx, const uint8_t *src, dmac_channel_number_t dma_ch, kpu_done_callback_t done_callback, void *userdata)
    {
        if (ctx->nncase_version == 0)
            return nncase_v0_run_kmodel(ctx, src, dma_ch, done_callback, userdata);
        else
            return nncase_v1_run_kmodel(ctx, src, dma_ch, done_callback, userdata);
    }
}
