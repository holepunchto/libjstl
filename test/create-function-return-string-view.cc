#include <assert.h>
#include <js.h>
#include <uv.h>

#include <jstl/std.h>

static const char utf8[] = "hello world";

static const char16_t utf16[] = u"hello world";

std::string_view
on_call_utf8(js_env_t *) {
  return std::string_view(utf8, 5);
}

std::u16string_view
on_call_utf16(js_env_t *) {
  return std::u16string_view(utf16, 5);
}

template <auto fn>
static std::string
call(js_env_t *env) {
  int e;

  js_handle_t handle;
  e = js_create_function<fn>(env, handle);
  assert(e == 0);

  js_string_t result;
  e = js_call_function(env, js_function_t<js_string_t>(static_cast<js_value_t *>(handle)), result);
  assert(e == 0);

  std::string value;
  e = js_get_value(env, result, value);
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

  assert(call<on_call_utf8>(env) == "hello");

  assert(call<on_call_utf16>(env) == "hello");

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
