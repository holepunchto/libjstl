#include <assert.h>
#include <js.h>
#include <uv.h>

#include "../include/jstl.h"

void
on_teardown() {
  assert(false);
}

int
main() {
  int e;

  uv_loop_t *loop = uv_default_loop();

  js_platform_t *platform;
  e = js_create_platform(loop, NULL, &platform);
  assert(e == 0);

  js_env_t *env;
  e = js_create_env(loop, platform, NULL, &env);
  assert(e == 0);

  e = js_add_teardown_callback<on_teardown>(env);
  assert(e == 0);

  e = js_remove_teardown_callback<on_teardown>(env);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
