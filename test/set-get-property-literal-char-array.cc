#include <assert.h>
#include <js.h>
#include <string.h>
#include <uv.h>

#include "../include/jstl.h"

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

  e = js_set_property(env, object, "foo", "bar");
  assert(e == 0);

  char value[4];
  e = js_get_property(env, object, "foo", value);
  assert(e == 0);

  assert(strcmp(value, "bar") == 0);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
