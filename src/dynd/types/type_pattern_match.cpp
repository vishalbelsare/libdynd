//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/types/type_pattern_match.hpp>
#include <dynd/func/arrfunc.hpp>
#include <dynd/types/pointer_type.hpp>
#include <dynd/types/base_struct_type.hpp>
#include <dynd/types/option_type.hpp>
#include <dynd/types/base_memory_type.hpp>
#include <dynd/types/fixed_dim_type.hpp>
#include <dynd/types/cfixed_dim_type.hpp>
#include <dynd/types/typevar_type.hpp>
#include <dynd/types/typevar_dim_type.hpp>
#include <dynd/types/pow_dimsym_type.hpp>
#include <dynd/types/ellipsis_dim_type.hpp>
#include <dynd/types/any_sym_type.hpp>
#include <dynd/types/dim_fragment_type.hpp>
#include <dynd/types/typevar_constructed_type.hpp>

using namespace std;
using namespace dynd;

// TODO: We need to properly add offsets to the concrete_arrmeta in every
// function below.

bool ndt::pattern_match(const ndt::type &concrete, const char *concrete_arrmeta,
                        const ndt::type &pattern,
                        std::map<nd::string, ndt::type> &typevars)
{
  return concrete.matches(concrete_arrmeta, pattern, NULL, typevars);
}