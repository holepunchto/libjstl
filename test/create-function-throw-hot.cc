#include <assert.h>
#include <js.h>
#include <stdexcept>
#include <uv.h>

#include "../include/jstl.h"

js_function_statistics_t stats_void;
js_function_statistics_t stats_result;
js_function_statistics_t stats_void_no_env;
js_function_statistics_t stats_result_no_env;

void
on_call_void(js_env_t *, js_receiver_t, int32_t value) {
  if (value == 999999) throw std::runtime_error("void");
}

int32_t
on_call_result(js_env_t *, js_receiver_t, int32_t value) {
  if (value == 999999) throw std::runtime_error("result");

  return value;
}

void
on_call_void_no_env(js_receiver_t, int32_t value) {
  if (value == 999999) throw std::runtime_error("void no env");
}

int32_t
on_call_result_no_env(js_receiver_t, int32_t value) {
  if (value == 999999) throw std::runtime_error("result no env");

  return value;
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

  js_handle_t fn_void;
  e = js_create_function<on_call_void, &stats_void>(env, fn_void);
  assert(e == 0);

  e = js_set_named_property(env, global, "fnVoid", static_cast<js_value_t *>(fn_void));
  assert(e == 0);

  js_handle_t fn_result;
  e = js_create_function<on_call_result, &stats_result>(env, fn_result);
  assert(e == 0);

  e = js_set_named_property(env, global, "fnResult", static_cast<js_value_t *>(fn_result));
  assert(e == 0);

  js_handle_t fn_void_no_env;
  e = js_create_function<on_call_void_no_env, &stats_void_no_env>(env, fn_void_no_env);
  assert(e == 0);

  e = js_set_named_property(env, global, "fnVoidNoEnv", static_cast<js_value_t *>(fn_void_no_env));
  assert(e == 0);

  js_handle_t fn_result_no_env;
  e = js_create_function<on_call_result_no_env, &stats_result_no_env>(env, fn_result_no_env);
  assert(e == 0);

  e = js_set_named_property(env, global, "fnResultNoEnv", static_cast<js_value_t *>(fn_result_no_env));
  assert(e == 0);

  // Each function throws only on its last call, by then hot and fast-called,
  // with its own message: the last message caught in each loop has to be its
  // own.
  assert(run(env, "function hot(fn) { let message = null; for (let i = 0; i < 1000000; i++) { try { fn(i) } catch (e) { message = e.message } } return message } hot((i) => fnVoid(i)) === 'void' && hot((i) => fnResult(i)) === 'result' && hot((i) => fnVoidNoEnv(i)) === 'void no env' && hot((i) => fnResultNoEnv(i)) === 'result no env'"));

  assert(stats_void.calls(js_function_call_t::typed) > 0);
  assert(stats_result.calls(js_function_call_t::typed) > 0);
  assert(stats_void_no_env.calls(js_function_call_t::typed) > 0);
  assert(stats_result_no_env.calls(js_function_call_t::typed) > 0);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
