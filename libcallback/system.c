#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

static int (*original_system)(const char *command);

static void __attribute ((constructor)) system_hook_init(void) {

  original_system = dlsym(dlopen ("/lib/libc.so.0", RTLD_LAZY), "system");
}

int system(const char *command) {

  fprintf(stderr, "======> system %s %d\n", command, getpid());
  const char *path = getenv("PATH");
  const char *lib_path = getenv("LD_LIBRARY_PATH");
  fprintf(stderr, "PATH: %s\n", path);
  fprintf(stderr, "LD_LIBRARY_PATH: %s\n", lib_path);

  int ret = original_system(command);

  fprintf(stderr, "<======= %d\n", ret);
}


