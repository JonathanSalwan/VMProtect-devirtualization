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
/// @file xed-convert-table-init.h

// This file was automatically generated.
// Do not edit this file.

#if !defined(XED_CONVERT_TABLE_INIT_H)
# define XED_CONVERT_TABLE_INIT_H
#include "xed-internal-header.h"
typedef struct {

   const char** table_name;

   xed_operand_enum_t opnd;

   unsigned int limit;

} xed_convert_table_t;
extern xed_convert_table_t xed_convert_table[XED_OPERAND_CONVERT_LAST];
#endif
