#include <assert.h>
#include <js.h>
#include <stdbool.h>
#include <uv.h>

#include "../include/jstl.h"

static bool constructor_called = false;
static bool destructor_called = false;

struct data_t {
  int32_t foo;
  bool bar;

  data_t() {
    constructor_called = true;
  }

  ~data_t() {
    destructor_called = true;
  }
};

void
on_call(js_env_t *env, std::unique_ptr<data_t> ptr) {
  assert(ptr->foo == 42);
  assert(ptr->bar == true);
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

  js_function_t<void, std::unique_ptr<data_t>> fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  auto data = std::make_unique<data_t>();

  data->foo = 42;
  data->bar = true;

  e = js_call_function(env, fn, std::move(data));
  assert(e == 0);

  assert(data == nullptr);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);
}
