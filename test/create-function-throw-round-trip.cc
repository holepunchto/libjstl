#include <assert.h>
#include <js.h>
#include <stdexcept>
#include <string>
#include <uv.h>

#include "../include/jstl.h"

struct test_error : std::runtime_error {
  test_error() : std::runtime_error("thrown from native") {}
};

void
on_throw(js_env_t *) {
  throw test_error();
}

bool
on_catch_native(js_env_t *env, js_function_t<void> fn) {
  int e = js_call_function(env, fn);

  if (e == 0) return false;

  try {
    js_check_exception(env);
  } catch (const test_error &error) {
    return std::string(error.what()) == "thrown from native";
  }

  return false;
}

void
on_propagate(js_env_t *env, js_function_t<void> fn) {
  int e = js_call_function(env, fn);

  if (e < 0) js_check_exception(env);
}

std::string
on_describe(js_env_t *env, js_function_t<void> fn) {
  int e = js_call_function(env, fn);

  assert(e < 0);

  try {
    js_check_exception(env);
  } catch (const js_exception_t &error) {
    return error.what();
  }

  return "";
}

static void
define(js_env_t *env, js_value_t *global, const char *name, js_handle_t &fn) {
  int e;
  e = js_set_named_property(env, global, name, static_cast<js_value_t *>(fn));
  assert(e == 0);
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

  js_handle_t throw_fn, catch_native_fn, propagate_fn, describe_fn;

  e = js_create_function<on_throw>(env, throw_fn);
  assert(e == 0);

  e = js_create_function<on_catch_native>(env, catch_native_fn);
  assert(e == 0);

  e = js_create_function<on_propagate>(env, propagate_fn);
  assert(e == 0);

  e = js_create_function<on_describe>(env, describe_fn);
  assert(e == 0);

  define(env, global, "throwNative", throw_fn);
  define(env, global, "catchNative", catch_native_fn);
  define(env, global, "propagate", propagate_fn);
  define(env, global, "describe", describe_fn);

  assert(run(env, "catchNative(() => throwNative())"));

  assert(run(env, "catchNative(() => { try { throwNative() } catch (e) { throw e } })"));

  assert(run(env, "const error = new Error('from js'); try { propagate(() => { throw error }); false } catch (e) { e === error }"));

  assert(run(env, "try { propagate(() => { throw 42 }); false } catch (e) { e === 42 }"));

  assert(run(env, "describe(() => { throw new TypeError('bad') }) === 'TypeError: bad'"));

  assert(run(env, "describe(() => { throw 42 }) === '42'"));

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
