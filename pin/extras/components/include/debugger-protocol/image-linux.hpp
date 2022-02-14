/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: debugger-protocol
// <FILE-TYPE>: component public header

#ifndef DEBUGGER_PROTOCOL_IMAGE_LINUX_HPP
#define DEBUGGER_PROTOCOL_IMAGE_LINUX_HPP

#include <vector>

namespace DEBUGGER_PROTOCOL
{
/*!
 * In the future, new image types may be added.  To retain backward compatibility,
 * clients should ignore types they don't recognize.
 */
enum IMAGE_TYPE_LINUX
{
    /*!
     * Image is from the main ELF executable file that is loaded into the target process.
     */
    IMAGE_TYPE_LINUX_ELF_MAIN,

    /*!
     * Image is from an ELF file representing a shared library in the target process.
     */
    IMAGE_TYPE_LINUX_ELF_LIBRARY
};

/*!
 * In the future, new fields may be added to the end of the IMAGE_INFO_LINUX structure.
 * If this happens, clients can use the \e _version field to retain backward
 * compatibility.
 *
 * When a client writes information to this structure, it should set \e _version
 * to the latest version that it supports.
 *
 * When a client reads this structure, it should use \e _version to tell which
 * fields are valid.  A client should allow that \e _version may be greater than
 * the newest version it knows about, which happens if an older front-end runs
 * with a newer back-end or vice-versa.
 */
enum IMAGE_INFO_LINUX_VERSION
{
    IMAGE_INFO_LINUX_VERSION_0 ///< This is the only defined version currently.
};

/*!
 * Information about an image in the target application.
 */
struct /*<UTILITY>*/ IMAGE_INFO_LINUX
{
    IMAGE_INFO_LINUX_VERSION _version; ///< Tells which fields in this structure are valid.
    IMAGE_TYPE_LINUX _type;            ///< The image type.
    std::string _name;                 ///< Absolute pathname to the ELF file (UTF-8).
    ANYADDR _offset;                   ///< Offset from ELF file's link-time address to it's loaded address.
    ADDRINT _lm;                       ///< Pointer to the image's struct link_map
    ADDRINT _l_ld;                     ///< Points to the image's PT_DYNAMIC segment
};

/*!
 * Images list in the target application for GDB.
 * Complying with the SVR4 standard.
 */
struct /*<UTILITY>*/ SVR4_IMAGES_LIST_LINUX
{
    SVR4_IMAGES_LIST_LINUX() : _main_lm(0) {} ///< Initialization
    ADDRINT _main_lm;                         ///< Pointer to the main image's struct link_map
    std::vector< IMAGE_INFO_LINUX > _list;    ///< List of all loaded images
};

} // namespace DEBUGGER_PROTOCOL
#endif // file guard
