#include <assert.h>
#include <js.h>
#include <uv.h>

#include <jstl.h>

bool
on_call(js_env_t *env, int32_t n) {
  assert(n == 42);

  return n == 42;
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

  js_object_t object;
  e = js_create_object(env, object);
  assert(e == 0);

  e = js_set_property<on_call>(env, object, "foo");
  assert(e == 0);

  js_function_t<bool, int32_t> fn;
  e = js_get_property(env, object, "foo", fn);
  assert(e == 0);

  bool result;
  e = js_call_function(env, fn, 42, result);
  assert(e == 0);

  assert(result == true);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
