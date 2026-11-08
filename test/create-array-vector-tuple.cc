#include <assert.h>
#include <js.h>
#include <stdint.h>
#include <tuple>
#include <uv.h>
#include <vector>

#include "../include/jstl.h"

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

  std::vector<std::tuple<int32_t, int32_t>> pairs = {{1, 2}, {3, 4}};

  js_array_t array;
  e = js_create_array(env, pairs, array);
  assert(e == 0);

  std::vector<std::tuple<int32_t, int32_t>> result;
  e = js_get_array_elements(env, array, result);
  assert(e == 0);

  assert(result == pairs);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
