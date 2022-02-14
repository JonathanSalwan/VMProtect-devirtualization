/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: debugger-protocol
// <FILE-TYPE>: component public header

#ifndef DEBUGGER_PROTOCOL_IMAGE_MAC_HPP
#define DEBUGGER_PROTOCOL_IMAGE_MAC_HPP

namespace DEBUGGER_PROTOCOL
{
/*!
 * In the future, new image types may be added.  To retain backward compatibility,
 * clients should ignore types they don't recognize.
 */
enum IMAGE_TYPE_MAC
{
    IMAGE_TYPE_MAC_MACHO_MAIN
};

/*!
 * In the future, new fields may be added to the end of the IMAGE_INFO_MAC structure.
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
enum IMAGE_INFO_MAC_VERSION
{
    IMAGE_INFO_MAC_VERSION_0 ///< This is the only defined version currently.
};

/*!
 * Information about an image in the target application.
 */
struct /*<UTILITY>*/ IMAGE_INFO_MAC
{
    IMAGE_INFO_MAC_VERSION _version; ///< Define which fields are valid.
    IMAGE_TYPE_MAC _type;            ///< The image type.
    std::string _name;               ///< Absolute pathname to the ELF file (UTF-8).
    ANYADDR _offset;                 ///< Offset from ELF file's link-time address to it's loaded address.
};

} // namespace DEBUGGER_PROTOCOL
#endif // file guard
