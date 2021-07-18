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
#include <cstring>
#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase_v1.h>
#include <stdio.h>
#include <sysctl.h>
#include <utils.h>

using namespace nncase;
using namespace nncase::runtime;

#define NNCASE_DEBUG 0

namespace
{
class nncase_context
{
public:
    result<void> load_kmodel(const uint8_t *buffer)
    {
        return interp_.load_model({ reinterpret_cast<const gsl::byte *>(buffer), SIZE_MAX });
    }

    result<void> get_output(uint32_t index, uint8_t **data, size_t *size)
    {
        try_var(tensor, interp_.output_tensor(index));
        try_var(host_tensor, tensor.as_host());
        try_var(map, hrt::map(host_tensor, hrt::map_read));
        auto buffer = map.buffer();

        *data = reinterpret_cast<uint8_t *>(buffer.data());
        *size = buffer.size_bytes();
        return ok();
    }

    result<void> run_kmodel(const uint8_t *src, dmac_channel_number_t dma_ch)
    {
#if NNCASE_DEBUG
        auto micro = sysctl_get_time_us();
#endif
        try_(interp_.options().set("dma_ch", (uint32_t)dma_ch));
        auto &shape = interp_.input_shape(0);
        auto type = interp_.input_desc(0).datatype;
        try_var(input_tensor, hrt::create(type, shape, { (gsl::byte *)src, get_bytes(type, shape) }, false, hrt::pool_shared));
        try_(hrt::sync(input_tensor, hrt::sync_write_back));
        try_(interp_.input_tensor(0, input_tensor));
        try_(interp_.run());
#if NNCASE_DEBUG
        auto duration = sysctl_get_time_us() - micro;
        printf("run kmodel takes %f ms.\n", duration / 1e3f);
#endif
        return ok();
    }

private:
    interpreter interp_;
};
}

int nncase_v1_load_kmodel(kpu_model_context_t *ctx, const uint8_t *buffer)
{
    auto nnctx = new (std::nothrow) nncase_context();
    if (ctx)
    {
        ctx->is_nncase = 1;
        ctx->nncase_ctx = nnctx;
        ctx->nncase_version = 1;
        auto ret = nnctx->load_kmodel(buffer);
        if (ret.is_ok())
            return 0;
        return -1;
    }
    else
    {
        return -1;
    }
}

int nncase_v1_get_output(kpu_model_context_t *ctx, uint32_t index, uint8_t **data, size_t *size)
{
    auto nnctx = reinterpret_cast<nncase_context *>(ctx->nncase_ctx);
    auto ret = nnctx->get_output(index, data, size);
    if (ret.is_ok())
        return 0;
    return -1;
}

void nncase_v1_model_free(kpu_model_context_t *ctx)
{
    auto nnctx = reinterpret_cast<nncase_context *>(ctx->nncase_ctx);
    delete nnctx;
    ctx->nncase_ctx = nullptr;
}

int nncase_v1_run_kmodel(kpu_model_context_t *ctx, const uint8_t *src, dmac_channel_number_t dma_ch, kpu_done_callback_t done_callback, void *userdata)
{
    auto nnctx = reinterpret_cast<nncase_context *>(ctx->nncase_ctx);
    auto ret = nnctx->run_kmodel(src, dma_ch);
    if (ret.is_ok())
    {
        done_callback(userdata);
        return 0;
    }
    else
    {
        printf("error: %s\n", ret.unwrap_err().message().c_str());
    }
    return -1;
}
