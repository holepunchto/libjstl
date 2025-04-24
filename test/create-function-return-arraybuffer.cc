#include <assert.h>
#include <js.h>
#include <uv.h>

#include "../include/jstl.h"

js_arraybuffer_t
on_call(js_env_t *env) {
  int e;

  js_arraybuffer_t arraybuffer;

  std::span<uint8_t> data;
  e = js_create_arraybuffer(env, 5, data, arraybuffer);
  assert(e == 0);

  data[0] = 'h';
  data[1] = 'e';
  data[2] = 'l';
  data[3] = 'l';
  data[4] = 'o';

  return arraybuffer;
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

  js_function_t<js_arraybuffer_t> fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  js_arraybuffer_t result;
  e = js_call_function(env, fn, result);
  assert(e == 0);

  std::span<uint8_t> data;
  e = js_get_arraybuffer_info(env, result, data);
  assert(e == 0);

  assert(data.size() == 5);

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
