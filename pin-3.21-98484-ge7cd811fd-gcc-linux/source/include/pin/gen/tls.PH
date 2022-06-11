/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_TLS_PH
#error duplicate inclusion of tls
#else
#define PIN_G_TLS_PH
/*! @file

 */
/*! @ingroup THREADS
 *  Type that represents TLS key - a unique identifier of a slot in the thread local storage.
 */
typedef INT32 TLS_KEY;

/*! @ingroup THREADS
 *  Invalid value of the TLS_KEY type.
 */
const TLS_KEY INVALID_TLS_KEY = (-1);

#endif // PIN_G_TLS_PH
