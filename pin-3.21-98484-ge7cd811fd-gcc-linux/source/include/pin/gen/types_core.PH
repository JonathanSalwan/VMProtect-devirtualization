/*
 * Copyright (C) 2003-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  this headerfile contains defines the (additional) types needed by @ref LEVEL_CORE
 */

#ifndef TYPES_CORE_PH
#define TYPES_CORE_PH

//
// The following is quite an ugly hack but it reduces code duplication significantly
//

/*
  INDEX is a wrapper template class to encapsulate an INT32, dummy template arg is used 
  to enable type checking 
 */

template< int dummy > class INDEX
{
  public:
    /*
      INT32 index; must be public - so that both vs8 and icc will treat
      functions that return  INDEX<num> classes (such as BBL) the same  - i.e. will return
      the value in a register without passing an implicit param into the function that is
      a pointer to the location to store the return value to.
      If it is private then vs8 will do the implicit passing and icc will not.
      This incompatability is supposed to be fixed in icc version 11.0
    */
    INT32 index;
    BOOL operator==(const INDEX< dummy > right) const { return right.index == index; }
    BOOL operator!=(const INDEX< dummy > right) const { return right.index != index; }
    BOOL operator<(const INDEX< dummy > right) const { return index < right.index; }

    // please do not try to introduce a constructor here
    // otherwise programs will not compile
    // and there are possibly performance penalties as well
    //INDEX(INT32 x=0) : index(x) {}
    INT32 q() const { return index; }
    BOOL is_valid() const { return (index > 0); }
    VOID q_set(INT32 y) { index = y; }
    VOID invalidate() { index = 0; }
};

/*! @ingroup APP
Handle for APP */
typedef class INDEX< 0 > APP;
/*! @ingroup IMG
Handle for IMG */
typedef class INDEX< 1 > IMG;
/*! @ingroup SEC
Handle for SEC */
typedef class INDEX< 2 > SEC;
/*! @ingroup RTN
Handle for RTN */
typedef class INDEX< 3 > RTN;
/*! @ingroup BBL
Handle for BBL */
typedef class INDEX< 4 > BBL;
/*! @ingroup EDG
Handle for EDG */
typedef class INDEX< 5 > EDG;
/*! @ingroup INS_INSPECTION
Handle for INS */
typedef class INDEX< 6 > INS;
/*! @ingroup REL
Handle for REL */
typedef class INDEX< 7 > REL;
typedef class INDEX< 8 > EXT;
/*! @ingroup SYMBOLS
Handle for SYM */
typedef class INDEX< 9 > SYM;

typedef class INDEX< 20 > CHUNK;
typedef class INDEX< 21 > LDEF;

typedef class INDEX< 22 > GOT;

// scheduler stuff
typedef class INDEX< 100 > BBX;
typedef class INDEX< 102 > EDX;
typedef class INDEX< 103 > INX;

// for initialization and performance reasons we avoid having an explicit constructor

template< int dummy > class ADDR
{
  private:
    static const ADDRINT INVALID_ADDR = ~ADDRINT(0);

    ADDRINT address;

  public:
    //explicit ADDR(PTRINT v) : address(v) {}

    VOID value_set(ADDRINT v = INVALID_ADDR) { address = v; }

    VOID add(ADDRINT v) { address += v; }
    VOID sub(ADDRINT v) { address -= v; }
    VOID round_up(UINT32 alignment) { address = RoundUp(address, alignment); }
    VOID round_down(UINT32 alignment) { address = RoundDown(address, alignment); }

    ADDRINT value() const { return address; }
    UINT32 value_mod_2() const { return address % 2; }
    UINT32 value_mod_4() const { return address % 4; }
    UINT32 value_mod_8() const { return address % 8; }
    UINT32 value_mod_16() const { return address % 16; }
    UINT32 value_mod_32() const { return address % 32; }

    BOOL valid() const { return address != INVALID_ADDR; }
    BOOL zero() const { return address == ADDRINT(0); }
    ADDRDELTA diff(ADDR< dummy > a) const { return address - a.address; }

    int operator==(const ADDR< dummy > right) const { return right.address == address; }
    int operator!=(const ADDR< dummy > right) const { return right.address != address; }

    bool operator<(const ADDR< dummy > right) const { return address < right.address; }
    bool operator<=(const ADDR< dummy > right) const { return address <= right.address; }
    bool operator>(const ADDR< dummy > right) const { return address > right.address; }
    bool operator>=(const ADDR< dummy > right) const { return address >= right.address; }

    static int cmp(const ADDR< dummy > lhs, const ADDR< dummy > rhs)
    {
        if (lhs < rhs)
            return -1;
        else if (rhs < lhs)
            return 1;
        else
            return 0;
    }

    static int qcmp(const void* x1, const void* x2)
    {
        return cmp(*static_cast< const ADDR< dummy >* >(x1), *static_cast< const ADDR< dummy >* >(x2));
    }
};

template< int dummy > inline std::string str(ADDR< dummy > a) { return StringFromAddrint(a.value()); }

#endif
