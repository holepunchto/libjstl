#include <assert.h>
#include <js.h>
#include <uv.h>

#include <jstl/std.h>

std::pair<std::string, int32_t>
on_call(js_env_t *, std::pair<int32_t, std::string> pair) {
  return {pair.second, pair.first};
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

  js_object_t global;
  e = js_get_global(env, global);
  assert(e == 0);

  e = js_set_property<on_call>(env, global, "swap");
  assert(e == 0);

  assert(run(env, "const r = swap([1, 'a']); Array.isArray(r) && r.length === 2 && r[0] === 'a' && r[1] === 1"));

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
