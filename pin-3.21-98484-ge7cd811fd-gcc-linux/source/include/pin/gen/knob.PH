/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_KNOB_PH
#error duplicate inclusion of knob
#else
#define PIN_G_KNOB_PH
/*! @file
  This module contains routines and classes for the processing of command line
  arguments.
*/
/*! @ingroup KNOBS
  Per knob mode - indicates how multiple appearances of the mode on the command line are handled
 */
typedef enum
{
    KNOB_MODE_INVALID,
    KNOB_MODE_COMMENT,    ///< comment for knob family
    KNOB_MODE_WRITEONCE,  ///< single value, single write
    KNOB_MODE_OVERWRITE,  ///< single value, overwrite, unless the user set the knob from command line.
    KNOB_MODE_ACCUMULATE, ///< single value, update
    KNOB_MODE_APPEND,     ///< list of values, append
    KNOB_MODE_LAST
} KNOB_MODE;

/*! @ingroup KNOBS
  
  typed and string representation a value

   Interface requirements for TYPE:
   - TYPE FromString(const string &) ... conversion from string to TYPE;
                                         this is always needed;
   - string ToString(const TYPE &) ..... conversion from TYPE to string
                                         default implementation uses the
                                         original string; specialization
                                         can be used to override this;
   - TYPE Accumulate(TYPE & lVal, 
                     const TYPE & rVal). add rVal to lVal and return new lVal;
                                         by defult implemented as operator +=.
                                         Only used for accumulate mode
*/
template< class TYPE > class KNOBVALUE
{
  private:
    TYPE _value;               ///< typed representation
    std::string _value_string; ///< string representation
    KNOBVALUE< TYPE >* _next;  ///< list pointer

    // Disable compiler generated copy c'tors
    KNOBVALUE(const KNOBVALUE&) : _next(NULL) { PIN_ERROR("KNOBVALUE should not be copied"); }
    // Disable compiler generated assignment operator
    KNOBVALUE& operator=(const KNOBVALUE&) { PIN_ERROR("KNOBVALUE should not be copied"); };

  public:
    // conversion methods
    STATIC TYPE FromString(const std::string& strvalue);
    STATIC std::string ToString(const TYPE& value, const std::string& origString) { return origString; }

    //acumulation method
    STATIC TYPE Accumulate(TYPE& lVal, const TYPE& rVal) { return (lVal += rVal); }

    // type name accessor
    STATIC const std::string Type();

    //
    // constructors/destructors
    //
    KNOBVALUE() : _value(TYPE()), _next(NULL) {}

    //
    // accessors
    //
    const TYPE& Value() const { return _value; }
    const std::string& ValueString() const { return _value_string; }

    KNOBVALUE* Next() const { return _next; }

    //
    // modifiers
    //
    VOID Overwrite(const std::string& valstring)
    {
        _value        = FromString(valstring);
        _value_string = ToString(_value, valstring);
    }
    VOID Accumulate(const std::string& valstring)
    {
        Accumulate(_value, FromString(valstring));
        _value_string = ToString(_value, valstring);
    }
    VOID Append(KNOBVALUE* next)
    {
        ASSERTX(_next == NULL);
        _next = next;
    }
};

/*! @ingroup KNOBS
  Specialization of the Accumulate(TYPE &, const TYPE &) method for [TYPE=BOOL].
  Implemented as logical OR of values.
*/
template<> inline BOOL KNOBVALUE< BOOL >::Accumulate(BOOL& lVal, const BOOL& rVal) { return (lVal = (lVal || rVal)); }

/*! @ingroup KNOBS
  List of values attached to a knob
*/
template< class TYPE > class KNOBVALUE_LIST
{
  private:
    typedef KNOBVALUE< TYPE > listnode;
    listnode _first;  ///< inline first node (for scalar value speed)
    UINT32 _numNodes; ///< number of nodes in list

  public:
    //
    // constructors/destructors
    //
    KNOBVALUE_LIST(const std::string& valstring, BOOL hasDefault) : _numNodes(0)
    {
        if (hasDefault)
        {
            Append(valstring);
        }
    }
    ~KNOBVALUE_LIST()
    {
        listnode* next;
        for (listnode* node = _first.Next(); node; node = next)
        {
            next = node->Next();
            delete node;
        }
    }

    //
    // accessors
    //
  private:
    // Disable compiler generated copy constructor
    KNOBVALUE_LIST(const KNOBVALUE_LIST&) { PIN_ERROR("KNOBVALUE_LIST should not be copied"); }
    // Disable compiler generated assignment operator
    KNOBVALUE_LIST& operator=(const KNOBVALUE_LIST&) { PIN_ERROR("KNOBVALUE_LIST should not be copied"); }

    // we need const and non-const versions for type checking, but
    // only want a single imlementation, thus the ugly const_cast
    const listnode* Node(const UINT32 index) const { return GetNode(index); }
    listnode* Node(const UINT32 index) { return const_cast< listnode* >(GetNode(index)); }
    const listnode* GetNode(const UINT32 index) const
    {
        ASSERT(_numNodes > index, "access index out of range");

        const listnode* node = &_first;
        for (UINT32 i = index; i > 0; i--)
        {
            node = node->Next();
            ASSERTX(node != NULL);
        }

        return node;
    }

  public:
    UINT32 NumberOfValues() const { return _numNodes; }
    const std::string& ValueString(const UINT32 index) const { return Node(index)->ValueString(); }
    const TYPE& Value(const UINT32 index) const { return Node(index)->Value(); }
    const TYPE& Value() const { return _first.Value(); }
    const std::string& ValueString() const { return _first.ValueString(); }

    //
    // modifiers
    //
    VOID Overwrite(const std::string& valstring) { _first.Overwrite(valstring); }
    VOID Accumulate(const std::string& valstring) { _first.Accumulate(valstring); }
    VOID Append(const std::string& valstring)
    {
        listnode* node;
        if (_numNodes == 0)
        {
            node = &_first;
        }
        else
        {
            node                 = new listnode;
            listnode* const last = Node(_numNodes - 1);
            last->Append(node);
        }
        node->Overwrite(valstring);
        _numNodes++;
    }
};

/*! @ingroup KNOBS
  keeps maintenance information that is independent of knob type
*/
class KNOB_BASE
{
  private:
    STATIC KNOB_BASE* _knob_list;
    STATIC BOOL _parsing_done;
    STATIC BOOL _developer_flags_enabled; // True when xyzzy is used
    BOOL _set_by_user;                    ///< An indicator to whether or not the user gave the knob a value in the command line.

  protected:
    KNOB_BASE* _next;
    const std::string _family;
    const std::string _name;
    const std::string _purpose;
    const std::string _default_value;
    const KNOB_MODE _mode;
    BOOL _disabled;
    INT32 _nreads;
    INT32 _nwrites;

  public:
    //
    // constructors/destructors
    //
    /*! @ingroup KNOBS
      Create a new knob
      @param myname Name of the knob
      @param myfamily Family to which the knob belongs to
      @param mydefault The default value to which the knob is initialized with
      @param mypurpose A string that explains the purpose of the knob
      @param mymode @ref KNOB_MODE
    */
    KNOB_BASE(const std::string& myprefix, const std::string& myname, const std::string& myfamily, const std::string& mydefault,
              const std::string& mypurpose, KNOB_MODE mymode = KNOB_MODE_WRITEONCE);

    // virtual destructor required due to virtual functions present
    virtual ~KNOB_BASE() {}

    //
    // accessors
    //
    virtual const std::string Type() = 0; // subclass needs to implement this

    KNOB_MODE Mode() const { return _mode; }

    const std::string& Family() const { return _family; }

    const std::string& Name() const { return _name; }

    std::string Cmd() const { return ("-" + _name); }

    BOOL Enabled() const { return !_disabled; }

    /*! @ingroup KNOBS
      Check for duplicates amongst the declared knobs
    */
    STATIC VOID CheckAllKnobs(BOOL allowDashes = FALSE);

    /*! @ingroup KNOBS
      @return The total number of knobs that have been declared 
    */
    STATIC UINT32 NumberOfKnobs();

    /*! @ingroup KNOBS
      Disable all options within a given family of knobs
      @param myfamily The family to disable
    */
    STATIC VOID DisableKnobFamily(const std::string& myfamily);

    /*! @ingroup KNOBS
      Enable all options within a given family of knobs
      @param myfamily The family to enable
    */
    STATIC VOID EnableKnobFamily(const std::string& myfamily);

    // Mark that developer flags are used (xyzzy)
    STATIC VOID SetDeveloperFlagsEnabled();

    // Return true if developer flags were used (xyzzy)
    STATIC BOOL WasDeveloperFlagsEnabled();

    /*! @ingroup KNOBS
      Disable a particular knob
      @param myname The particular knob to disable
    */
    STATIC VOID DisableKnob(const std::string& myname);

    /*! @ingroup KNOBS
      Enable a particular knob
      @param myname The knob to enable
    */
    STATIC VOID EnableKnob(const std::string& myname);

    /*! @ingroup KNOBS
      Print out a summary of all the knobs declared
    */
    STATIC std::string StringKnobSummary();

    /*! @ingroup KNOBS
      x
    */
    STATIC std::string StringLongAll();

    /*! @ingroup KNOBS
      Locate a knob from the list of declared knobs
      @param myname The name of the knob to locate
    */
    STATIC KNOB_BASE* FindKnob(const std::string& name);

    /*! @ingroup KNOBS
      Search for a knob family
      @param family The family of knobs to locate
    */
    STATIC KNOB_BASE* FindFamily(const std::string& name);

    /*! @ingroup KNOBS
      Search for a knob that is currently not disabled
      @param myname The name of the knob to search which is currently enabled
    */
    STATIC KNOB_BASE* FindEnabledKnob(const std::string& name);

    // Calling this method indicates that the command line parsing is done.
    // It is guaranteed that the values of all knobs are consisntent once
    // the command line parsing is done.
    STATIC VOID SetParsingDone();

    // Return TRUE when the command line parsing is done.
    STATIC BOOL WasParsingDone();

    /*! @ingroup KNOBS
      @return TRUE if two knobs are identical.
    */
    int Compare(const KNOB_BASE& k2) const;

    // get string representation of value number index
    virtual const std::string& ValueString(const UINT32 index) const = 0;

    // get number of values
    virtual UINT32 NumberOfValues() const = 0;

    /*!
     * The function operate differently based on the knob's mode:
     *
     * KNOB_MODE_WRITEONCE - Set knob's value with the specified value. If the knob was already given a value, exit with PIN_ERROR.
     * KNOB_MODE_OVERWRITE - Overwrite knob's value with the specified value unless knob was initialized from command line
     * (knobs which were initialized with a value in the command line by the user are not permitted to be changed).
     * KNOB_MODE_ACCUMULATE - Add the value to the knob's former value. If the knob wasn't given a value before,
     * make it the knob's value.
     * KNOB_MODE_APPEND - Concatenate the value to the knob's value list.
     *
     * @return TRUE if the value of the knob got changed, False otherwise.
     */
    virtual BOOL AddValue(const std::string& valstring) = 0;

    /*! @ingroup KNOBS
     * Mark that this knob was set from command line.
     */
    VOID TurnOnSetByUser();

    /*! @ingroup KNOBS
     *  @return TRUE if the user set the knob from command line, FALSE otherwise.
     */
    BOOL SetByUser() const;
};

/*! @ingroup KNOBS
  typed knob
*/
template< class TYPE > class KNOB : public KNOB_BASE
{
  private:
    typedef KNOBVALUE_LIST< TYPE > valuelist;
    valuelist _value_list;

    // Disable compiler generated default c'tor
    KNOB() { PIN_ERROR("KNOB should not be constructed using default constructor"); }
    // Disable compiler generated copy c'tor
    KNOB(const KNOB&) { PIN_ERROR("KNOB should not be copied"); }
    // Disable compiler generated assignment operator
    KNOB& operator=(const KNOB&) { PIN_ERROR("KNOB should not be copied"); }

  public:
    KNOB(KNOB_MODE mymode, const std::string& myfamily, const std::string& myname, const std::string& mydefault,
         const std::string& mypurpose, const std::string& myprefix = "")
        : KNOB_BASE(myprefix, myname, myfamily, mydefault, mypurpose, mymode),
          _value_list(mydefault, (mymode != KNOB_MODE_APPEND))
    {}

    /*!
     * The function operate differently based on the knob's mode:
     *
     * KNOB_MODE_WRITEONCE - Set knob's value with the specified value. If the knob was already given a value, exit with PIN_ERROR.
     *  unless knob was initialized from command line (knobs which are....)
     * KNOB_MODE_OVERWRITE - Overwrite knob's value with the specified value unless knob was initialized from command line
     * (knobs which were initialized with a value in the command line by the user are not permitted to be changed).
     * KNOB_MODE_ACCUMULATE - Add the value to the knob's former value. If the knob wasn't given a value before,
     * make it the knob's value.
     * KNOB_MODE_APPEND - Concatenate the value to the knob's value list.
     *
     * @return TRUE if the value of the knob got changed, False otherwise.
     */
    BOOL AddValue(const std::string& valstring)
    {
        switch (_mode)
        {
            case KNOB_MODE_WRITEONCE:
                // Ignore exact-duplicate values
                if (!(_nwrites == 0 || _value_list.ValueString() == valstring))
                {
                    PIN_ERROR("trying to overwrite write-once knob '" + _name + "'\n");
                }

                // intentional fall-through
            case KNOB_MODE_OVERWRITE:
                if (SetByUser())
                {
                    return FALSE;
                }
                _value_list.Overwrite(valstring);
                break;

            case KNOB_MODE_ACCUMULATE:
                _value_list.Accumulate(valstring);
                break;

            case KNOB_MODE_APPEND:
                _value_list.Append(valstring);
                break;

            default:
                ASSERT(false, "unknown mode " + decstr(static_cast< UINT32 >(_mode)) + " for knob '" + _name + "'");
        }

        _nwrites++;

        return TRUE;
    }

    //
    // accessors
    //
    // implementation of virtual Type string accessor
    const std::string Type() { return KNOBVALUE< TYPE >::Type(); }
    // fast accessor for first element
    const TYPE& Value() const
    { /* too slow _nreads++; */
        return _value_list.Value();
    }
    const std::string& ValueString() const
    { /* too slow _nreads++; */
        return _value_list.ValueString();
    }
    // implicit conversion from KNOB<TYPE> to (rvalue) const TYPE
    operator TYPE() const { return Value(); }
    // accessor for any elements in value list
    const TYPE& Value(const UINT32 index)
    {
        _nreads++;
        return _value_list.Value(index);
    }
    const std::string& ValueString(const UINT32 index) const { return _value_list.ValueString(index); }
    UINT32 NumberOfValues() const { return _value_list.NumberOfValues(); }
};

/*! @ingroup KNOBS
  Family/comment knob
*/
class KNOB_COMMENT : public KNOB< BOOL >
{
  public:
    KNOB_COMMENT(const std::string& family, const std::string& purpose) : KNOB< BOOL >(KNOB_MODE_COMMENT, family, "", "", purpose)
    {}
};

/*! @ingroup KNOBS
  Minimal type for representing address ranges to be used as a value of a knob
*/
class ADDRESS_RANGE
{
  public:
    ADDRESS_RANGE(ADDRINT low, ADDRINT high)
    {
        _low  = low;
        _high = high;
    }

    ADDRESS_RANGE()
    {
        _low  = 0;
        _high = 0;
    }

    ADDRINT _low;
    ADDRINT _high;

    ADDRESS_RANGE& operator+=(const ADDRESS_RANGE& right) { return *this; }

    std::string String() const { return hexstr(_low) + ":" + hexstr(_high); }

    BOOL Valid() const { return (_high - _low) == 0 ? FALSE : TRUE; }

    ADDRINT Size() const { return _high - _low; }
};

/*! @ingroup KNOBS
  Knob for deciding whether using expensive sanity checks.
*/
extern KNOB< BOOL > KnobSlowAsserts;

#endif // PIN_G_KNOB_PH
