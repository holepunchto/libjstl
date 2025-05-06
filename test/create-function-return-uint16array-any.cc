#include <assert.h>
#include <js.h>
#include <stdint.h>
#include <uv.h>

#include "../include/jstl.h"

js_typedarray_t<>
on_call(js_env_t *env) {
  int e;

  uint16_t *data;

  js_typedarray_t<> typedarray;
  e = js_create_typedarray(env, 5, data, typedarray);
  assert(e == 0);

  data[0] = 'h';
  data[1] = 'e';
  data[2] = 'l';
  data[3] = 'l';
  data[4] = 'o';

  return typedarray;
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

  js_function_t<js_typedarray_t<>> fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  js_typedarray_t<> result;
  e = js_call_function(env, fn, result);
  assert(e == 0);

  uint16_t *data;
  size_t len;
  e = js_get_typedarray_info(env, result, data, len);
  assert(e == 0);

  assert(len == 5);

  assert(data[0] == 'h');
  assert(data[1] == 'e');
  assert(data[2] == 'l');
  assert(data[3] == 'l');
  assert(data[4] == 'o');

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
