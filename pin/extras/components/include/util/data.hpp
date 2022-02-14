/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: util
// <FILE-TYPE>: component public header

#ifndef UTIL_DATA_HPP
#define UTIL_DATA_HPP

#include <string>
#include <cstring>

#include "types.h"

namespace UTIL
{
/*!
 * Utility that holds a raw data buffer.  The internal implementation uses reference
 * counting, so the various copy and "slice" operations are fast.
 *
 * None of the operations are thread safe, so the caller must provide any necessary
 * synchronization.  Since the implementation uses reference counting, two distinct
 * DATA objects may actually share a buffer and require mutual synchronization.  To
 * avoid this, use DATA::MakeUnique() if necessary.
 */
class /*<UTILITY>*/ DATA
{
  public:
    /*!
     * This is used when constructing a DATA that is a copy of some sort of input buffer.
     */
    enum COPY
    {
        /*!
         * The input buffer is copied immediately.
         */
        COPY_EAGER,

        /*!
         * The input buffer is copied only if the DATA is modified.  However, the input
         * buffer must remain valid and must not be modified until one of the following
         * occurs:
         *
         *  - The DATA is destroyed.
         *  - The Assign() method or "=" (the assignment operator) reconstructs the DATA.
         *  - A writable buffer is obtained via GetWritableBuf().
         */
        COPY_LAZY
    };

    /*!
     * Tells what to do when a DATA buffer is allocated and no initial value is specified.
     */
    enum FILL
    {
        FILL_UNSPECIFIED, ///< Leave the buffer unspecified.
        FILL_ZERO         ///< Zero fill the buffer.
    };

  public:
    /*!
     * Construct a new empty buffer.
     */
    DATA() : _sbuf(0), _start(0), _size(0) {}

    /*!
     * Construct a new buffer.
     *
     *  @param[in] size     Size (bytes) of the buffer.
     *  @param[in] fill     Tells whether the initial contents are zero-filled or left unspecified.
     */
    DATA(size_t size, FILL fill = FILL_UNSPECIFIED) : _sbuf(new SHARED_BUF(size)), _start(_sbuf->_buf), _size(size)
    {
        if (fill == FILL_ZERO) std::memset(_start, 0, _size);
    }

    /*!
     * Construct a new buffer that is a copy of some existing data.
     *
     *  @param[in] buf      Points to the data to copy.
     *  @param[in] size     Size (bytes) of data in \a buf.
     *  @param[in] copy     Tells whether \a buf is copied eagerly or lazily.
     */
    DATA(const void* buf, size_t size, COPY copy = COPY_EAGER) { CopyFromBuffer(buf, size, copy); }

    /*!
     * Construct a new buffer that is a copy of a C string (not including its terminating NUL).
     *
     *  @param[in] str      NUL-terminated string.
     *  @param[in] copy     Tells whether \a str is copied eagerly or lazily.
     */
    DATA(const char* str, COPY copy = COPY_EAGER) { CopyFromBuffer(str, std::strlen(str), copy); }

    /*!
     * Construct a new buffer that is a copy of an C++ string.
     *
     *  @param[in] str      The string.
     *  @param[in] copy     Tells whether \a str is copied eagerly or lazily.
     */
    DATA(const std::string& str, COPY copy = COPY_EAGER) { CopyFromBuffer(str.c_str(), str.size(), copy); }

    /*!
     * Construct a new buffer that is a copy of a subrange of an existing buffer.
     *
     *  @param[in] other    The new buffer is a copy of the contents of \a other.
     *  @param[in] off      The new buffer starts at \a off bytes from the start of \a other.
     *                       If \a off is larger than \a other, the new buffer is empty.
     */
    DATA(const DATA& other, size_t off = 0) { CopyFromData(other, off); }

    /*!
     * Construct a new buffer that is a copy of a subrange of an existing buffer.
     *
     *  @param[in] other    The new buffer is a copy of the contents of \a other.
     *  @param[in] off      The new buffer starts at \a off bytes from the start of \a other.
     *                       If \a off is larger than \a other, the new buffer is empty.
     *  @param[in] len      The new buffer is at most \a len bytes long.  If \a off + \a len
     *                       is greater than the length of \a other, the new buffer is a copy
     *                       of the data up to the end of \a other.
     */
    DATA(const DATA& other, size_t off, size_t len) { CopyFromDataWithLen(other, off, len); }

    ~DATA() { DetachBuf(); }

    /*!
     * Reconstruct the buffer to be a copy of another buffer.
     *
     *  @param[in] other    The contents of this buffer are copied.
     *
     * @return  Reference to the new data buffer.
     */
    DATA& operator=(const DATA& other)
    {
        Assign(other);
        return *this;
    }

    /*!
     * Reconstruct the buffer to a new size.
     *
     *  @param[in] size     Size (bytes) of the buffer.
     *  @param[in] fill     Tells whether the initial contents are zero-filled or left unspecified.
     */
    void Assign(size_t size, FILL fill = FILL_UNSPECIFIED)
    {
        DetachBuf();
        _sbuf  = new SHARED_BUF(size);
        _start = _sbuf->_buf;
        _size  = size;
        if (fill == FILL_ZERO) std::memset(_start, 0, _size);
    }

    /*!
     * Reconstruct the buffer to be a copy of some existing data.
     *
     *  @param[in] buf      Points to the data to copy.
     *  @param[in] size     Size (bytes) of data in \a buf.
     *  @param[in] copy     Tells whether \a buf is copied eagerly or lazily.
     */
    void Assign(const void* buf, size_t size, COPY copy = COPY_EAGER)
    {
        DetachBuf();
        CopyFromBuffer(buf, size, copy);
    }

    /*!
     * Reconstruct the buffer to be a copy of a C string (not including its terminating NUL).
     *
     *  @param[in] str      NUL-terminated string.
     *  @param[in] copy     Tells whether \a str is copied eagerly or lazily.
     */
    void Assign(const char* str, COPY copy = COPY_EAGER)
    {
        DetachBuf();
        CopyFromBuffer(str, std::strlen(str), copy);
    }

    /*!
     * Reconstruct the buffer to be a copy of an C++ string.
     *
     *  @param[in] str      The string.
     *  @param[in] copy     Tells whether \a str is copied eagerly or lazily.
     */
    void Assign(const std::string& str, COPY copy = COPY_EAGER)
    {
        DetachBuf();
        CopyFromBuffer(str.c_str(), str.size(), copy);
    }

    /*!
     * Reconstruct the buffer to be a copy of a subrange of an existing buffer.
     *
     *  @param[in] other    The contents of this buffer are copied.
     *  @param[in] off      This buffer starts at \a off bytes from the start of \a other.
     *                       If \a off is larger than \a other, the new buffer is empty.
     */
    void Assign(const DATA& other, size_t off = 0)
    {
        if (this != &other)
        {
            DetachBuf();
            CopyFromData(other, off);
        }
        else
        {
            PopFront(off);
        }
    }

    /*!
     * Reconstruct the buffer to be a copy of a subrange of an existing buffer.
     *
     *  @param[in] other    The contents of this buffer are a copy of the contents of \a other.
     *  @param[in] off      This buffer starts at \a off bytes from the start of \a other.
     *                       If \a off is larger than \a other, the new buffer is empty.
     *  @param[in] len      This buffer is at most \a len bytes long.  If \a off + \a len
     *                       is greater than the length of \a other, the new buffer is a copy
     *                       of the data up to the end of \a other.
     */
    void Assign(const DATA& other, size_t off, size_t len)
    {
        if (this != &other)
        {
            DetachBuf();
            CopyFromDataWithLen(other, off, len);
        }
        else
        {
            PopFront(off);
            if (len < _size) _size = len;
        }
    }

    /*!
     * Clear the content of the buffer, making it empty.
     */
    void Clear()
    {
        DetachBuf();
        _sbuf  = 0;
        _start = 0;
        _size  = 0;
    }

    /*!
     * Remove initial bytes from the start of the buffer, making it shorter.
     *
     *  @param[in] num  This many bytes are removed from the buffer.  If \a num
     *                   is larger than the length of the buffer, the buffer becomes empty.
     */
    void PopFront(size_t num)
    {
        if (num >= _size)
        {
            DetachBuf();
            _sbuf  = 0;
            _start = 0;
            _size  = 0;
            return;
        }
        _start = static_cast< UINT8* >(_start) + num;
        _size -= num;
    }

    /*!
     * Remove trailing bytes from the end of the buffer, making it shorter.
     *
     *  @param[in] num  This many bytes are removed from the buffer.  If \a num
     *                   is larger than the length of the buffer, the buffer becomes empty.
     */
    void PopBack(size_t num)
    {
        if (num >= _size)
        {
            DetachBuf();
            _sbuf  = 0;
            _start = 0;
            _size  = 0;
            return;
        }
        _size -= num;
    }

    /*!
     * Change the size of the buffer, retaining it's current content.  If the new size is
     * smaller than the previous size, trailing bytes in the buffer are lost.  If the new
     * size is greater than the previous size, the \a fill parameter tells whether the new
     * buffer space is left unspecified or is zero-filled.
     *
     *  @param[in] newSize  The new buffer size (bytes).
     *  @param[in] fill     Tells what to do with the new buffer space if \a newSize is
     *                       greater than the existing buffer size.
     */
    void Resize(size_t newSize, FILL fill = FILL_UNSPECIFIED)
    {
        if (newSize <= _size)
        {
            if (!newSize)
            {
                DetachBuf();
                _sbuf  = 0;
                _start = 0;
                _size  = 0;
                return;
            }
            _size = newSize;
        }
        else
        {
            // If we grow, we must re-allocate the input buffer, so we may as well copy
            // any "lazy" buffer too.  Note that we do NOT want to share the copied buffer
            // with any other DATA because doing so would invalidate any GetBuf() pointers
            // in the other DATA.  Therefore, any other DATA's continue to use the old buffer.
            //
            SHARED_BUF* sbuf = new SHARED_BUF(newSize);
            std::memcpy(sbuf->_buf, _start, _size);
            if (fill == FILL_ZERO) std::memset(static_cast< UINT8* >(sbuf->_buf) + _size, 0, newSize - _size);
            DetachBuf();
            _sbuf  = sbuf;
            _start = sbuf->_buf;
            _size  = newSize;
        }
    }

    /*!
     * Calling this function ensures that the buffer does not share any data with any other
     * DATA object, copying any shared buffer if necessary.  This could be useful, for example,
     * to ensure that two DATA objects can be safely used by different threads.
     */
    void MakeUnique()
    {
        // If no other DATA's share our buffer, there's nothing to do.
        //
        if (!_sbuf || _sbuf->_refCount == 1 || _sbuf->_refCount == EXCLUSIVE) return;

        // Since we need to copy the buffer anyways, this also copies any "lazy" buffer.
        //
        _sbuf->_refCount--;
        _sbuf = new SHARED_BUF(_size);
        std::memcpy(_sbuf->_buf, _start, _size);
        _start = _sbuf->_buf;
    }

    /*!
     * @return  Size (bytes) of the buffer.
     */
    size_t GetSize() const { return _size; }

    /*!
     * @return  A pointer to the buffer's data.  The returned pointer remains valid only until
     *           the next operation that modifies the DATA.
     */
    template< typename T > const T* GetBuf() const { return static_cast< const T* >(_start); }

    /*!
     * Get a writable pointer to the buffer's data.  This is considered a modifying operation, so
     * pointers returned by previous calls to GetBuf() are no longer valid.  When you are
     * finished with the pointer, call ReleaseWritableBuf() to release it.
     *
     * @return  A pointer to the buffer's data.  The returned pointer remains valid only until
     *           the next operation that modifies the DATA or until ReleaseWritableBuf() is
     *           called.
     */
    template< typename T > T* GetWritableBuf()
    {
        // Since the caller could use this pointer to change the contents of the buffer,
        // we must make sure that doing so won't change the contents of any other DATA's
        // buffer.
        //
        if (_sbuf && _sbuf->_refCount != EXCLUSIVE)
        {
            // We must make a private copy if there are other DATA's sharing this buffer.
            // This also copies any "lazy" buffer.
            //
            if (_sbuf->_refCount > 1)
            {
                _sbuf->_refCount--;
                _sbuf = new SHARED_BUF(_size);
                std::memcpy(_sbuf->_buf, _start, _size);
                _start = _sbuf->_buf;
            }

            // Even if we're the only DATA that references this buffer, it can't be
            // a "lazy" buffer.
            //
            else if (_sbuf->_isLazy)
            {
                UINT8* buf = new UINT8[_size];
                std::memcpy(buf, _start, _size);
                _sbuf->_buf    = buf;
                _sbuf->_size   = _size;
                _sbuf->_isLazy = 0;
                _start         = buf;
            }

            // Mark the buffer as exclusively ours.  This prevents any future DATA from
            // sharing our buffer while there is a writable pointer to it.
            //
            _sbuf->_refCount = EXCLUSIVE;
        }

        return static_cast< T* >(_start);
    }

    /*!
     * Release all previously obtained writable pointers obtained via GetWritableBuf().
     */
    void ReleaseWritableBuf()
    {
        if (_sbuf && _sbuf->_refCount == EXCLUSIVE) _sbuf->_refCount = 1;
    }

  private:
    /*!
     * Construct this DATA by copying an external buffer.
     *
     *  @param[in] buf      Points to the data to copy.
     *  @param[in] size     Size (bytes) of data in \a buf.
     *  @param[in] copy     Tells whether \a buf is copied eagerly or lazily.
     */
    void CopyFromBuffer(const void* buf, size_t size, COPY copy)
    {
        if (copy == COPY_EAGER)
        {
            _sbuf = new SHARED_BUF(size);
            std::memcpy(_sbuf->_buf, buf, size);
        }
        else
        {
            _sbuf = new SHARED_BUF(buf, size);
        }
        _start = _sbuf->_buf;
        _size  = size;
    }

    /*!
     * Construct this DATA by copying a slice of another DATA.
     *
     *  @param[in] other    The new buffer is a copy of the contents of \a other.
     *  @param[in] off      The new buffer starts at \a off bytes from the start of \a other.
     *                       If \a off is larger than \a other, the new buffer is empty.
     */
    void CopyFromData(const DATA& other, size_t off)
    {
        // Check for a zero-length buffer.
        //
        if (off >= other._size)
        {
            _sbuf  = 0;
            _start = 0;
            _size  = 0;
            return;
        }

        // If 'other' has an outstanding writable pointer, we can't share its buffer because
        // we don't want changes to 'other' to affect us.
        //
        if (other._sbuf->_refCount == EXCLUSIVE)
        {
            _size = other._size - off;
            _sbuf = new SHARED_BUF(_size);
            std::memcpy(_sbuf->_buf, static_cast< UINT8* >(other._start) + off, _size);
            _start = _sbuf->_buf;
            return;
        }

        // Otherwise, we can just share the buffer.
        //
        _sbuf = other._sbuf;
        _sbuf->_refCount++;
        _start = static_cast< UINT8* >(other._start) + off;
        _size  = other._size - off;
    }

    /*!
     * Construct this DATA by copying a slice of another DATA.
     *
     *  @param[in] other    The new buffer is a copy of the contents of \a other.
     *  @param[in] off      The new buffer starts at \a off bytes from the start of \a other.
     *                       If \a off is larger than \a other, the new buffer is empty.
     *  @param[in] len      The new buffer is at most \a len bytes long.  If \a off + \a len
     *                       is greater than the length of \a other, the new buffer is a copy
     *                       of the data up to the end of \a other.
     */
    void CopyFromDataWithLen(const DATA& other, size_t off, size_t len)
    {
        // Check for a zero-length buffer.
        //
        if (off >= other._size)
        {
            _sbuf  = 0;
            _start = 0;
            _size  = 0;
            return;
        }

        if (len > other._size - off)
            _size = other._size - off;
        else
            _size = len;

        // If 'other' has an outstanding writable pointer, we can't share its buffer because
        // we don't want changes to 'other' to affect us.
        //
        if (other._sbuf->_refCount == EXCLUSIVE)
        {
            _sbuf = new SHARED_BUF(_size);
            std::memcpy(_sbuf->_buf, static_cast< UINT8* >(other._start) + off, _size);
            _start = _sbuf->_buf;
            return;
        }

        // Otherwise, we can just share the buffer.
        //
        _sbuf = other._sbuf;
        _sbuf->_refCount++;
        _start = static_cast< UINT8* >(other._start) + off;
    }

    /*!
     * If there is an attached SHARED_BUF, detach it and remove it if there are no other
     * owners.
     */
    void DetachBuf()
    {
        if (_sbuf && ((_sbuf->_refCount == EXCLUSIVE) || (--(_sbuf->_refCount) == 0)))
        {
            if (_sbuf->_isLazy == 0) delete[] static_cast< UINT8* >(_sbuf->_buf);
            delete _sbuf;
        }
    }

  private:
    static const UINT32 EXCLUSIVE = UINT32(0x7fffffff);

    // This is potentially shared by many DATA instances.
    //
    struct SHARED_BUF
    {
        // Constructor for an allocated buffer.
        //
        SHARED_BUF(size_t sz) : _refCount(1), _isLazy(0), _size(sz), _buf(new UINT8[_size]) {}

        // Constructor for an lazy-copied buffer.
        //
        SHARED_BUF(const void* buf, size_t sz) : _refCount(1), _isLazy(1), _size(sz), _buf(const_cast< void* >(buf)) {}

        UINT32 _refCount : 31; // Number of DATA's pointing to this SHARED_BUF, or EXCLUSIVE.
        UINT32 _isLazy : 1;    // Tells if '_buf' is lazy-copied or allocated.
        size_t _size;          // Size of '_buf'.

        // If '_isLazy' is TRUE, this points to a lazy-copied input buffer and must be treated as "const".
        // If '_isLazy' is FALSE, this points to a "new[]" allocated buffer.
        //
        void* _buf;
    };

    SHARED_BUF* _sbuf;
    void* _start; // Start of my instance's data in _buf (treated as "const" if '_isLazy' is TRUE).
    size_t _size; // Size of my instance's data
};

} // namespace UTIL
#endif // file guard
