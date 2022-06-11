#ifndef __stl_config__pin_h
#define __stl_config__pin_h

#define _STLP_PLATFORM "Pin"

// Mostly Unix-like.
#define _STLP_UNIX 1

// Have pin threads support.
#define _STLP_PIN_THREADS

// Don't have native <cplusplus> headers
#define _STLP_HAS_NO_NEW_C_HEADERS 1

#ifdef TARGET_LINUX
// Use unistd for streams since stdio implementation fails to read
// file from the proc file system
#define _STLP_USE_UNIX_IO 1
#else
#ifdef __linux__
#error Build cross OS not supported
#endif
// Use stdio for streams
#define _STLP_USE_STDIO_IO 1
#endif

// No rtti support
#define _STLP_NO_RTTI 1

// Don't let STLPORT try to auto-link the program with pragma comment
#define _STLP_DONT_USE_AUTO_LINK 1

// C library is in the global namespace.
#define _STLP_VENDOR_GLOBAL_CSTD 1

// Don't have underlying local support.
#undef _STLP_REAL_LOCALE_IMPLEMENTED

// No pthread_spinlock_t in Android
#define _STLP_DONT_USE_PTHREAD_SPINLOCK 1

// Enable thread support
#undef _NOTHREADS

#define _STLP_THREADS

// Little endian platform.
#define _STLP_LITTLE_ENDIAN 1

// No <exception> headers
#define _STLP_NO_EXCEPTION_HEADER 1

// No throwing exceptions
#define _STLP_NO_EXCEPTIONS 1

#define _STLP_OWN_STDEXCEPT 1

// Don't use STLPort allocator
#define _STLP_USE_MALLOC 1

// No need to define our own namespace
#define _STLP_NO_OWN_NAMESPACE 1

// Use __new_alloc instead of __node_alloc, so we don't need static functions.
#define _STLP_USE_SIMPLE_NODE_ALLOC 1

// Don't use extern versions of range errors, so we don't need to
// compile as a library.
#define _STLP_USE_NO_EXTERN_RANGE_ERRORS 1

// The system math library doesn't have long double variants, e.g
// sinl, cosl, etc
#define _STLP_NO_VENDOR_MATH_L 1

// Define how to include our native headers.
#define _STLP_NATIVE_HEADER(header) <libstdc++/include/header>
#define _STLP_NATIVE_C_HEADER(header) <include/header>
#define _STLP_NATIVE_CPP_C_HEADER(header) <libstdc++/include/header>
#define _STLP_NATIVE_CPP_RUNTIME_HEADER(header) <libstdc++/include/header>
#define _STLP_NATIVE_OLD_STREAMS_HEADER(header) <libstdc++/include/header>
#define _STLP_NATIVE_CPP_RUNTIME_HEADER(header) <libstdc++/include/header>

#ifndef _CRT_SECURE_NO_DEPRECATE
# define _CRT_SECURE_NO_DEPRECATE
#endif

#if defined(__clang__)
# include <stl/config/_clang.h>
#elif defined(_MSC_VER)
# include <stl/config/_msvc.h>
#else
// Include most of the gcc settings.
# include <stl/config/_gcc.h>
#endif

// Do not use glibc, Android is missing some things.
#undef _STLP_USE_GLIBC

// No exceptions.
#ifndef _STLP_NO_UNCAUGHT_EXCEPT_SUPPORT
# define _STLP_NO_UNCAUGHT_EXCEPT_SUPPORT 1
#endif
#ifndef _STLP_NO_UNEXPECTED_EXCEPT_SUPPORT
# define _STLP_NO_UNEXPECTED_EXCEPT_SUPPORT 1
#endif

#if !defined(__BUILDING_STLPORT)
// Turn off container extension which fails correct, standard-compliant code.
// Note: Container extension uses template member function to implement
// insert, find, ... member functions of the containers.
#define _STLP_NO_CONTAINERS_EXTENSION 1
#endif

// Without this, we don't get int16_t/uint16_t num_get::get overloads.
#define _STLP_FIX_LIBRARY_ISSUES 1

#endif /* __stl_config__pin_h */
