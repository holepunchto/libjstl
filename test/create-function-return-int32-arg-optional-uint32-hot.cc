#include "../include/jstl.h"
#include <assert.h>
#include <js.h>
#include <optional>
#include <uv.h>

js_function_statistics_t stats;

int32_t
on_call(js_env_t *, js_receiver_t, std::optional<uint32_t> value) {
  return value ? int32_t(*value) : -1;
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
  e = js_create_function<on_call, &stats>(env, call);
  assert(e == 0);

  e = js_set_named_property(env, global, "fn", static_cast<js_value_t *>(call));
  assert(e == 0);

  // Hot enough for V8 to optimize hot() and fast-call fn.
  assert(run(env, "function hot(v) { return fn(v) } let ok = true; for (let i = 0; i < 1000000; i++) { const v = i % 100 ? i : undefined; if (hot(v) !== (v === undefined ? -1 : i)) ok = false } ok"));

  assert(stats.calls(js_function_call_t::typed) > 0);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
