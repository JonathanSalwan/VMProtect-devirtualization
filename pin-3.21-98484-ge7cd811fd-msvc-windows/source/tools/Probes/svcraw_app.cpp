/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <rpc/rpc.h>

int main()
{
    SVCXPRT* rpcServiceTransport = svcraw_create();
    svc_destroy(rpcServiceTransport);
    return 0;
}
