/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
using std::cout;
using std::endl;
extern "C"
{
    void marker_start_counting() {}
    void marker_stop_counting() {}

    void marker_emit_stats() {}

    void marker_zero_stats() {}

} // end of extern "C"

int main(int argc, char** argv)
{
    for (int i = 0; i < 3; i++)
    {
        marker_zero_stats();
        marker_start_counting();
        cout << "Hello" << endl;
        marker_stop_counting();
        marker_emit_stats();
    }
    marker_zero_stats();
    return 0;
}
