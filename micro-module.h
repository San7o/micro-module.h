///////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// micro-module.h
// ==============
//
// A lightweight header-only library for defining, loading, and
// unloading runtime modules / plugins with a simple C99 API.
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o
//
//
// Quickstart
// ----------
//
// A module is just a normal C shared library where at least three
//  symbols are exported: the module name, an init function, and
// an exit function. The actual symbols are specified by the loader
// and their types are specified by the library.
//
// For this example let's use `micro_module_name`,`micro_module_init`
// and `micro_module_exit`:
//
//   // The name of this module, used as an identifier
//   const char micro_module_name[] = "example_module1";
//
//   // Function called when module gets loaded
//   extern int micro_module_init(void* arg)
//   {
//     // ....
//     return 0;
//   }
//
//   // Function called when module gets unloaded
//   extern int micro_module_exit(void* arg)
//   {
//     // ....
//     return 0;
//   }
//
// Compile the module with -fPIC and -shared.
//
// In your loader, you can load and unload a module with
// `micro_module_init` and `micro_module_exit`. Alternatively, you can
// use the `_all` variants of these functions to load all the modules
// from a directory, or unload all loaded modules.
//
// Do this:
//
//   #define MICRO_MODULE_IMPLEMENTATION
//
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
//
//   #include ...
//   #include ...
//   #include ...
//   #define MICRO_MODULE_IMPLEMENTATION
//   #include "micro-module.h"
//
// Here is an example loader:
//
//   #define MICRO_MODULE_IMPLEMENTATION
//   #include "micro-module.h"
//
//   int main(void)
//   {
//     MicroModule mm =
//       micro_module_setup("micro_module_name",  // symbol for name
//                          "micro_module_init",  // symbol for the init func
//                          "micro_module_exit",  // symbol for the exit func
//                          true);  // create a new symbol namespace
//
//     // Load all modules from the modules directory
//     micro_module_init_all(&mm, "./example_modules/compiled", NULL);
//
//     // Unload a specific module
//     micro_module_exit(&mm, "example_module1", NULL);
//
//     // Unload all modules
//     micro_module_exit_all(&mm, NULL);
//     return 0;
//   }
//
// You can tune the library by #defining certain values. See the
// "Config" comments under "Configuration" below.
//
//
// Code
// ----
//
// The official git repository of micro-module.h is hosted at:
//
//     https://github.com/San7o/micro-module.h
//
// This is part of a bigger collection of header-only C99 libraries
// called "micro-headers", contributions are welcome:
//
//     https://github.com/San7o/micro-headers
//

#ifndef MICRO_MODULE
#define MICRO_MODULE

#define MICRO_MODULE_MAJOR 0
#define MICRO_MODULE_MINOR 1

// Needed for dlmopen
#define _GNU_SOURCE

#ifdef __cplusplus
extern "C" {
#endif

//
// Configuration
//

// Config: Prefix for all functions
// For function inlining, set this to `static inline` and then define
// the implementation in all the files
#ifndef MICRO_MODULE_DEF
  #define MICRO_MODULE_DEF extern
#endif

// Config: Define memory allocation function
// Notes: This is expected to be used like malloc(3)
#ifndef MICRO_MODULE_MALLOC
  #include <stdlib.h>
  #define MICRO_MODULE_MALLOC malloc
#endif

// Config: Define memory free function
// Notes: This is expected to be used like free(3)
#ifndef MICRO_MODULE_FREE
  #include <stdlib.h>
  #define MICRO_MODULE_FREE free
#endif
  
//
// Macros
//

//
// Errors
//

#define MICRO_MODULE_OK                           0
#define MICRO_MODULE_ERROR_IS_NULL               -1
#define MICRO_MODULE_ERROR_OPEN_MODULES_DIR      -2
#define MICRO_MODULE_ERROR_CLOSE_MODULES_DIR     -3
#define MICRO_MODULE_ERROR_LOCATING_INIT_SYMBOL  -4
#define MICRO_MODULE_ERROR_LOCATING_EXIT_SYMBOL  -5
#define MICRO_MODULE_ERROR_LOCATING_NAME_SYMBOL  -6
#define MICRO_MODULE_ERROR_OPENING_MODULE        -7
#define MICRO_MODULE_ERROR_CLOSING_MODULE        -8
#define MICRO_MODULE_ERROR_ALLOCATING_MEMORY     -9
#define MICRO_MODULE_ERROR_MODULE_NOT_REGISTERED -10
#define MICRO_MODULE_ERROR_ARG_NULL              -11
#define _MICRO_MODULE_ERROR_MAX                  -12

//
// Types
//

#include <stdbool.h>
  
// Init and exit functions
typedef int(*micro_module_init_fn)(void*);
typedef int(*micro_module_exit_fn)(void*);

// Struct representing a single module
typedef struct {
  // Module name, used as an identifier
  char *name;
  // Init function pointer
  micro_module_init_fn init_fn;
  // Exit function pointer
  micro_module_exit_fn exit_fn;
  // The opaque handler returned from dlopen / dlmopen
  void* dlhandler;
} MicroModuleEntry;

// Linked list of modules, where the head is the last loaded module
struct MicroModuleList;
typedef struct MicroModuleList MicroModuleList;
struct MicroModuleList {
  MicroModuleList* next;
  MicroModuleEntry module;
};

// Central struct of this library
typedef struct {
  // Linked list of modules
  MicroModuleList *modules;
  // Symbol for a module name
  const char* name_symbol;
  // Symbol for the init function
  const char* init_fn_symbol;
  // Symbol for the exit function
  const char* exit_fn_symbol;
  // Wether to create a new namespace of not
  // If a new namespace is created, the module will not be able
  // to access symbols from the loader.
  bool use_new_namespace;
} MicroModule;

//
// Function declarations
//

// You can use this to construct the MicroModule
// Basically assigns the fiels with the specified arguments.
MICRO_MODULE_DEF MicroModule
micro_module_setup(const char* name_symbol,
                   const char* init_fn_symbol,
                   const char* exit_fn_symbol,
                   bool use_new_namespace);

// Load and initialize module located in [filename], passing [arg]
//
// If the module was already loaded, it first unloads it and then loads
// it again.
// Returns MICRO_MODULE_OK on success, or a negative MICRO_MODULE_ERROR_
MICRO_MODULE_DEF int
micro_module_init(MicroModule *mm, char* filename, void* arg);

// Load and initialize all modules from [modules_dir], passing [arg]
//
// If a module was already loaded, it first unloads it and then loads
// it again.
// Returns MICRO_MODULE_OK on success, or a negative MICRO_MODULE_ERROR_
MICRO_MODULE_DEF int
micro_module_init_all(MicroModule *mm, char* modules_dir, void* arg);

// Unloads module identified by [module_name]
// Returns MICRO_MODULE_OK on success, or a negative MICRO_MODULE_ERROR_
MICRO_MODULE_DEF int
micro_module_exit(MicroModule *mm,
                  const char *module_name,
                  void* arg);

// Unloads all loaded modules
// Returns MICRO_MODULE_OK on success, or a negative MICRO_MODULE_ERROR_
MICRO_MODULE_DEF int micro_module_exit_all(MicroModule *mm, void* arg);
  
//
// Implementation
//
  
#ifdef MICRO_MODULE_IMPLEMENTATION

#include <fts.h>
#include <dlfcn.h>
#include <string.h>

MICRO_MODULE_DEF MicroModule
micro_module_setup(const char* name_symbol,
                   const char* init_fn_symbol,
                   const char* exit_fn_symbol,
                   bool use_new_namespace)
{
  return (MicroModule) {
    .name_symbol       = name_symbol,
    .init_fn_symbol    = init_fn_symbol,
    .exit_fn_symbol    = exit_fn_symbol,
    .use_new_namespace = use_new_namespace,
    .modules           = NULL,
  };
}

MICRO_MODULE_DEF int
micro_module_init(MicroModule *mm, char* filename, void* arg)
{
  if (!mm) return MICRO_MODULE_ERROR_IS_NULL;

  MicroModuleEntry module;

  if (mm->use_new_namespace)
  {
    module.dlhandler = dlmopen(LM_ID_NEWLM, filename, RTLD_LAZY | RTLD_LOCAL);
  }
  else
  {
    module.dlhandler = dlmopen(LM_ID_BASE, filename, RTLD_LAZY | RTLD_LOCAL);
  }
  
  if (!module.dlhandler) return MICRO_MODULE_ERROR_OPENING_MODULE;

  *(void**)(&module.init_fn) = dlsym(module.dlhandler, mm->init_fn_symbol);
  if (!module.init_fn)
  {
    dlclose(module.dlhandler);
    return MICRO_MODULE_ERROR_LOCATING_INIT_SYMBOL;
  }
  
  *(void**)(&module.exit_fn) = dlsym(module.dlhandler, mm->exit_fn_symbol);
  if (!module.exit_fn)
  {
    dlclose(module.dlhandler);
    return MICRO_MODULE_ERROR_LOCATING_EXIT_SYMBOL;
  }

  module.name = dlsym(module.dlhandler, mm->name_symbol);
  if (!module.name)
  {
    dlclose(module.dlhandler);
    return MICRO_MODULE_ERROR_LOCATING_NAME_SYMBOL;
  }

  // Check if module was already registered
  bool found = false;
  MicroModuleList *it = mm->modules;
  while (it)
  {
    if (strcmp(it->module.name, module.name) == 0)
    {
      found = true;

      // Exit the loaded module
      it->module.exit_fn(arg);
      if (dlclose(it->module.dlhandler) != 0)
      {
        dlclose(module.dlhandler);
        return MICRO_MODULE_ERROR_CLOSING_MODULE;
      }

      it->module = module;
      break;
    }
    it = it->next;
  }

  if (!found)
  {
    // Add to the module list
    MicroModuleList *new_module = MICRO_MODULE_MALLOC(sizeof(MicroModuleList));
    if (!new_module) return MICRO_MODULE_ERROR_ALLOCATING_MEMORY;
    new_module->module = module;
    if (!mm->modules)
    {  
      new_module->next = NULL;
      mm->modules = new_module;
    }
    else
    {
      new_module->next = mm->modules;
      mm->modules = new_module;
    }
  }
  
  // Call the function
  int err = 0;
  err = module.init_fn(arg);
  if (err != 0) return err;

  return MICRO_MODULE_OK;
}

MICRO_MODULE_DEF int
micro_module_init_all(MicroModule *mm, char *modules_dir, void* arg)
{
  if (!mm) return MICRO_MODULE_ERROR_IS_NULL;

  int err = MICRO_MODULE_OK;
  char *path_argv[] = { modules_dir, NULL };
  FTSENT *file_entry = NULL;
  FTS *files = fts_open(path_argv, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
  if (!files) return MICRO_MODULE_ERROR_OPEN_MODULES_DIR;

  while ((file_entry = fts_read(files)))
  {
    switch (file_entry->fts_info)
    {
    case FTS_D: // Directory
      if (file_entry->fts_level == 0)
      {
        // Top-level directory itself, continue
        continue;
      } else {
        // Skip descending into subdirectories
        fts_set(files, file_entry, FTS_SKIP);
      }
      break;
    case FTS_F:  // Regular file
    case FTS_SL: // Symbolic link
    case FTS_DEFAULT:
      if (file_entry->fts_level == 1)
      {
        err = micro_module_init(mm, file_entry->fts_path, arg);
        if (err != MICRO_MODULE_OK)
        {
          goto exit;
        }
      }
      break;
    default:
      break;
    }
  }

 exit:
  if (fts_close(files) < 0 && err == MICRO_MODULE_OK)
    return MICRO_MODULE_ERROR_CLOSE_MODULES_DIR;
  return err;
}

MICRO_MODULE_DEF int
micro_module_exit(MicroModule *mm,
                  const char* module_name,
                  void* arg)
{
  if (!mm) return MICRO_MODULE_ERROR_IS_NULL;
  if (!mm->modules) return MICRO_MODULE_ERROR_MODULE_NOT_REGISTERED;
  if (!module_name) return MICRO_MODULE_ERROR_ARG_NULL;
  
  if (strcmp(mm->modules->module.name, module_name) == 0)
  {
    MicroModuleList *module = mm->modules;
    mm->modules->module.exit_fn(arg);
      if (dlclose(mm->modules->module.dlhandler) != 0)
        return MICRO_MODULE_ERROR_CLOSING_MODULE;
      
    mm->modules = mm->modules->next;
    MICRO_MODULE_FREE(module);
    return MICRO_MODULE_OK;
  }
  
  MicroModuleList *prev = NULL;
  MicroModuleList *it = mm->modules;
  while(it)
  {
    if (strcmp(it->module.name, module_name) == 0)
    {
      it->module.exit_fn(arg);
      if (dlclose(it->module.dlhandler) != 0)
        return MICRO_MODULE_ERROR_CLOSING_MODULE;
      
      prev->next = it->next;
      MICRO_MODULE_FREE(it);
      return MICRO_MODULE_OK;
    }
    prev = it;
    it = it->next;
  }
  
  return MICRO_MODULE_ERROR_MODULE_NOT_REGISTERED;
}

MICRO_MODULE_DEF int micro_module_exit_all(MicroModule *mm, void* arg)
{
  if (!mm) return MICRO_MODULE_ERROR_IS_NULL;

  int err = MICRO_MODULE_OK;
  MicroModuleList *it = mm->modules;
  while (it)
  {
    MicroModuleList *next = it->next;
    
    err = micro_module_exit(mm, it->module.name, arg);
    if (err != MICRO_MODULE_OK)
      return err;
    
    it = next;
  }
  mm->modules = NULL;
  
  return MICRO_MODULE_OK;
}

#endif // MICRO_MODULE_IMPLEMENTATION

//
// Examples
//
  
#if 0

#define MICRO_MODULE_IMPLEMENTATION
#include "micro-module.h"

#include <assert.h>

int main(void)
{
  MicroModule mm =
    micro_module_setup("micro_module_name",
                       "micro_module_init",
                       "micro_module_exit",
                       true);

  // Load all modules from the modules directory
  assert(micro_module_init_all(&mm, "./example_modules/compiled", NULL)
         == MICRO_MODULE_OK);

  // Unload a specific module
  assert(micro_module_exit(&mm, "example_module1", NULL)
         == MICRO_MODULE_OK);

  // Unload all modules
  assert(micro_module_exit_all(&mm, NULL) == MICRO_MODULE_OK);
  return 0;
}

#endif // 0
  
#ifdef __cplusplus
}
#endif

#endif // MICRO_MODULE
