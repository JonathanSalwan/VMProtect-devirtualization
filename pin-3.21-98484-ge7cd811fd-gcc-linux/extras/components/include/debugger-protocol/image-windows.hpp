/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: debugger-protocol
// <FILE-TYPE>: component public header

#ifndef DEBUGGER_PROTOCOL_IMAGE_WINDOWS_HPP
#define DEBUGGER_PROTOCOL_IMAGE_WINDOWS_HPP

namespace DEBUGGER_PROTOCOL
{
/*!
 * In the future, new image types may be added.  To retain backward compatibility,
 * clients should ignore types they don't recognize.
 */
enum IMAGE_TYPE_WINDOWS
{
    /*!
     * Image is from the main PE executable file that is loaded into the target process.
     */
    IMAGE_TYPE_WINDOWS_PE_MAIN,

    /*!
     * Image is from a PE file representing a DLL in the target process.
     */
    IMAGE_TYPE_WINDOWS_PE_LIBRARY
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
enum IMAGE_INFO_WINDOWS_VERSION
{
    IMAGE_INFO_WINDOWS_VERSION_0 ///< This is the only defined version currently.
};

/*!
 * Information about an image in the target application.
 */
struct /*<UTILITY>*/ IMAGE_INFO_WINDOWS
{
    IMAGE_INFO_WINDOWS_VERSION _version; ///< Tells which fields in this structure are valid.
    IMAGE_TYPE_WINDOWS _type;            ///< The image type.
    ANYADDR _base;                       ///< Base address at which the PE file is loaded into memory.
    size_t _size;                        ///< Total size in bytes of the loaded image in memory.
    std::string _name;                   ///< Absolute pathname to the PE file (UTF-8).
};

} // namespace DEBUGGER_PROTOCOL
#endif // file guard
