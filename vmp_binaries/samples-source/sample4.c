#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "VMProtectSDK.h"

int secret(int x, int y);

int main(int ac, const char*av[]) {
  if (ac != 3)
    return 0;

  int x = atoi(av[1]);
  int y = atoi(av[2]);
  int r = secret(x, y);

  printf("> %d\n", r);

  return 0;
}

int secret(int x, int y) {
  VMProtectBegin("secret");
  int r = 0;
  if (x >= y)
    r = x;
  else
    r = y;
  VMProtectEnd();
  return r;
}
