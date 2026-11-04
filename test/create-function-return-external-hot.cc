#include "../include/jstl.h"
#include <assert.h>
#include <js.h>
#include <uv.h>

static int value = 42;

js_function_statistics_t make_stats;
js_function_statistics_t read_stats;

js_external_t<int>
on_make(js_env_t *env, js_receiver_t) {
  int e;

  js_external_t<int> result;
  e = js_create_external(env, &value, result);
  assert(e == 0);

  return result;
}

int32_t
on_read(js_env_t *, js_receiver_t, int *data) {
  return *data;
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

  js_handle_t make;
  e = js_create_function<on_make, &make_stats>(env, make);
  assert(e == 0);

  e = js_set_named_property(env, global, "make", static_cast<js_value_t *>(make));
  assert(e == 0);

  js_handle_t read;
  e = js_create_function<on_read, &read_stats>(env, read);
  assert(e == 0);

  e = js_set_named_property(env, global, "read", static_cast<js_value_t *>(read));
  assert(e == 0);

  // Hot enough for V8 to optimize hot(). make returns a js_external_t, which
  // has to stay on the normal path; read takes a plain pointer and is
  // fast-called.
  assert(run(env, "function hot() { return read(make()) } let ok = true; for (let i = 0; i < 1000000; i++) { if (hot() !== 42) ok = false } ok"));

  assert(make_stats.calls(js_function_call_t::typed) == 0);
  assert(read_stats.calls(js_function_call_t::typed) > 0);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
