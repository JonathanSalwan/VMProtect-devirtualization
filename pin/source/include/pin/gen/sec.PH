/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_SEC_PH
#error duplicate inclusion of sec
#else
#define PIN_G_SEC_PH
/*! @file
  This file contains @ref SEC and Mapping primitives
 */
/*! @ingroup SEC
  SEC types
*/
typedef enum
{
    SEC_TYPE_INVALID,
    SEC_TYPE_UNUSED,
    SEC_TYPE_REGREL,    ///< relocations
    SEC_TYPE_DYNREL,    ///< dynamic relocations
    SEC_TYPE_EXEC,      ///< contains code
    SEC_TYPE_DATA,      ///< contains initialized data
    SEC_TYPE_DYNAMIC,   ///< Dynamic linking information (.dynamic)
    SEC_TYPE_OPD,       ///< contains the official procedure descriptors.
    SEC_TYPE_GOT,       ///< Global offset table. 
    SEC_TYPE_STACK,     ///< Stack segment.
    SEC_TYPE_PLTOFF,    ///< The procedure linking table.
    SEC_TYPE_HASH,      ///< A symbol hash table. 
    SEC_TYPE_LSDA,      ///< old exception_info (obsolete)
    SEC_TYPE_UNWIND,    ///< Stack unwind information. (exception handling)
    SEC_TYPE_UNWINDINFO,///< Stack unwind information. (exception handling)
    SEC_TYPE_REGSYM,    ///< Static symbol table (.symtab)
    SEC_TYPE_DYNSYM,    ///< Dynamic linking symbol table. 
    SEC_TYPE_DEBUG,     ///< Contains debugging information.  
    SEC_TYPE_BSS,       ///< contains uninitialized data
    SEC_TYPE_SYMSTR,    ///< String table section (.strtab)
    SEC_TYPE_DYNSTR,    ///< Dynamic linking string table (.dynstr)
    SEC_TYPE_SECSTR,    ///< Section header string table. (.shstrtab)
    SEC_TYPE_COMMENT,   ///< contains version control information. 
    SEC_TYPE_LOOS,      ///< OS specific
    SEC_TYPE_USER,      ///< Application specific
    SEC_TYPE_PROC,      ///< Processor specific
    SEC_TYPE_LAST
} SEC_TYPE;

#endif // PIN_G_SEC_PH
