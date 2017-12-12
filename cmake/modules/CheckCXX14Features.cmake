 ################################################################################
 #    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    #
 #                                                                              #
 #              This software is distributed under the terms of the             # 
 #              GNU Lesser General Public Licence (LGPL) version 3,             #  
 #                  copied verbatim in the file "LICENSE"                       #
 ################################################################################
# - Check which parts of the C++11/14 standard the compiler supports
#
# When found it will set the following variables
#
#  CXX14_COMPILER_FLAGS         - the compiler flags needed to get C++11/14 features
#
#  HAS_CXX14_AUTO               - auto keyword
#  HAS_CXX14_AUTO_RET_TYPE      - function declaration with deduced return types
#  HAS_CXX14_CLASS_OVERRIDE     - override and final keywords for classes and methods
#  HAS_CXX14_CONSTEXPR          - constexpr keyword
#  HAS_CXX14_CSTDINT_H          - cstdint header
#  HAS_CXX14_DECLTYPE           - decltype keyword
#  HAS_CXX14_FUNC               - __func__ preprocessor constant
#  HAS_CXX14_INITIALIZER_LIST   - initializer list
#  HAS_CXX14_LAMBDA             - lambdas
#  HAS_CXX14_LIB_REGEX          - regex library
#  HAS_CXX14_LONG_LONG          - long long signed & unsigned types
#  HAS_CXX14_NULLPTR            - nullptr
#  HAS_CXX14_RVALUE_REFERENCES  - rvalue references
#  HAS_CXX14_SIZEOF_MEMBER      - sizeof() non-static members
#  HAS_CXX14_STATIC_ASSERT      - static_assert()
#  HAS_CXX14_VARIADIC_TEMPLATES - variadic templates
#  HAS_CXX14_SHAREDPOINTER      - shared pointer 

#=============================================================================
# Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
# Copyright 2012 Andreas Weis
# Copyleft  2017 Lorenzo Zanetti <lzanetti@ikp.tu-darmstad.de>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

#
# Each feature may have up to 3 checks, every one of them in it's own file
# FEATURE.cpp              - example that must build and return 0 when run
# FEATURE_fail.cpp         - example that must build, but may not return 0 when run
# FEATURE_fail_compile.cpp - example that must fail compilation
#
# The first one is mandatory, the latter 2 are optional and do not depend on
# each other (i.e. only one may be present).
#

if (NOT CMAKE_CXX_COMPILER_LOADED)
    message(FATAL_ERROR "CheckCXX14Features modules only works if language CXX is enabled")
endif ()

cmake_minimum_required(VERSION 2.8.2)

#
### Check for needed compiler flags
#
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++14" _HAS_CXX14_FLAG)
if (NOT _HAS_CXX14_FLAG)
    check_cxx_compiler_flag("-std=c++0x" _HAS_CXX0X_FLAG)
endif ()

if (_HAS_CXX14_FLAG)
    set(CXX14_COMPILER_FLAGS "-std=c++14")
elseif (_HAS_CXX0X_FLAG)
    set(CXX14_COMPILER_FLAGS "-std=c++0x")
endif ()

function(cxx14_check_feature FEATURE_NAME RESULT_VAR)
    if (NOT DEFINED ${RESULT_VAR})
        set(_bindir "${CMAKE_CURRENT_BINARY_DIR}/cxx11/cxx11_${FEATURE_NAME}")

        set(_SRCFILE_BASE ${CheckSrcDir}/cxx11-test-${FEATURE_NAME})
        set(_LOG_NAME "\"${FEATURE_NAME}\"")
        message(STATUS "Checking C++11/14 support for ${_LOG_NAME}")

        set(_SRCFILE "${_SRCFILE_BASE}.cpp")
        set(_SRCFILE_FAIL "${_SRCFILE_BASE}_fail.cpp")
        set(_SRCFILE_FAIL_COMPILE "${_SRCFILE_BASE}_fail_compile.cpp")

        if (CROSS_COMPILING)
            try_compile(${RESULT_VAR} "${_bindir}" "${_SRCFILE}"
                        COMPILE_DEFINITIONS "${CXX14_COMPILER_FLAGS}")
            if (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
                try_compile(${RESULT_VAR} "${_bindir}_fail" "${_SRCFILE_FAIL}"
                            COMPILE_DEFINITIONS "${CXX14_COMPILER_FLAGS}")
            endif (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
        else (CROSS_COMPILING)
            try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR
                    "${_bindir}" "${_SRCFILE}"
                    COMPILE_DEFINITIONS "${CXX14_COMPILER_FLAGS}")
            if (_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
                set(${RESULT_VAR} TRUE)
            else (_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
                set(${RESULT_VAR} FALSE)
            endif (_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
            if (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
                try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR
                        "${_bindir}_fail" "${_SRCFILE_FAIL}"
                         COMPILE_DEFINITIONS "${CXX14_COMPILER_FLAGS}")
                if (_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
                    set(${RESULT_VAR} TRUE)
                else (_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
                    set(${RESULT_VAR} FALSE)
                endif (_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
            endif (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
        endif (CROSS_COMPILING)
        if (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL_COMPILE})
            try_compile(_TMP_RESULT "${_bindir}_fail_compile" "${_SRCFILE_FAIL_COMPILE}"
                        COMPILE_DEFINITIONS "${CXX14_COMPILER_FLAGS}")
            if (_TMP_RESULT)
                set(${RESULT_VAR} FALSE)
            else (_TMP_RESULT)
                set(${RESULT_VAR} TRUE)
            endif (_TMP_RESULT)
        endif (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL_COMPILE})

        if (${RESULT_VAR})
            message(STATUS "Checking C++11/14 support for ${_LOG_NAME}: works")
        else (${RESULT_VAR})
            message(STATUS "Checking C++11/14 support for ${_LOG_NAME}: not supported")
        endif (${RESULT_VAR})
        set(${RESULT_VAR} ${${RESULT_VAR}} CACHE INTERNAL "C++11/14 support for ${_LOG_NAME}")
    endif (NOT DEFINED ${RESULT_VAR})
endfunction(cxx14_check_feature)

cxx14_check_feature("__func__" HAS_CXX14_FUNC)
cxx14_check_feature("auto" HAS_CXX14_AUTO)
cxx14_check_feature("auto_ret_type" HAS_CXX14_AUTO_RET_TYPE)
cxx14_check_feature("class_override_final" HAS_CXX14_CLASS_OVERRIDE)
cxx14_check_feature("constexpr" HAS_CXX14_CONSTEXPR)
cxx14_check_feature("constructor-delegation" HAS_CXX14_CONSTRUCTOR_DELEGATION)
cxx14_check_feature("cstdint" HAS_CXX14_CSTDINT_H)
cxx14_check_feature("decltype" HAS_CXX14_DECLTYPE)
cxx14_check_feature("initializer_list" HAS_CXX14_INITIALIZER_LIST)
cxx14_check_feature("lambda" HAS_CXX14_LAMBDA)
cxx14_check_feature("long_long" HAS_CXX14_LONG_LONG)
cxx14_check_feature("nullptr" HAS_CXX14_NULLPTR)
cxx14_check_feature("regex" HAS_CXX14_LIB_REGEX)
cxx14_check_feature("rvalue-references" HAS_CXX14_RVALUE_REFERENCES)
cxx14_check_feature("sizeof_member" HAS_CXX14_SIZEOF_MEMBER)
cxx14_check_feature("static_assert" HAS_CXX14_STATIC_ASSERT)
cxx14_check_feature("variadic_templates" HAS_CXX14_VARIADIC_TEMPLATES)
cxx14_check_feature("sharedpointer" HAS_CXX14_SHAREDPOINTER)
cxx14_check_feature("funcptr-to-lambda-conversion" HAS_CXX14_PATTERN1)

