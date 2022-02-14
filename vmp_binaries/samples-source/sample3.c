#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "VMProtectSDK.h"

char secret(char x, char y);

int main(int ac, const char*av[]) {
  if (ac != 3)
    return 0;

  int x = atoi(av[1]);
  int y = atoi(av[2]);
  int r = secret(x & 0xff, y & 0xff);

  printf("> %d\n", r);

  return 0;
}

// This function is an MBA that computes: (x ^ 92) + y
// We will protect this MBA with VMProtect and see if we can recover "(x ^ 92) + y"
char secret(char x, char y) {
  VMProtectBegin("secret");
  int a = 229 * x + 247;
  int b = 237 * a + 214 + ((38 * a + 85) & 254);
  int c = (b + ((-(2 * b) + 255) & 254)) * 3 + 77;
  int d = ((86 * c + 36) & 70) * 75 + 231 * c + 118;
  int e = ((58 * d + 175) & 244) + 99 * d + 46;
  int f = (e & 148);
  int g = (f - (e & 255) + f) * 103 + 13;
  int r = (237 * (45 * g + (174 * g | 34) * 229 + 194 - 247) & 255) + y;
  VMProtectEnd();
  return r;
}
