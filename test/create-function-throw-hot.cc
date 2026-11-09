#include <assert.h>
#include <js.h>
#include <stdexcept>
#include <string>
#include <uv.h>

#include "../include/jstl.h"

js_function_statistics_t stats_void;
js_function_statistics_t stats_result;
js_function_statistics_t stats_void_no_env;
js_function_statistics_t stats_result_no_env;
js_function_statistics_t stats_void_unscoped;
js_function_statistics_t stats_result_unscoped;
js_function_statistics_t stats_noexcept;

void
on_call_void(js_env_t *, js_receiver_t, int32_t value) {
  if (value) throw std::runtime_error("void");
}

int32_t
on_call_result(js_env_t *, js_receiver_t, int32_t value) {
  if (value) throw std::runtime_error("result");

  return value;
}

void
on_call_void_no_env(js_receiver_t, int32_t value) {
  if (value) throw std::runtime_error("void no env");
}

int32_t
on_call_result_no_env(js_receiver_t, int32_t value) {
  if (value) throw std::runtime_error("result no env");

  return value;
}

void
on_call_noexcept(js_env_t *, js_receiver_t, std::string) noexcept {}

static bool
run(js_env_t *env, const std::string &script) {
  int e;

  js_value_t *source;
  e = js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(script.c_str()), script.size(), &source);
  assert(e == 0);

  js_value_t *result;
  e = js_run_script(env, NULL, 0, 0, source, &result);
  assert(e == 0);

  bool value;
  e = js_get_value_bool(env, result, &value);
  assert(e == 0);

  return value;
}

// Every call throws, so each typed call has to raise an error in JS.
template <auto fn, js_function_options_t options>
static void
check(js_env_t *env, const std::string &name, const std::string &arg, const std::string &expected) {
  int e;

  js_handle_t function;
  e = js_create_function<fn, options>(env, function);
  assert(e == 0);

  js_value_t *global;
  e = js_get_global(env, &global);
  assert(e == 0);

  e = js_set_named_property(env, global, name.c_str(), static_cast<js_value_t *>(function));
  assert(e == 0);

  assert(run(env, "(() => { let thrown = 0; for (let i = 0; i < 20000; i++) { try { " + name + "(" + arg + ") } catch (e) { if (" + expected + ") thrown++ } } return thrown === 20000 })()"));

  assert(options.statistics->calls(js_function_call_t::typed) > 0);
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

  check<on_call_void, &stats_void>(env, "fnVoid", "1", "e.message === 'void'");
  check<on_call_result, &stats_result>(env, "fnResult", "1", "e.message === 'result'");
  check<on_call_void_no_env, &stats_void_no_env>(env, "fnVoidNoEnv", "1", "e.message === 'void no env'");
  check<on_call_result_no_env, &stats_result_no_env>(env, "fnResultNoEnv", "1", "e.message === 'result no env'");
  check<on_call_void, js_function_options_t(false, &stats_void_unscoped)>(env, "fnVoidUnscoped", "1", "e.message === 'void'");
  check<on_call_result, js_function_options_t(false, &stats_result_unscoped)>(env, "fnResultUnscoped", "1", "e.message === 'result'");

  // The function is noexcept, but unmarshalling a number as a string still
  // fails on the typed path.
  check<on_call_noexcept, &stats_noexcept>(env, "fnNoexcept", "1", "e instanceof Error");

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
