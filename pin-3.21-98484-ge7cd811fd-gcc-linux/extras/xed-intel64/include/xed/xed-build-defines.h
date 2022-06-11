/* BEGIN_LEGAL 

Copyright (c) 2021 Intel Corporation

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  
END_LEGAL */
#if !defined(XED_BUILD_DEFINES_H)
#  define XED_BUILD_DEFINES_H

#  if !defined(XED_AMD_ENABLED)
#    define XED_AMD_ENABLED
#  endif
#  if !defined(XED_AVX)
#    define XED_AVX
#  endif
#  if !defined(XED_CET)
#    define XED_CET
#  endif
#  if !defined(XED_DECODER)
#    define XED_DECODER
#  endif
#  if !defined(XED_DLL)
#    define XED_DLL
#  endif
#  if !defined(XED_ENCODER)
#    define XED_ENCODER
#  endif
#  if !defined(XED_GIT_VERSION)
#    define XED_GIT_VERSION "13.0.0"
#  endif
#  if !defined(XED_MPX)
#    define XED_MPX
#  endif
#  if !defined(XED_SUPPORTS_AVX512)
#    define XED_SUPPORTS_AVX512
#  endif
#  if !defined(XED_SUPPORTS_LZCNT_TZCNT)
#    define XED_SUPPORTS_LZCNT_TZCNT
#  endif
#  if !defined(XED_SUPPORTS_SHA)
#    define XED_SUPPORTS_SHA
#  endif
#  if !defined(XED_SUPPORTS_WBNOINVD)
#    define XED_SUPPORTS_WBNOINVD
#  endif
#  if !defined(XED_VIA_ENABLED)
#    define XED_VIA_ENABLED
#  endif
#endif
