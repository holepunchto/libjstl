#include <assert.h>
#include <js.h>
#include <string>
#include <uv.h>
#include <vector>

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

  js_object_t nested;
  e = js_create_object(env, nested, js_property_t("foo", 42));
  assert(e == 0);

  js_array_t array;
  e = js_create_array(env, std::vector<std::string>{"a", "b"}, array);
  assert(e == 0);

  js_value_t *null;
  e = js_get_null(env, &null);
  assert(e == 0);

  js_object_t object;
  e = js_create_object(
    env,
    object,
    js_property_t("string", std::string("hello")),
    js_property_t("object", nested),
    js_property_t("array", array),
    js_property_t("handle", js_handle_t(null)),
    js_property_t("vector", std::vector<std::string>{"c", "d"})
  );
  assert(e == 0);

  std::string string;
  e = js_get_property(env, object, "string", string);
  assert(e == 0);
  assert(string == "hello");

  js_object_t object_value;
  e = js_get_property(env, object, "object", object_value);
  assert(e == 0);

  int32_t foo;
  e = js_get_property(env, object_value, "foo", foo);
  assert(e == 0);
  assert(foo == 42);

  std::vector<std::string> strings;
  e = js_get_property(env, object, "array", strings);
  assert(e == 0);
  assert(strings == std::vector<std::string>({"a", "b"}));

  e = js_get_property(env, object, "vector", strings);
  assert(e == 0);
  assert(strings == std::vector<std::string>({"c", "d"}));

  js_value_t *handle;
  e = js_get_named_property(env, static_cast<js_value_t *>(object), "handle", &handle);
  assert(e == 0);

  bool is_null;
  e = js_is_null(env, handle, &is_null);
  assert(e == 0);
  assert(is_null);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
