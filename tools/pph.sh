#!/bin/bash
#
# Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
pph() {
  version=$1 revision=$2 patch=$3
  echo "//"
  echo "// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)"
  echo "//"
  echo "// Distributed under the Boost Software License, Version 1.0."
  echo "// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)"
  echo "//"
  echo "#pragma once"
  echo "#if (__cplusplus < 201305L && _MSC_VER < 1900)"
  echo "#error \"Boost.DI requires C++14 support (Clang-3.4+, GCC-5.1+, MSVC-2015+)\""
  echo "#else"
  echo "#define BOOST_DI_VERSION ${version}'${revision}'${patch}"
  echo "#define BOOST_DI_NAMESPACE_BEGIN namespace boost { namespace di { inline namespace v${version}_${revision}_${patch} {"
  echo "#define BOOST_DI_NAMESPACE_END }}}"
  echo "#if !defined(BOOST_DI_CFG_DIAGNOSTICS_LEVEL)"
  echo "#define BOOST_DI_CFG_DIAGNOSTICS_LEVEL 1"
  echo "#endif"
  echo "#if defined(BOOST_DI_CFG_FWD)"
  echo "BOOST_DI_CFG_FWD"
  echo "#endif"
  rm -rf tmp && mkdir tmp && cp -r boost tmp && cd tmp
  find . -iname "*.hpp" | xargs sed -i "s/BOOST_DI_NAMESPACE/::boost::di::v${version}_${revision}_${patch}/g"
  find . -iname "*.hpp" | xargs sed -i "s/\(.*\)__pph__/\/\/\/\/\1/g"
  find . -iname "*.hpp" | xargs sed -i "s/.*\(clang-format.*\)/\/\/\/\/\1/g"
  tail -n +10 "boost/di/aux_/compiler.hpp" | head -n -2 | sed '/^$/d' | sed "s/ \/\/\\(.*\)//g" | sed "s/\/\/\/\///"
  echo "#if defined(__CLANG__)"
  echo "#if (!BOOST_DI_CFG_DIAGNOSTICS_LEVEL)"
  echo "#pragma clang diagnostic error \"-Wdeprecated-declarations\""
  echo "#else"
  echo "#pragma clang diagnostic warning \"-Wdeprecated-declarations\""
  echo "#endif"
  echo "#pragma clang diagnostic push"
  echo "#pragma clang diagnostic error \"-Wundefined-inline\""
  echo "#pragma clang diagnostic error \"-Wundefined-internal\""
  echo "#pragma clang diagnostic ignored \"-Wmissing-field-initializers\""
  echo "#elif defined(__GCC__)"
  echo "#pragma GCC diagnostic error \"-Wdeprecated-declarations\""
  echo "#pragma GCC diagnostic push"
  echo "#if (__GNUC__ < 6)"
  echo "#pragma GCC diagnostic error \"-Werror\""
  echo "#endif"
  echo "#elif defined(__MSVC__)"
  echo "#pragma warning(disable : 4503)"
  echo "#pragma warning(disable : 4822)"
  echo "#pragma warning(disable : 4505)"
  echo "#endif"
  echo '
    #include "boost/di/fwd_ext.hpp"
    BOOST_DI_NAMESPACE_BEGIN
    #include "boost/di/config.hpp"
    #include "boost/di/bindings.hpp"
    #include "boost/di/inject.hpp"
    #include "boost/di/injector.hpp"
    #include "boost/di/make_injector.hpp"
    #include "boost/di/scopes/deduce.hpp"
    #include "boost/di/scopes/external.hpp"
    #include "boost/di/scopes/singleton.hpp"
    #include "boost/di/scopes/unique.hpp"
    #include "boost/di/policies/constructible.hpp"
    #include "boost/di/providers/heap.hpp"
    #include "boost/di/providers/stack_over_heap.hpp"' > tmp.hpp
  cpp -C -P -nostdinc -I. \
    -DBOOST_DI_AUX_COMPILER_HPP \
    -DBOOST_DI_AUX_PREPROCESSOR_HPP \
    -DBOOST_DI_INJECT_HPP tmp.hpp | \
    sed "s/\/\/\/\///" | \
    sed "s/[ $]*#define/##define/g" | \
    cpp -P -I. -fpreprocessed - 2>/dev/null | \
    sed "s/clang-format\(.*\)/\/\/ clang-format\1/g" | \
    sed "s/^##define/#define/g"
  tail -n +11 "boost/di/aux_/preprocessor.hpp" | head -n -2 | sed '/^$/d' | sed "s/ \/\/\\(.*\)//g"
  tail -n +15 "boost/di/inject.hpp" | head -n -2 | sed '/^$/d' | sed "s/ \/\/\\(.*\)//g"
  cd .. && rm -rf tmp
  echo "BOOST_DI_NAMESPACE_END"
  echo "#endif"
  echo "#if defined(__CLANG__)"
  echo "#pragma clang diagnostic pop"
  echo "#elif defined(__GCC__)"
  echo "#pragma GCC diagnostic pop"
  echo "#endif"
  echo
}

set -e
cd ${0%/*}/../include && pph `head -1 ../doc/CHANGELOG.md  | sed "s/.*\[\(.*\)\].*/\1/" | tr '.' ' '` > "boost/di.hpp"
${CLANG_FORMAT:=clang-format} -i "boost/di.hpp"
