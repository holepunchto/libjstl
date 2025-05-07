#include <assert.h>
#include <js.h>
#include <uv.h>

#include "../include/jstl.h"

struct data {
  int32_t foo;
  bool bar;
};

template <>
struct js_type_info_t<struct data> {
  template <bool checked>
  static auto
  marshall(js_env_t *env, struct data value, js_value_t *&result) {
    int err;

    err = js_create_object(env, &result);
    if (err < 0) return err;

    err = js_set_property(env, js_object_t(result), "foo", value.foo);
    if (err < 0) return err;

    err = js_set_property(env, js_object_t(result), "bar", value.bar);
    if (err < 0) return err;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, struct data &result) {
    int err;

    err = js_get_property(env, js_object_t(value), "foo", result.foo);
    if (err < 0) return err;

    err = js_get_property(env, js_object_t(value), "bar", result.bar);
    if (err < 0) return err;

    return 0;
  }
};

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

  {
    struct data data;
    data.foo = 42;
    data.bar = true;

    e = js_set_property(env, object, "foo", data);
    assert(e == 0);
  }
  {
    struct data data;
    e = js_get_property(env, object, "foo", data);
    assert(e == 0);

    assert(data.foo == 42);
    assert(data.bar == true);
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
