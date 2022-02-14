#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "VMProtectSDK.h"

unsigned secret(unsigned x, unsigned y);

int main(int ac, const char*av[]) {
  if (ac != 3)
    return 0;

  int x = atoi(av[1]);
  int y = atoi(av[2]);
  int r = secret(x, y);

  printf("> %d\n", r);

  return 0;
}

unsigned secret(unsigned x, unsigned y) {
  VMProtectBegin("secret");
  unsigned r = 0;
  if (x + y < 1234)
    r = x + 1;
  else
    r = y - 1;
  VMProtectEnd();
  return r;
}
