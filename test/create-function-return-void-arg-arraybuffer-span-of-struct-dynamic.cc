#include <assert.h>
#include <js.h>
#include <stdint.h>
#include <uv.h>

#include "../include/jstl.h"

struct data {
  int32_t foo;
  bool bar;
};

void
on_call(js_env_t *env, js_arraybuffer_span_of_t<struct data> data) {
  assert(data.size() == 3);

  assert(data[0].foo == 1);
  assert(data[0].bar == true);
  assert(data[1].foo == 2);
  assert(data[1].bar == true);
  assert(data[2].foo == 3);
  assert(data[2].bar == true);
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

  js_function_t<void, js_arraybuffer_span_of_t<struct data>> fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  struct data data[3];
  data[0].foo = 1;
  data[0].bar = true;
  data[1].foo = 2;
  data[1].bar = true;
  data[2].foo = 3;
  data[2].bar = true;

  e = js_call_function(env, fn, js_arraybuffer_span_of_t(data, 3));
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
