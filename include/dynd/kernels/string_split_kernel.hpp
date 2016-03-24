//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

// String split kernel

#pragma once

#include <dynd/string.hpp>
#include <dynd/string_search.hpp>
#include <dynd/types/var_dim_type.hpp>

namespace dynd {
namespace nd {

  struct string_split_kernel : base_strided_kernel<string_split_kernel, 2> {

    intrusive_ptr<memory_block_data> m_dst_memblock;

    string_split_kernel(intrusive_ptr<memory_block_data> dst_memblock) : m_dst_memblock(dst_memblock) {}

    void single(char *dst, char *const *src)
    {
      ndt::var_dim_type::data_type *dst_v = reinterpret_cast<ndt::var_dim_type::data_type *>(dst);

      const string *const *s = reinterpret_cast<const string *const *>(src);
      const string &haystack = *(s[0]);
      const string &needle = *(s[1]);

      intptr_t count = dynd::string_count(haystack, needle);

      if (count == 0) {
        dst_v->begin = m_dst_memblock->alloc(1);
        dst_v->size = 1;
        string *dst_str = reinterpret_cast<string *>(dst_v->begin);
        dst_str[0] = haystack;
        return;
      }

      dst_v->begin = m_dst_memblock->alloc(count + 1);
      dst_v->size = count + 1;
      string *dst_str = reinterpret_cast<string *>(dst_v->begin);

      dynd::detail::string_splitter<string> f(dst_str, haystack, needle);
      dynd::detail::string_search(haystack, needle, f);
      f.finish();
    }
  };

} // namespace nd
} // namespace dynd
