#include "../include/jstl.h"
#include <assert.h>
#include <js.h>
#include <uv.h>
#include <vector>

std::vector<int32_t>
on_call(js_env_t *env) {
  int e;
  e = js_throw_error(env, NULL, "pending");
  assert(e == 0);

  return {};
}

static bool
run(js_env_t *env, const char *script) {
  int e;

  js_value_t *source;
  e = js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(script), size_t(-1), &source);
  assert(e == 0);

  js_value_t *result;
  e = js_run_script(env, NULL, 0, 0, source, &result);
  assert(e == 0);

  bool value;
  e = js_get_value_bool(env, result, &value);
  assert(e == 0);

  return value;
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

  js_value_t *global;
  e = js_get_global(env, &global);
  assert(e == 0);

  js_handle_t call;
  e = js_create_function<on_call>(env, call);
  assert(e == 0);

  e = js_set_named_property(env, global, "fn", static_cast<js_value_t *>(call));
  assert(e == 0);

  assert(run(env, "try { fn(); false } catch (e) { e.message === 'pending' }"));

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
