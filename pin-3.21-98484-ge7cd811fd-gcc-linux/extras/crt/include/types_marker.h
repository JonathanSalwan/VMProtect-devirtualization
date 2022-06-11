/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component private header

/*! @file
  this headerfile contains defines stuff
 */

/*
** This file is also included by c files, so do not introduce any C++ stuff in here.
** For example, do not use C++ style comments
*/

#ifndef TYPES_MARKER_H
#define TYPES_MARKER_H

#ifndef ASM_ONLY

/*
 PIN_MS_COMPATIBLE, PIN_GNU_COMPATIBLE
 These defines serve as guards for compiler-specific stuff. The intel compiler
 acts like a microsoft compiler on windows and like a gnu compiler on linux.
*/
#if defined(_MSC_VER)
#define PIN_MS_COMPATIBLE
#elif defined(__GNUC__)
#define PIN_GNU_COMPATIBLE
#else
#error "Could not find suitable compiler (MS or GNU)"
#endif

/*
 SECTION
 Definition of the section attribute.
 The macro is deprecated since it can not be appropriately defined for all supported
 compilers and platforms. Use macros  DATA_SECTION, DATA_CONST_SECTION and CODE_SECTION
 instead.
*/
#if defined(PIN_GNU_COMPATIBLE)

#if defined(TARGET_MAC)
#define SECTION(name)
#else
#define SECTION(name) __attribute__((section(name)))
#endif

#elif defined(PIN_MS_COMPATIBLE)

#pragma deprecated("SECTION")
#define SECTION(name) __pragma(message("The SECTION macro is ignored. Use DATA_SECTION and CODE_SECTION instead."))

#endif

/*
 CODE_SECTION(name), DATA_SECTION(name), DATA_CONST_SECTION(name)
 Names a section in which the following function/data/constant data item will be allocated.
 The macro should appear immediately before the item's type qualifier.

 SECTION_END
 Designates the end of the most recent data or code section.
 The macro should appear immediately after the data or function definition.

 Only one data or function definition is allowed inside the section. Nested sections are
 disallowed.

 Usage:
 DATA_SECTION("mydata")
 static int myInt = 0;
 SECTION_END

 CODE_SECTION("mycode")
 extern void MyFunc ()
 {
    ......
 }
 SECTION_END

 NOTE: Currently, the <extern "C"> type qualifiers, like GLOBALCFUN are not supported
 inside sections. One can fix the issue by allowing section macro to appear after
 the type qualifier, like GLOBALCFUN CODE_SECTION("mycode") void MyFunc () {...} SECTION_END.
 This, however, will require a change in the script generating headers, so the section macro
 will be excluded from the item declaration in the generated header.
*/
#if defined(PIN_GNU_COMPATIBLE)

#if defined(TARGET_MAC)
#define CODE_SECTION(name) __attribute__((section("__TEXT, " name)))
#define DATA_SECTION(name) __attribute__((section("__DATA, " name)))
#define DATA_CONST_SECTION(name) __attribute__((section("__TEXT, " name)))
#else
#define CODE_SECTION(name) __attribute__((section(name)))
#define DATA_SECTION(name) __attribute__((section(name)))
#define DATA_CONST_SECTION(name) __attribute__((section(name)))
#endif
#define SECTION_END

#elif defined(PIN_MS_COMPATIBLE)

#define PUSH_SECTIONS__ __pragma(code_seg(push)) __pragma(data_seg(push)) __pragma(const_seg(push)) __pragma(bss_seg(push))
#define POP_SECTIONS__ __pragma(code_seg(pop)) __pragma(data_seg(pop)) __pragma(const_seg(pop)) __pragma(bss_seg(pop))

#define CODE_SECTION(name) PUSH_SECTIONS__ __pragma(code_seg(name))
#define DATA_SECTION(name) PUSH_SECTIONS__ __pragma(data_seg(name))
#define DATA_CONST_SECTION(name) PUSH_SECTIONS__ __pragma(const_seg(name))
#define BSS_SECTION(name) PUSH_SECTIONS__ __pragma(bss_seg(name))
#define SECTION_END POP_SECTIONS__

#endif

/*
 UNUSED         - Designates possibly unused variable
 PRE_ALIGNTO    - Designates type of variable that should be aligned to a specific alignment.
                    This should precede the type declaration.
                    One should use both PRE_ALIGNTO and POST_ALIGNTO (below) to ensure proper alignment on all operating systems.
 POST_ALIGNTO   - Designates type of variable that should be aligned to a specific alignment.
                    This should follow the type declaration
                    One should use both PRE_ALIGNTO (above) and POST_ALIGNTO to ensure proper alignment on all operating systems.
 NORETURN       - Designates function that never returns
 REGPARM        - Designates function whose first arguments are passed in registers instead of on the stack.
                    The registers used to pass arguments are compiler-dependent, for example:
                    GCC: EAX, EDX, and ECX
                    MS:  ECX and EDX
*/
#if defined(PIN_GNU_COMPATIBLE)

#define UNUSED __attribute__((__unused__))
#define PRE_ALIGNTO(c)
#define POST_ALIGNTO(c) __attribute__((aligned(c)))
#define NORETURN __attribute__((noreturn))
#define REGPARM __attribute__((regparm(3)))
#define NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))

#elif defined(PIN_MS_COMPATIBLE)

#define UNUSED
#define PRE_ALIGNTO(c) __declspec(align(c))
#define POST_ALIGNTO(c)
#define NORETURN __declspec(noreturn)
#define REGPARM __fastcall
#define NO_SANITIZE_ADDRESS

#endif

#ifndef PIN_DEPRECATED_WARNINGS
#if (PIN_NO_DEPRECATED_WARNINGS)
#define PIN_DEPRECATED_WARNINGS 0
#else
/*! @ingroup DEPRECATED
   By default Pin will annotate deprecated parts of the API so that their use in tools will generate
   compiler warnings.
   If you want to suppress these warnings during the compilation of your tool, you can do so by
   defining the macro PIN_DEPRECATED_WARNINGS with the value zero.
*/
#define PIN_DEPRECATED_WARNINGS 1
#endif
#endif

/*! @ingroup DEPRECATED
   Apply this macro to deprecated interface definitions to cause warnings when users refer to them.
   When using this macro, also do the following:\n
   [1] change the interface group to DEPRECATED.\n
   [2] add a comment with the new interface that will get propagated into the generated PH files. \n
   [3] add a note DEPRECATED with the new interface at the end of the old interface documentation.\n
*/
#if (PIN_DEPRECATED_WARNINGS == 0)
/* User asked us to ignore these. */
#define PIN_DEPRECATED_API
#else
#if defined(PIN_GNU_COMPATIBLE)
#define PIN_DEPRECATED_API __attribute__((deprecated))
#elif defined(PIN_MS_COMPATIBLE)
#define PIN_DEPRECATED_API __declspec(deprecated)
#else
/* Unknown compiler */
#define PIN_DEPRECATED_API
#endif
#endif

/*
 Type qualifiers.
 Serve as keywords in generating headers and code conventions checks.
*/

#ifdef __cplusplus
#define GLOBALCFUN extern "C"
/* SPECIALCFUN is not auto-exported */
#define SPECIALCFUN extern "C"
#else
#define GLOBALCFUN extern
#define SPECIALCFUN extern
#endif

#if defined(TARGET_WINDOWS)
#pragma section(".CRT$XIU", read)
#define GLOBALDLLFUN extern __declspec(dllexport)
#define GLOBALDLLCFUN GLOBALCFUN __declspec(dllexport)
#define IMPORTVAR extern __declspec(dllimport)
#define CONSTRUCTOR_FUN(fun)                                            \
    int __cdecl fun(void);                                              \
    __declspec(allocate(".CRT$XIU")) int(__cdecl * fun##_)(void) = fun; \
    static int __cdecl fun(void)
#define CONST_COMDATVAR(type, name, val) extern const __declspec(selectany) type name = val
#else
#define GLOBALDLLFUN extern
#define GLOBALDLLCFUN GLOBALCFUN
#define IMPORTVAR extern
#define CONSTRUCTOR_FUN(fun)                    \
    int fun(void) __attribute__((constructor)); \
    static int fun(void)
#define WEAK_CFUN_DECL(fun_signature) GLOBALCFUN fun_signature __attribute__((weak))

#define CONST_COMDATVAR(type, name, val)          \
    extern const __attribute__((weak)) type name; \
    const type name = val
#endif

#ifdef __cplusplus
#define LOCALCFUN extern "C"
#else
#define LOCALCFUN extern
#endif

#ifdef __cplusplus
#define GLOBALCVAR extern "C"
#else
#define GLOBALCVAR extern
#endif

#if defined(TARGET_WINDOWS)
/* Defines global pointer to an object of specified type.
     The pointer is considered either exported as data object or imported depending on VAR_EXPORTED.
     The name of pointer is created with prefix of Import Address Table entry.
     This way all references to the pointer in all kit images use the same name
     regardless of export / import status.
     When exported, the pointer is statically initialized with specified value.
     Note: to become exported a separate linker directive should be provided.
     It is expected (but not validated) only one exported pointer is defined in whole kit.
  */
#if defined(VAR_EXPORTED)
#if defined(TARGET_IA32)
#define GLOBALDLLCVAR(type, name, val)                         \
    GLOBALCVAR type name                                = val; \
    GLOBALCVAR __declspec(selectany) type* _imp__##name = &name
#else
#define GLOBALDLLCVAR(type, name, val)                         \
    GLOBALCVAR type name                                = val; \
    GLOBALCVAR __declspec(selectany) type* __imp_##name = &name
#endif
#else
#define GLOBALDLLCVAR(type, name, val) GLOBALCVAR __declspec(dllimport) type name
#endif
#if defined(EXPORT_CLASS)
#define EXPORT_CLASS_DECL __declspec(dllexport)
#else
#define EXPORT_CLASS_DECL
#endif
#else
#define GLOBALDLLCVAR(type, name, val) \
    GLOBALCVAR type name;              \
    type name = val
#endif

#define STATIC static

#endif // ASM_ONLY

#endif
