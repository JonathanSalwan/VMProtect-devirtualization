/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_IMAGE_ELF_PH
#error duplicate inclusion of image_elf
#else
#define PIN_G_IMAGE_ELF_PH
/*! @file
    Static model for instrumenting IMG's once they are loaded into memory.
    This file contains the ELF specific part for Linux.
*/
#if defined(TARGET_LINUX)
#endif //defined(TARGET_LINUX)
#endif // PIN_G_IMAGE_ELF_PH
