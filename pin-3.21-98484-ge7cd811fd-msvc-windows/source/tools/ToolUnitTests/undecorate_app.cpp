/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* This application does nothing
 * It is used only for checking symbol demangling feature
 */
#include <vector>
#include <string>
using std::string;
using std::vector;

class A
{
  public:
    void MyMethod1(const vector< int >& param1) {}
    void MyMethod2(string param2) {}
    void MyMethod3(string param3) const {}
};

template< class T > T MyMethod(T& arg) { return arg; }

template< class B > class MyMethod1 : public B
{
  public:
    MyMethod1() {}
};

void Foo()
{
    int i;
    volatile int* ip = (volatile int*)&i;

    *ip = 0;
}

// If you add new names, then update undecorate.cpp as well, so that it knows about them...
int main()
{
    A a;
    MyMethod1< A > goo;
    vector< int > v;
    v.push_back(1);

    a.MyMethod1(v);
    a.MyMethod2("MyString");
    a.MyMethod3("Foo");
    MyMethod(a);

    Foo();

    return 0;
}
