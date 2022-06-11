/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin doesn't crash when demangling a symbol name which
 * includes the "operator" keyword.
 * We expect Pin _not_ to identify "my_operator<int>" as the "operator<" keyword
 * and thus not to change the symbol name.
 */

/*
 * This is a template function, therefore the symbol name will include the string
 * "operator<". Pin should not corrupt this string.
 */
template< class T > int my_operator(const T& src) { return src; }

int main()
{
    int a = my_operator< int >(0);
    return a;
}
