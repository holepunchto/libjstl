#include <assert.h>
#include <js.h>
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

std::shared_ptr<data_t>
on_call(js_env_t *env) {
  auto data = std::make_shared<data_t>();

  data->foo = 42;
  data->bar = true;

  return data;
}

int
main() {
  int e;

  uv_loop_t *loop = uv_default_loop();

  js_platform_options_t options = {
    .expose_garbage_collection = true,
    .trace_garbage_collection = true,
  };

  js_platform_t *platform;
  e = js_create_platform(loop, &options, &platform);
  assert(e == 0);

  js_env_t *env;
  e = js_create_env(loop, platform, NULL, &env);
  assert(e == 0);

  js_handle_scope_t *scope;
  e = js_open_handle_scope(env, &scope);
  assert(e == 0);

  js_function_t<std::shared_ptr<data_t>> fn;
  e = js_create_function<on_call>(env, fn);
  assert(e == 0);

  std::shared_ptr<data_t> result;
  e = js_call_function(env, fn, result);
  assert(e == 0);

  assert(constructor_called);

  assert(result->foo == 42);
  assert(result->bar == true);

  result.reset();

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_request_garbage_collection(env);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);

  assert(destructor_called);
}
