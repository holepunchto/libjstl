#include <assert.h>
#include <chrono>
#include <js.h>
#include <uv.h>

#include <jstl/std.h>

using std::chrono::milliseconds;
using std::chrono::system_clock;

system_clock::time_point
on_call(js_env_t *, system_clock::time_point time) {
  return time + milliseconds(1);
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

  e = js_set_property<on_call>(env, global, "next");
  assert(e == 0);

  assert(run(env, "const d = next(new Date(1700000000000)); d instanceof Date && d.getTime() === 1700000000001"));

  assert(run(env, "try { next(new Date(NaN)); false } catch (e) { e instanceof RangeError }"));

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
