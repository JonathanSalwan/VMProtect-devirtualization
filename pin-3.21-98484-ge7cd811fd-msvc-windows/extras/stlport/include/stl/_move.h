/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_INTERNAL_MOVE_H
#define _STLP_INTERNAL_MOVE_H

#include <type_traits>

_STLP_BEGIN_NAMESPACE

template <class T>
constexpr typename tr1::remove_reference<T>::type&& move(T&& t) noexcept {
  return static_cast<typename tr1::remove_reference<T>::type&&>(t);
}

_STLP_END_NAMESPACE

#endif /* _STLP_INTERNAL_MOVE_H */

// Local Variables:
// mode:C++
// End:
