#include <assert.h>
#include <js.h>
#include <uv.h>

#include "../include/jstl.h"

int
on_call(js_env_t *) {
  return 42;
}

void
on_threadsafe_call(js_env_t *env, js_function_t<int> fn, int *context, bool *data) {
  assert(*context == 42);
  assert(*data == true);
}

void
on_finalize(js_env_t *env, int *context) {
  assert(*context == 42);
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

  js_function_t<int> fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  int context = 42;

  js_threadsafe_function_t *tsfn;
  e = js_create_threadsafe_function<on_threadsafe_call, on_finalize, int, bool>(env, fn, 0, 1, &context, tsfn);
  assert(e == 0);

  bool data = true;
  e = js_call_threadsafe_function(tsfn, &data);
  assert(e == 0);

  e = js_release_threadsafe_function(tsfn);
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
