/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test passing arguments in probes.
//

extern Bar(int, int, int, int);
extern Baz(int);

void Foo(int a, int b, int c, int d) { Bar(a, b, c, d); }

int main()
{
    Foo(12, 345, 678, 90);
    Foo(11, 22, 33, 44);
    Foo(99, 88, 77, 66);
    Baz(666);

    return 0;
}
