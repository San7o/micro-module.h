// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#define MICRO_MODULE_IMPLEMENTATION
#include "micro-module.h"

#include <assert.h>

int main(void)
{
  MicroModule mm =
    micro_module_setup("micro_module_name",  // name symbol
                       "micro_module_init",  // init func symbol
                       "micro_module_exit",  // exit func symbol
                       true);  // create a new symbol namespace

  // Load all modules from the modules directory
  assert(micro_module_init_all(&mm, "./example_modules/compiled", NULL)
         == MICRO_MODULE_OK);

  // Reload a module
  assert(micro_module_init(&mm, "./example_modules/compiled/example_module1.so", NULL)
         == MICRO_MODULE_OK);
  
  // Unload a specific module
  assert(micro_module_exit(&mm, "example_module2", NULL)
         == MICRO_MODULE_OK);

  // Unload all modules
  assert(micro_module_exit_all(&mm, NULL) == MICRO_MODULE_OK);  
  return 0;
}
