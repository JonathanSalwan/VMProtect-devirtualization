/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sstream>
#include <cstdlib>
using std::ostringstream;
#if defined(TARGET_LINUX)
#include <unistd.h>
#include <sys/types.h>
#elif defined(TARGET_MAC)
#include <unistd.h>
#endif //TARGET_LINUX

/*
 * This application prints its memory map to stdout
 */

int main()
{
    ostringstream os;

#if defined(TARGET_LINUX)
    os << "/bin/cat /proc/" << getpid() << "/maps";
#elif defined(TARGET_MAC)
    os << "/usr/bin/vmmap " << getpid();
#endif
    system(os.str().c_str());

    return 0;
}
