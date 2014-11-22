//
// Copyright (C) 2011-14 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/kernels/ckernel_builder.hpp>
#include <dynd/types/arrfunc_type.hpp>
#include <dynd/kernels/expr_kernels.hpp>

#ifdef __CUDACC__

namespace dynd { namespace kernels { namespace detail {

__global__ void cuda_parallel_single(char *DYND_UNUSED(dst), char *DYND_UNUSED(src0), char *DYND_UNUSED(src1),
                                     ckernel_prefix *DYND_UNUSED(self))
{
//  char *src[2] = {src0, src1};

  //expr_single_t func = self->get_function<expr_single_t>();
//  func(dst, src, self);
}

}

class cuda_parallel_ck : public expr_ck<cuda_parallel_ck, 1> {
  typedef cuda_parallel_ck self_type;

  cuda_device_ckernel_builder m_ckb;
  dim3 m_blocks;
  dim3 m_threads;

public:
  cuda_parallel_ck(dim3 blocks, dim3 threads)
      : m_blocks(blocks), m_threads(threads)
  {
  }

  cuda_device_ckernel_builder get_ckb() const {
    return m_ckb;
  }

  void single(char *dst, char **src)
  {
    detail::cuda_parallel_single<<<m_blocks, m_threads>>>(dst, src[0], src[1], m_ckb.get());
    throw_if_not_cuda_success(cudaDeviceSynchronize());
  }

  static intptr_t instantiate(const arrfunc_type_data *DYND_UNUSED(af_self),
                              const arrfunc_type *DYND_UNUSED(af_tp),
                              ckernel_builder *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp),
                              const char *DYND_UNUSED(dst_arrmeta),
                              const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta),
                              kernel_request_t kernreq,
                              const eval::eval_context *DYND_UNUSED(ectx),
                              const nd::array &DYND_UNUSED(args),
                              const nd::array &DYND_UNUSED(kwds))
  {
    self_type::create(ckb, kernreq, ckb_offset, 1, 1);
    return ckb_offset;
  }
};
}
} // namespace kernels

#endif // __CUDACC__