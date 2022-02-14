/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_IMG_PH
#error duplicate inclusion of img
#else
#define PIN_G_IMG_PH
/*! @file
  This file contains IMG primitives
 */
/*! @ingroup IMG
*/
typedef enum
{
    IMG_TYPE_INVALID,
    IMG_TYPE_STATIC,       ///< Main image, linked with -static
    IMG_TYPE_SHARED,       ///< Main image, linked against shared libraries
    IMG_TYPE_SHAREDLIB,    ///< Shared library or main image linked with -pie
    IMG_TYPE_RELOCATABLE,  ///< Relocatble object (.o file)
    IMG_TYPE_DYNAMIC_CODE, ///< Dynamically created code
    IMG_TYPE_API_CREATED,  ///< Artificial image created by @ref IMG_CreateAt ()
    IMG_TYPE_LAST
} IMG_TYPE;

/*! @ingroup IMG
 * Properties available for an image. Typically defined as meta data within the image.
 * May define some specific hardware that is used or needed or some features enabled by the image.
 * Property may be OS/CPU specific.
 */
typedef enum
{
    IMG_PROPERTY_INVALID,
    /// CET shadow stack enabled.
    /// Supported only for Linux
    IMG_PROPERTY_SHSTK_ENABLED,
    /// CET indirect branch tracking enabled.
    /// Supported only for Linux
    IMG_PROPERTY_IBT_ENABLED,
    IMG_PROPERTY_LAST
} IMG_PROPERTY;

#endif // PIN_G_IMG_PH
