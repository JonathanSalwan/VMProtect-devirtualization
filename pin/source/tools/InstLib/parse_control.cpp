/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "parse_control.H"
#include "control_chain.H"
#include "util/strings.hpp"

using namespace std;

namespace CONTROLLER
{
unsigned int PARSER::SplitArgs(const string sep, const string& input, vector< string >& output_array)
{
    // returns the number of args
    // rip off the separator characters and split the src
    // string based on separators.

    // find the string between last_pos and pos. pos is after last_pos
    size_t last_pos = input.find_first_not_of(sep, 0);
    size_t pos      = input.find_first_of(sep, last_pos);

    int i = 0;
    while (pos != string::npos && last_pos != string::npos)
    {
        string a = input.substr(last_pos, pos - last_pos);
        output_array.push_back(a);

        last_pos = input.find_first_not_of(sep, pos);
        pos      = input.find_first_of(sep, last_pos);
        i++;
    }
    if (last_pos != string::npos && pos == string::npos)
    {
        string a = input.substr(last_pos); // get the rest of the string
        output_array.push_back(a);
        i++;
    }
    return i;
}

BOOL PARSER::ConfigToken(const string& control_str)
{
    vector< string > tokens;
    SplitArgs(":", control_str, tokens);
    if (tokens[0] == "repeat" || tokens[0] == "name" || tokens[0] == "waitfor")
    {
        return TRUE;
    }
    return FALSE;
}

BOOL PARSER::UniformToken(vector< string >& tokens)
{
    if (tokens[0] == "uniform")
    {
        return TRUE;
    }
    return FALSE;
}
VOID PARSER::ParseConfigTokens(const string& control_str, CONTROL_CHAIN* chain)
{
    vector< string > tokens;
    SplitArgs(":", control_str, tokens);
    if (tokens[0] == "repeat")
    {
        if (tokens.size() == 1)
        {
            chain->SetRepeat(REPEAT_INDEFINITELY);
        }
        else
        {
            UINT32 repeat = StringToUint32(tokens[1]);
            chain->SetRepeat(repeat);
        }
    }
    else if (tokens[0] == "name")
    {
        chain->SetName(tokens[1]);
    }
    else if (tokens[0] == "waitfor")
    {
        chain->SetWaitFor(tokens[1]);
    }
    else
    {
        ASSERT(FALSE, "Unexpected config token");
    }
}

BOOL PARSER::ParseBcastToken(const string& str, BOOL* bcast)
{
    string bcast_str = "bcast";

    if (str.compare(0, bcast_str.length(), bcast_str) == 0)
    {
        *bcast = TRUE;
        return TRUE;
    }
    return FALSE;
}

UINT32 PARSER::GetTIDToken(const string& str)
{
    string tid_str = "tid";

    if (str.compare(0, tid_str.length(), tid_str) == 0)
    {
        string s = str.substr(tid_str.length());
        return StringToUint32(s);
    }
    ASSERT(FALSE, "failed to parse tid token");
    return 0;
}

BOOL PARSER::ParseTIDToken(const string& str, UINT32* tid)
{
    string tid_str = "tid";

    if (str.compare(0, tid_str.length(), tid_str) == 0)
    {
        string s = str.substr(tid_str.length());
        *tid     = StringToUint32(s);
        return TRUE;
    }
    return FALSE;
}

BOOL PARSER::ParseGlobalToken(const string& str, BOOL* global_count)
{
    string global_str = "global";

    if (str.compare(0, global_str.length(), global_str) == 0)
    {
        *global_count = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL PARSER::ParseRepeatToken(const string& str)
{
    string repeat_str = "repeat";

    if (str.compare(0, repeat_str.length(), repeat_str) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL PARSER::ParseCountToken(const string& str, UINT64* count)
{
    string count_str = "count";

    if (str.compare(0, count_str.length(), count_str) == 0)
    {
        string c = str.substr(count_str.length());
        ASSERT(!c.empty(), "count must have a numeric value");
        *count = StringToUint64(c);
        return TRUE;
    }
    return FALSE;
}

UINT32 PARSER::StringToUint32(const string& s)
{
    UINT32 val = 0;
    string::const_iterator it;
    it = UTIL::ParseUnsigned(s.begin(), s.end(), 0, &val);

    if (it != s.end()) ASSERT(FALSE, "failed converting string to int:" + s);

    return val;
}

UINT64 PARSER::StringToUint64(const string& s)
{
    UINT64 val = 0;
    string::const_iterator it;
    it = UTIL::ParseUnsigned(s.begin(), s.end(), 0, &val);

    if (it != s.end()) ASSERT(FALSE, "failed converting string to int:" + s);

    return val;
}

static UINT8 convert_nibble(UINT8 n)
{
    if (n >= '0' && n <= '9') return n - '0';
    if (n >= 'a' && n <= 'f') return n - 'a' + 10;
    if (n >= 'A' && n <= 'F') return n - 'A' + 10;
    cerr << "Bad nibble in hex string: " << (char)n << endl;
    ASSERTX(0);
    return 0;
}

VOID PARSER::str2hex(const char* in, unsigned char* out, size_t len)
{
    size_t i = 0, j = 0;
    for (i = 0; i < len; i += 2)
        out[j++] = convert_nibble(in[i]) * 16 + convert_nibble(in[i + 1]);
}

VOID PARSER::ParseOldConfigTokens(const string& str, string& value, string& count, string& tid, BOOL& repeat)
{
    //parsing old controller tokens: [:<int>][:tid<tid>][:repeat]
    vector< string > tokens;
    SplitArgs(":", str, tokens);
    value = tokens[0];
    if (tokens.size() > 1)
    {
        for (UINT32 i = 1; i < tokens.size(); i++)
        {
            if (tokens[i] == "repeat")
            {
                repeat = TRUE;
            }
            else if (tokens[i].substr(0, 3) == "tid")
            {
                tid = tokens[i];
            }
            else
            {
                //this must be a count token -> simple int
                count = tokens[i];
            }
        }
    }
}

}; // namespace CONTROLLER
