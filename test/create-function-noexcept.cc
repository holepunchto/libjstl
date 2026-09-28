#include <assert.h>
#include <js.h>
#include <uv.h>

#include "../include/jstl.h"

static int32_t called = 0;

int32_t
on_add(js_env_t *, int32_t a, int32_t b) noexcept {
  return a + b;
}

void
on_call(int32_t value) noexcept {
  called = value;
}

void
on_throw(js_env_t *env) noexcept {
  int e;
  e = js_throw_error(env, NULL, "thrown");
  assert(e == 0);
}

void
on_call_throwing(js_env_t *) {}

static_assert(js_is_noexcept<decltype(&on_add)>);
static_assert(js_is_noexcept<decltype(&on_call)>);
static_assert(!js_is_noexcept<decltype(&on_call_throwing)>);

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

  js_handle_t add_fn, call_fn, throw_fn;

  e = js_create_function<on_add>(env, add_fn);
  assert(e == 0);

  e = js_create_function<on_call>(env, call_fn);
  assert(e == 0);

  e = js_create_function<on_throw>(env, throw_fn);
  assert(e == 0);

  define(env, global, "add", add_fn);
  define(env, global, "call", call_fn);
  define(env, global, "fail", throw_fn);

  assert(run(env, "add(1, 2) === 3"));

  assert(run(env, "call(42) === undefined"));

  assert(called == 42);

  assert(run(env, "try { fail(); false } catch (e) { e.message === 'thrown' }"));

  if (js_is_debug) {
    assert(run(env, "try { add('1', 2); false } catch (e) { e instanceof TypeError }"));
  }

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
