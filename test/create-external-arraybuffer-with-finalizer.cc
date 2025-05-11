#include <assert.h>
#include <js.h>
#include <uv.h>

#include "../include/jstl.h"

bool did_finalize = false;

void
on_finalize(js_env_t *, uint8_t *data) {
  did_finalize = true;
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

  uint8_t data[5];

  data[0] = 'h';
  data[1] = 'e';
  data[2] = 'l';
  data[3] = 'l';
  data[4] = 'o';

  js_arraybuffer_t arraybuffer;
  e = js_create_external_arraybuffer<uint8_t, on_finalize>(env, data, 5, arraybuffer);
  assert(e == 0);

  e = js_close_handle_scope(env, scope);
  assert(e == 0);

  e = js_destroy_env(env);
  assert(e == 0);

  e = js_destroy_platform(platform);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);

  assert(did_finalize);
}
