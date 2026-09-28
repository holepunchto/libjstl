#include <assert.h>
#include <exception>
#include <js.h>
#include <new>
#include <stdexcept>
#include <uv.h>

#include "../include/jstl.h"

struct unknown_error {};

static std::exception_ptr next;

void
on_call(js_env_t *) {
  std::rethrow_exception(next);
}

int32_t
on_call_with_result(js_env_t *) {
  std::rethrow_exception(next);
}

void
on_call_with_pending_exception(js_env_t *env) {
  int e;
  e = js_throw_error(env, NULL, "pending");
  assert(e == 0);

  throw js_pending_exception;
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

  js_handle_t fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  e = js_set_named_property(env, global, "fn", static_cast<js_value_t *>(fn));
  assert(e == 0);

  js_handle_t fn_with_result;
  e = js_create_function<on_call_with_result>(env, fn_with_result);
  assert(e == 0);

  e = js_set_named_property(env, global, "fnWithResult", static_cast<js_value_t *>(fn_with_result));
  assert(e == 0);

  js_handle_t fn_with_pending_exception;
  e = js_create_function<on_call_with_pending_exception>(env, fn_with_pending_exception);
  assert(e == 0);

  e = js_set_named_property(env, global, "fnWithPendingException", static_cast<js_value_t *>(fn_with_pending_exception));
  assert(e == 0);

  next = std::make_exception_ptr(std::invalid_argument("invalid"));

  assert(run(env, "try { fn(); false } catch (e) { e instanceof TypeError && e.message === 'invalid' }"));

  next = std::make_exception_ptr(std::out_of_range("out of range"));

  assert(run(env, "try { fn(); false } catch (e) { e instanceof RangeError && e.message === 'out of range' }"));

  next = std::make_exception_ptr(std::runtime_error("runtime"));

  assert(run(env, "try { fn(); false } catch (e) { e.constructor === Error && e.message === 'runtime' }"));

  next = std::make_exception_ptr(std::bad_alloc());

  assert(run(env, "try { fn(); false } catch (e) { e.constructor === Error }"));

  next = std::make_exception_ptr(unknown_error());

  assert(run(env, "try { fn(); false } catch (e) { e.constructor === Error && e.message === 'Unknown native exception' }"));

  next = std::make_exception_ptr(std::invalid_argument("invalid"));

  assert(run(env, "try { fnWithResult(); false } catch (e) { e instanceof TypeError && e.message === 'invalid' }"));

  assert(run(env, "try { fnWithPendingException(); false } catch (e) { e.message === 'pending' }"));

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
