#include <assert.h>
#include <js.h>
#include <uv.h>

#include "../include/jstl.h"

void
on_call(js_env_t *, js_receiver_t) {}

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

  e = js_set_property(env, object, "foo", uint32_t(42));
  assert(e == 0);

  uint32_t value;
  e = js_get_property(env, object, "foo", value);
  assert(e == 0);

  assert(value == 42);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
