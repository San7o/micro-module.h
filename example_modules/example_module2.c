// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#include <stdio.h>

void hello_message(void)
{
  printf("Hello from module 2\n");
  return;
}

void bye_message(void)
{
  printf("Bye from module 2\n");
  return;
}

// Functions required by micro_module

const char micro_module_name[] = "example_module2";

extern int micro_module_init(void* arg)
{
  hello_message();
  return 0;
}

extern int micro_module_exit(void* arg)
{
  bye_message();
  return 0;
}
