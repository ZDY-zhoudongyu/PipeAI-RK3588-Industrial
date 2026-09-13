#include "rknn_engine_runtime.hpp"
#ifdef PIPEAI_WITH_RKNN
#include <rknn_api.h>
#endif

namespace pipeai {

bool RknnRuntimeInfo::query(void* context)
{
#ifdef PIPEAI_WITH_RKNN
    rknn_context ctx = reinterpret_cast<rknn_context>(context);
    rknn_input_output_num io{};
    if(rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io, sizeof(io)) != RKNN_SUCC)
        return false;

    inputs_.resize(io.n_input);
    outputs_.resize(io.n_output);

    for(uint32_t i=0;i<io.n_input;i++)
    {
        rknn_tensor_attr attr{};
        attr.index=i;
        if(rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &attr, sizeof(attr))==RKNN_SUCC)
        {
            inputs_[i].index=i;
            inputs_[i].type=attr.type;
            inputs_[i].scale=attr.scale;
            inputs_[i].zero_point=attr.zp;
            for(uint32_t j=0;j<attr.n_dims;j++)
                inputs_[i].dims.push_back(attr.dims[j]);
        }
    }
    return true;
#else
    (void)context;
    return false;
#endif
}

}
