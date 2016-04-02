//
// Copyright (C) 2011-16 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/callables/base_callable.hpp>
#include <dynd/kernels/forward_na_kernel.hpp>

namespace dynd {
namespace nd {

  template <int... I>
  class forward_na_callable;

  template <int I>
  class forward_na_callable<I> : public base_callable {
    callable m_child;

  public:
    forward_na_callable(const ndt::type &tp, const callable &child) : base_callable(tp), m_child(child) {}

    ndt::type resolve(base_callable *DYND_UNUSED(caller), char *DYND_UNUSED(data), call_graph &cg,
                      const ndt::type &dst_tp, size_t DYND_UNUSED(nsrc), const ndt::type *src_tp, size_t nkwd,
                      const array *kwds, const std::map<std::string, ndt::type> &tp_vars) {
      cg.emplace_back(this);

      is_na->resolve(this, nullptr, cg, ndt::make_type<bool>(), 1, src_tp + I, nkwd, kwds, tp_vars);

      ndt::type src_value_tp[2];
      src_value_tp[I] = src_tp[I].extended<ndt::option_type>()->get_value_type();
      src_value_tp[1 - I] = src_tp[1 - I];

      ndt::type res_value_tp =
          m_child->resolve(this, nullptr, cg, dst_tp.is_symbolic() ? m_child.get_ret_type() : dst_tp, 2, src_value_tp,
                           nkwd, kwds, tp_vars);
      return assign_na->resolve(this, nullptr, cg, ndt::make_type<ndt::option_type>(res_value_tp), 0, nullptr, nkwd,
                                kwds, tp_vars);
    }

    void resolve_dst_type(char *data, ndt::type &dst_tp, intptr_t nsrc, const ndt::type *src_tp, intptr_t nkwd,
                          const array *kwds, const std::map<std::string, ndt::type> &tp_vars) {
      ndt::type child_src_tp[2];
      child_src_tp[I] = src_tp[I].extended<ndt::option_type>()->get_value_type();
      child_src_tp[1 - I] = src_tp[1 - I];

      const ndt::type &child_dst_tp = m_child.get_ret_type();
      if (child_dst_tp.is_symbolic()) {
        m_child->resolve_dst_type(data, dst_tp, nsrc, child_src_tp, nkwd, kwds, tp_vars);
      } else {
        dst_tp = child_dst_tp;
      }
      dst_tp = ndt::make_type<ndt::option_type>(dst_tp);
    }

    void instantiate(call_node *node, char *data, kernel_builder *ckb, const ndt::type &dst_tp, const char *dst_arrmeta,
                     intptr_t nsrc, const ndt::type *src_tp, const char *const *src_arrmeta, kernel_request_t kernreq,
                     intptr_t nkwd, const array *kwds, const std::map<std::string, ndt::type> &tp_vars) {
      size_t self_offset = ckb->size();
      size_t child_offsets[2];

      ckb->emplace_back<forward_na_kernel<I>>(kernreq);
      ckb->emplace_back(2 * sizeof(size_t));

      node = next(node);
      node->callee->instantiate(node, data, ckb, dst_tp, dst_arrmeta, nsrc, src_tp + I, src_arrmeta + I,
                                kernel_request_single, nkwd, kwds, tp_vars);

      ndt::type child_src_tp[2];
      child_src_tp[I] = src_tp[I].extended<ndt::option_type>()->get_value_type();
      child_src_tp[1 - I] = src_tp[1 - I];

      child_offsets[0] = ckb->size() - self_offset;
      node = next(node);
      node->callee->instantiate(node, data, ckb, dst_tp.extended<ndt::option_type>()->get_value_type(), dst_arrmeta,
                                nsrc, child_src_tp, src_arrmeta, kernel_request_single, nkwd, kwds, tp_vars);

      child_offsets[1] = ckb->size() - self_offset;
      node = next(node);
      node->callee->instantiate(node, data, ckb, dst_tp, nullptr, 0, nullptr, nullptr, kernel_request_single, nkwd,
                                kwds, tp_vars);

      memcpy(ckb->get_at<forward_na_kernel<I>>(self_offset)->get_offsets(), child_offsets, 2 * sizeof(size_t));
    }
  };

  template <callable &Callable, bool Src0IsOption, bool Src1IsOption>
  class option_comparison_callable;

  template <callable &Callable>
  class option_comparison_callable<Callable, true, true> : public base_callable {
  public:
    option_comparison_callable() : base_callable(ndt::type("(?Scalar, ?Scalar) -> ?bool")) {}

    ndt::type resolve(base_callable *DYND_UNUSED(caller), char *DYND_UNUSED(data), call_graph &cg,
                      const ndt::type &DYND_UNUSED(dst_tp), size_t DYND_UNUSED(nsrc), const ndt::type *src_tp,
                      size_t nkwd, const array *kwds, const std::map<std::string, ndt::type> &tp_vars) {
      cg.emplace_back(this);

      is_na->resolve(this, nullptr, cg, ndt::make_type<bool>(), 1, &src_tp[0], nkwd, kwds, tp_vars);
      is_na->resolve(this, nullptr, cg, ndt::make_type<bool>(), 1, &src_tp[1], nkwd, kwds, tp_vars);

      const ndt::type src_value_tp[2] = {src_tp[0].extended<ndt::option_type>()->get_value_type(),
                                         src_tp[1].extended<ndt::option_type>()->get_value_type()};
      ndt::type res_value_tp =
          Callable->resolve(this, nullptr, cg, ndt::make_type<bool1>(), 2, src_value_tp, nkwd, kwds, tp_vars);
      return assign_na->resolve(this, nullptr, cg, ndt::make_type<ndt::option_type>(ndt::make_type<bool1>()), 0,
                                nullptr, nkwd, kwds, tp_vars);
    }

    void instantiate(call_node *node, char *data, kernel_builder *ckb, const ndt::type &dst_tp, const char *dst_arrmeta,
                     intptr_t nsrc, const ndt::type *src_tp, const char *const *src_arrmeta, kernel_request_t kernreq,
                     intptr_t nkwd, const array *kwds, const std::map<std::string, ndt::type> &tp_vars) {
      intptr_t ckb_offset = ckb->size();
      intptr_t option_comp_offset = ckb_offset;
      ckb->emplace_back<option_comparison_kernel<true, true>>(kernreq);
      ckb_offset = ckb->size();

      node = next(node);
      node->callee->instantiate(node, data, ckb, dst_tp, dst_arrmeta, nsrc, &src_tp[0], &src_arrmeta[0],
                                kernel_request_single, nkwd, kwds, tp_vars);
      ckb_offset = ckb->size();
      option_comparison_kernel<true, true> *self =
          ckb->get_at<option_comparison_kernel<true, true>>(option_comp_offset);
      self->is_na_rhs_offset = ckb_offset - option_comp_offset;

      node = next(node);
      node->callee->instantiate(node, data, ckb, dst_tp, dst_arrmeta, nsrc, &src_tp[1], &src_arrmeta[1],
                                kernel_request_single, nkwd, kwds, tp_vars);
      ckb_offset = ckb->size();
      self = ckb->get_at<option_comparison_kernel<true, true>>(option_comp_offset);
      self->comp_offset = ckb_offset - option_comp_offset;
      auto cmp = Callable;
      const ndt::type child_src_tp[2] = {src_tp[0].extended<ndt::option_type>()->get_value_type(),
                                         src_tp[1].extended<ndt::option_type>()->get_value_type()};
      node = next(node);
      node->callee->instantiate(node, data, ckb, dst_tp.extended<ndt::option_type>()->get_value_type(), dst_arrmeta,
                                nsrc, child_src_tp, src_arrmeta, kernel_request_single, nkwd, kwds, tp_vars);
      ckb_offset = ckb->size();
      self = ckb->get_at<option_comparison_kernel<true, true>>(option_comp_offset);
      self->assign_na_offset = ckb_offset - option_comp_offset;
      node = next(node);
      node->callee->instantiate(node, data, ckb, ndt::make_type<ndt::option_type>(ndt::make_type<bool1>()), nullptr, 0,
                                nullptr, nullptr, kernel_request_single, nkwd, kwds, tp_vars);
      ckb_offset = ckb->size();
    }
  };

} // namespace dynd::nd
} // namespace dynd
