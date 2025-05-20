#include <assert.h>
#include <js.h>
#include <stdbool.h>
#include <uv.h>

#include "../include/jstl.h"

void
on_call(js_env_t *env, int32_t n, bool b) {
  assert(n == true);
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

  js_handle_scope_t *scope;
  e = js_open_handle_scope(env, &scope);
  assert(e == 0);

  js_function_t<void, int32_t, bool> fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  e = js_call_function(env, fn, int32_t(42), true);
  assert(e == 0);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
