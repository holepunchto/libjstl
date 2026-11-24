#include <assert.h>
#include <js.h>
#include <uv.h>

#include <jstl/std.h>

// A character outside the Basic Multilingual Plane, which takes a surrogate
// pair in UTF-16, between two ASCII characters.
static const std::u16string utf16 = u"a\U0001F600b";

static const std::string utf8 = "a\xF0\x9F\x98\x80" "b";

std::u16string
on_call(js_env_t *, std::u16string value) {
  assert(value == utf16);

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

  js_handle_t handle;
  e = js_create_function<on_call>(env, handle);
  assert(e == 0);

  js_string_t argument;
  e = js_create_string(env, utf8.c_str(), argument);
  assert(e == 0);

  js_string_t result;
  e = js_call_function(env, js_function_t<js_string_t, js_string_t>(static_cast<js_value_t *>(handle)), argument, result);
  assert(e == 0);

  std::string value;
  e = js_get_value(env, result, value);
  assert(e == 0);

  assert(value == utf8);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
