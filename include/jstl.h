#pragma once

#include <array>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <js.h>
#include <stdbool.h>
#include <stdint.h>
#include <utf.h>

#ifndef NDEBUG
static constexpr bool js_is_debug = true;
#else
static constexpr bool js_is_debug = false;
#endif

struct js_handle_t {
  js_value_t *value;

  js_handle_t() : value(nullptr) {}

  js_handle_t(js_value_t *value) : value(value) {}

  virtual ~js_handle_t() = default;

  operator js_value_t *() const {
    return value;
  }

  operator js_value_t **() {
    return &value;
  }
};

struct js_primitive_t : js_handle_t {
  js_primitive_t() : js_handle_t() {}

  js_primitive_t(js_value_t *value) : js_handle_t(value) {}
};

struct js_boolean_t : js_primitive_t {
  js_boolean_t() : js_primitive_t() {}

  js_boolean_t(js_value_t *value) : js_primitive_t(value) {}
};

struct js_numeric_t : js_primitive_t {
  js_numeric_t() : js_primitive_t() {}

  js_numeric_t(js_value_t *value) : js_primitive_t(value) {}
};

struct js_number_t : js_numeric_t {
  js_number_t() : js_numeric_t() {}

  js_number_t(js_value_t *value) : js_numeric_t(value) {}
};

struct js_integer_t : js_number_t {
  js_integer_t() : js_number_t() {}

  js_integer_t(js_value_t *value) : js_number_t(value) {}
};

struct js_bigint_t : js_numeric_t {
  js_bigint_t() : js_numeric_t() {}

  js_bigint_t(js_value_t *value) : js_numeric_t(value) {}
};

struct js_name_t : js_primitive_t {
  js_name_t() : js_primitive_t() {}

  js_name_t(js_value_t *value) : js_primitive_t(value) {}
};

struct js_string_t : js_name_t {
  js_string_t() : js_name_t() {}

  js_string_t(js_value_t *value) : js_name_t(value) {}
};

struct js_symbol_t : js_name_t {
  js_symbol_t() : js_name_t() {}

  js_symbol_t(js_value_t *value) : js_name_t(value) {}
};

struct js_object_t : js_handle_t {
  js_object_t() : js_handle_t() {}

  js_object_t(js_value_t *value) : js_handle_t(value) {}
};

struct js_array_t : js_object_t {
  js_array_t() : js_object_t() {}

  js_array_t(js_value_t *value) : js_object_t(value) {}
};

struct js_arraybuffer_t : js_object_t {
  js_arraybuffer_t() : js_object_t() {}

  js_arraybuffer_t(js_value_t *value) : js_object_t(value) {}
};

template <typename T>
struct js_typedarray_t : js_object_t {
  js_typedarray_t() : js_object_t() {}

  js_typedarray_t(js_value_t *value) : js_object_t(value) {}
};

struct js_receiver_t : js_handle_t {
  js_receiver_t() : js_handle_t() {}

  js_receiver_t(js_value_t *value) : js_handle_t(value) {}

  js_receiver_t(const js_handle_t &value) : js_handle_t(value.value) {}
};

template <typename R, typename... A>
struct js_function_t : js_object_t {
  js_function_t() : js_object_t() {}

  js_function_t(js_value_t *value) : js_object_t(value) {}
};

struct js_external_t : js_handle_t {
  js_external_t() : js_handle_t() {}

  js_external_t(js_value_t *value) : js_handle_t(value) {}
};

template <typename T>
struct js_persistent_t {
  js_env_t *env;
  js_ref_t *ref;

  js_persistent_t() : env(nullptr), ref(nullptr) {}

  js_persistent_t(js_persistent_t &&that) : env(that.env), ref(std::exchange(that.ref, nullptr)) {}

  js_persistent_t(const js_persistent_t &) = delete;

  ~js_persistent_t() {
    if (ref == nullptr) return;

    int err;
    err = js_delete_reference(env, ref);
    assert(err == 0);
  }

  void
  operator=(const js_persistent_t &) = delete;
};

template <int check(js_env_t *, js_value_t *, bool *result)>
static inline int
js_check_value(js_env_t *env, js_value_t *value, const char *label) {
  int err;

  bool is_type;
  err = check(env, value, &is_type);
  if (err < 0) return err;

  if (is_type) return 0;

  err = js_throw_type_errorf(env, nullptr, "Value is not of type '%s'", label);
  assert(err == 0);

  return js_pending_exception;
}

template <int check(js_env_t *, const js_handle_t &, bool &result)>
static inline int
js_check_value(js_env_t *env, js_value_t *value, const char *label) {
  int err;

  bool is_type;
  err = check(env, value, is_type);
  if (err < 0) return err;

  if (is_type) return 0;

  err = js_throw_type_errorf(env, nullptr, "Value is not of type '%s'", label);
  assert(err == 0);

  return js_pending_exception;
}

template <typename T>
struct js_typedarray_info_t;

template <>
struct js_typedarray_info_t<int8_t> {
  static constexpr auto type = js_int8array;

  static constexpr auto label = "int8array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_int8array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<uint8_t> {
  static constexpr auto type = js_uint8array;

  static constexpr auto label = "uint8array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_uint8array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<int16_t> {
  static constexpr auto type = js_int16array;

  static constexpr auto label = "int16array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_int16array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<uint16_t> {
  static constexpr auto type = js_uint16array;

  static constexpr auto label = "uint16array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_uint16array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<int32_t> {
  static constexpr auto type = js_int32array;

  static constexpr auto label = "int32array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_int32array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<uint32_t> {
  static constexpr auto type = js_uint32array;

  static constexpr auto label = "uint32array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_uint32array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<int64_t> {
  static constexpr auto type = js_bigint64array;

  static constexpr auto label = "bigint64array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_bigint64array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<uint64_t> {
  static constexpr auto type = js_biguint64array;

  static constexpr auto label = "biguint64array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_biguint64array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<float> {
  static constexpr auto type = js_float32array;

  static constexpr auto label = "float32array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_float32array(env, value.value, &result);
  }
};

template <>
struct js_typedarray_info_t<double> {
  static constexpr auto type = js_float64array;

  static constexpr auto label = "float64array";

  static inline auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_float64array(env, value.value, &result);
  }
};

template <typename T>
static inline auto
js_is_typedarray(js_env_t *env, const js_handle_t &value, bool &result) {
  return js_typedarray_info_t<T>::is(env, value, result);
}

template <typename T>
struct js_type_info_t;

template <>
struct js_type_info_t<void> {
  using type = void;

  static constexpr auto signature = js_undefined;

  template <bool checked>
  static auto
  marshall(js_env_t *env, js_value_t *&result) {
    return js_get_undefined(env, &result);
  }
};

template <>
struct js_type_info_t<bool> {
  using type = bool;

  static constexpr auto signature = js_boolean;

  static auto
  marshall(bool value, bool &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  marshall(js_env_t *, bool value, bool &result) {
    return marshall(value, result);
  }

  template <bool checked>
  static auto
  marshall(js_env_t *env, bool value, js_value_t *&result) {
    return js_get_boolean(env, value, &result);
  }

  static auto
  unmarshall(bool value, bool &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *, bool value, bool &result) {
    return unmarshall(value, result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, bool &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_boolean>(env, value, "boolean");
      if (err < 0) return err;
    }

    return js_get_value_bool(env, value, &result);
  }
};

template <>
struct js_type_info_t<int32_t> {
  using type = int32_t;

  static constexpr auto signature = js_int32;

  static auto
  marshall(int32_t value, int32_t &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  marshall(js_env_t *, int32_t value, int32_t &result) {
    return marshall(value, result);
  }

  template <bool checked>
  static auto
  marshall(js_env_t *env, int32_t value, js_value_t *&result) {
    return js_create_int32(env, value, &result);
  }

  static auto
  unmarshall(int32_t value, int32_t &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *, int32_t value, int32_t &result) {
    return unmarshall(value, result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, int32_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_int32>(env, value, "int32");
      if (err < 0) return err;
    }

    return js_get_value_int32(env, value, &result);
  }
};

template <>
struct js_type_info_t<uint32_t> {
  using type = uint32_t;

  static constexpr auto signature = js_uint32;

  static auto
  marshall(uint32_t value, uint32_t &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  marshall(js_env_t *, uint32_t value, uint32_t &result) {
    return marshall(value, result);
  }

  template <bool checked>
  static auto
  marshall(js_env_t *env, uint32_t value, js_value_t *&result) {
    return js_create_uint32(env, value, &result);
  }

  static auto
  unmarshall(uint32_t value, uint32_t &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *, uint32_t value, uint32_t &result) {
    return unmarshall(value, result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, uint32_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_uint32>(env, value, "uint32");
      if (err < 0) return err;
    }

    return js_get_value_uint32(env, value, &result);
  }
};

template <>
struct js_type_info_t<int64_t> {
  using type = int64_t;

  static constexpr auto signature = js_int64;

  static auto
  marshall(int64_t value, int64_t &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  marshall(js_env_t *, int64_t value, int64_t &result) {
    return marshall(value, result);
  }

  template <bool checked>
  static auto
  marshall(js_env_t *env, int64_t value, js_value_t *&result) {
    return js_create_int64(env, value, &result);
  }

  static auto
  unmarshall(int64_t value, int64_t &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *, int64_t value, int64_t &result) {
    return unmarshall(value, result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, int64_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_number>(env, value, "int64");
      if (err < 0) return err;
    }

    return js_get_value_int64(env, value, &result);
  }
};

template <>
struct js_type_info_t<double> {
  using type = double;

  static constexpr auto signature = js_float64;

  static auto
  marshall(double value, double &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  marshall(js_env_t *, double value, double &result) {
    return marshall(value, result);
  }

  template <bool checked>
  static auto
  marshall(js_env_t *env, double value, js_value_t *&result) {
    return js_create_double(env, value, &result);
  }

  static auto
  unmarshall(double value, double &result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *, double value, double &result) {
    return unmarshall(value, result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, double &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_number>(env, value, "double");
      if (err < 0) return err;
    }

    return js_get_value_double(env, value, &result);
  }
};

template <>
struct js_type_info_t<js_bigint_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_bigint;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_bigint_t &bigint, js_value_t *&result) {
    result = bigint.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_bigint_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_bigint>(env, value, "bigint");
      if (err < 0) return err;
    }

    result = js_bigint_t(value);

    return 0;
  }
};

template <>
struct js_type_info_t<js_string_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_string_t &string, js_value_t *&result) {
    result = string.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_string_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    result = js_string_t(value);

    return 0;
  }
};

template <>
struct js_type_info_t<js_symbol_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_symbol;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_symbol_t &symbol, js_value_t *&result) {
    result = symbol.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_symbol_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_symbol>(env, value, "symbol");
      if (err < 0) return err;
    }

    result = js_symbol_t(value);

    return 0;
  }
};

template <>
struct js_type_info_t<js_object_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_object_t &object, js_value_t *&result) {
    result = object.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_object_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_object>(env, value, "object");
      if (err < 0) return err;
    }

    result = js_object_t(value);

    return 0;
  }
};

template <>
struct js_type_info_t<js_array_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_array_t &array, js_value_t *&result) {
    result = array.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_array_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    result = js_array_t(value);

    return 0;
  }
};

template <>
struct js_type_info_t<js_arraybuffer_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_arraybuffer_t &arraybuffer, js_value_t *&result) {
    result = arraybuffer.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_arraybuffer_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_arraybuffer>(env, value, "arraybuffer");
      if (err < 0) return err;
    }

    result = js_arraybuffer_t(value);

    return 0;
  }
};

namespace {

static uint8_t js_arraybuffer_span_nil[1] = {0};

}

struct js_arraybuffer_span_t {
  js_arraybuffer_span_t() : data_(js_arraybuffer_span_nil), size_(0) {}

  js_arraybuffer_span_t(uint8_t *data, size_t len) : data_(len == 0 ? js_arraybuffer_span_nil : data), size_(len) {}

  uint8_t &
  operator[](const size_t i) {
    return data_[i];
  }

  const uint8_t &
  operator[](const size_t i) const {
    return data_[i];
  }

  uint8_t *data() const {
    return data_;
  }

  size_t size() const {
    return size_;
  }

  uint8_t *begin() const {
    return data_;
  }

  uint8_t *end() const {
    return data_ + size_;
  }

private:
  uint8_t *data_;
  size_t size_;
};

template <>
struct js_type_info_t<js_arraybuffer_span_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const js_arraybuffer_span_t &view, js_value_t *&result) {
    int err;

    uint8_t *data;
    err = js_create_arraybuffer(env, view.size(), (void **) &data, &result);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_arraybuffer_span_t &result) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_arraybuffer>(env, value, "arraybuffer");
      if (err < 0) return err;
    }

    uint8_t *data;
    size_t len;
    err = js_get_arraybuffer_info(env, value, (void **) &data, &len);
    if (err < 0) return err;

    result = js_arraybuffer_span_t(data, len);

    return 0;
  }
};

template <typename T>
struct js_type_info_t<js_typedarray_t<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_typedarray_t<T> &typedarray, js_value_t *&result) {
    result = typedarray.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_t<T> &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_typedarray<T>>(env, value, js_typedarray_info_t<T>::label);
      if (err < 0) return err;
    }

    result = js_typedarray_t<T>(value);

    return 0;
  }
};

namespace {

template <typename T>
static T js_typedarray_span_nil[1] = {T(0)};

}

template <typename T>
struct js_typedarray_span_t {
  js_typedarray_span_t() : data_(js_typedarray_span_nil<T>), size_(0) {}

  js_typedarray_span_t(T *data, size_t len) : data_(len == 0 ? js_typedarray_span_nil<T> : data), size_(len) {}

  T &
  operator[](const size_t i) {
    return data_[i];
  }

  const T &
  operator[](const size_t i) const {
    return data_[i];
  }

  T *data() const {
    return data_;
  }

  size_t size() const {
    return size_;
  }

  size_t size_bytes() const {
    return size_ * sizeof(T);
  }

  T *begin() const {
    return data_;
  }

  T *end() const {
    return data_ + size_;
  }

private:
  T *data_;
  size_t size_;
};

template <typename T>
struct js_type_info_t<js_typedarray_span_t<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const js_typedarray_span_t<T> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    T *data;
    err = js_create_arraybuffer(env, view.size_bytes(), (void **) &data, &arraybuffer);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return js_create_typedarray(env, js_typedarray_info_t<T>::type, view.size(), arraybuffer, 0, &result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_span_t<T> &result) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_typedarray<T>>(env, value, js_typedarray_info_t<T>::label);
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    err = js_get_typedarray_info(env, value, nullptr, (void **) &data, &len, nullptr, nullptr);
    if (err < 0) return err;

    result = js_typedarray_span_t(data, len);

    return 0;
  }
};

template <>
struct js_type_info_t<js_receiver_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  static auto
  marshall(const js_receiver_t &receiver, js_value_t *&result) {
    result = receiver.value;

    return 0;
  }

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_receiver_t &receiver, js_value_t *&result) {
    return marshall(receiver, result);
  }

  static auto
  unmarshall(js_value_t *value, js_receiver_t &result) {
    result = js_receiver_t(value);

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *, js_value_t *value, js_receiver_t &result) {
    return unmarshall(value, result);
  }
};

template <typename R, typename... A>
struct js_type_info_t<js_function_t<R, A...>> {
  using type = js_value_t *;

  static constexpr auto signature = js_function;

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_function_t<R, A...> &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_function>(env, value, "function");
      if (err < 0) return err;
    }

    result = js_function_t<R, A...>(value);

    return 0;
  }
};

template <>
struct js_type_info_t<js_external_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_external;

  template <bool checked>
  static auto
  marshall(js_env_t *, const js_external_t &external, js_value_t *&result) {
    result = external.value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_external_t &result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_external>(env, value, "external");
      if (err < 0) return err;
    }

    result = js_external_t(value);

    return 0;
  }
};

template <typename T>
struct js_type_info_t<T *> {
  using type = T *;

  static constexpr auto signature = js_external;

  static auto
  marshall(T *value, T *&result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  marshall(js_env_t *, T *value, T *&result) {
    return marshall(value, result);
  }

  template <bool checked>
  static auto
  marshall(js_env_t *env, T *value, js_value_t *&result) {
    return js_create_external(env, (void *) value, nullptr, nullptr, &result);
  }

  static auto
  unmarshall(T *value, T *&result) {
    result = value;

    return 0;
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *, T *value, T *&result) {
    return unmarshall(value, result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, T *&result) {
    if constexpr (checked) {
      int err;
      err = js_check_value<js_is_external>(env, value, "external");
      if (err < 0) return err;
    }

    return js_get_value_external(env, value, (void **) &result);
  }
};

template <size_t N>
struct js_type_info_t<char[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const char value[N], js_value_t *&result) {
    return js_create_string_utf8(env, (const utf8_t *) value, N, &result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, char result[N]) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf8(env, value, (utf8_t *) result, N, &len);
    if (err < 0) return err;

    assert(len == N);

    return 0;
  }
};

template <size_t N>
struct js_type_info_t<const char[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const char value[N], js_value_t *&result) {
    return js_create_string_utf8(env, (const utf8_t *) value, N, &result);
  }
};

template <>
struct js_type_info_t<char *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const char *value, js_value_t *&result) {
    return js_create_string_utf8(env, (const utf8_t *) value, -1, &result);
  }
};

template <>
struct js_type_info_t<const char *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const char *value, js_value_t *&result) {
    return js_create_string_utf8(env, (const utf8_t *) value, -1, &result);
  }
};

template <>
struct js_type_info_t<std::string> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const std::string &value, js_value_t *&result) {
    return js_create_string_utf8(env, (const utf8_t *) value.data(), value.length(), &result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::string &result) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (err < 0) return err;

    result.resize(len);

    return js_get_value_string_utf8(env, value, (utf8_t *) result.data(), result.length(), nullptr);
  }
};

template <typename T, size_t N>
struct js_type_info_t<T[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const T array[N], js_value_t *&result) {
    int err;

    err = js_create_array_with_length(env, N, &result);
    assert(err == 0);

    js_value_t *values[N];

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template marshall<checked>(env, array[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, (const js_value_t **) values, N, 0);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, T result[N]) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    js_value_t *values[N];
    uint32_t len;
    err = js_get_array_elements(env, value, values, N, 0, &len);
    if (err < 0) return err;

    assert(len == N);

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template unmarshall<checked>(env, values[i], result[i]);
      if (err < 0) return err;
    }

    return 0;
  }
};

template <typename T, size_t N>
struct js_type_info_t<std::array<T, N>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const std::array<T, N> &array, js_value_t *&result) {
    int err;

    err = js_create_array_with_length(env, N, &result);
    assert(err == 0);

    js_value_t *values[N];

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template marshall<checked>(env, array[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, (const js_value_t **) values, N, 0);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::array<T, N> &result) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    js_value_t *values[N];
    uint32_t len;
    err = js_get_array_elements(env, value, values, N, 0, &len);
    if (err < 0) return err;

    assert(len == N);

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template unmarshall<checked>(env, values[i], result[i]);
      if (err < 0) return err;
    }

    return 0;
  }
};

template <typename T>
struct js_type_info_t<std::vector<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const std::vector<T> &vector, js_value_t *&result) {
    int err;

    auto len = vector.size();

    err = js_create_array_with_length(env, len, &result);
    assert(err == 0);

    std::vector<js_value_t *> values(len);

    for (uint32_t i = 0; i < len; i++) {
      err = js_type_info_t<T>::template marshall<checked>(env, vector[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, (const js_value_t **) values.data(), len, 0);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::vector<T> &result) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    uint32_t len;
    err = js_get_array_length(env, value, &len);
    if (err < 0) return err;

    std::vector<js_value_t *> values(len);
    err = js_get_array_elements(env, value, values.data(), len, 0, &len);
    if (err < 0) return err;

    result.resize(len);

    for (uint32_t i = 0; i < len; i++) {
      err = js_type_info_t<T>::template unmarshall<checked>(env, values[i], result[i]);
      if (err < 0) return err;
    }

    return 0;
  }
};

template <typename... T>
struct js_type_info_t<std::tuple<T...>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked, size_t... I>
  static auto
  marshall(js_env_t *env, const std::tuple<T...> &tuple, js_value_t *&result, std::index_sequence<I...>) {
    int err;
    err = js_create_array_with_length(env, sizeof...(T), &result);
    assert(err == 0);

    try {
      js_value_t *values[] = {
        js_marshall_untyped_value<checked, T>(env, std::get<I>(tuple))...
      };

      return js_set_array_elements(env, result, (const js_value_t **) values, sizeof...(T), 0);
    } catch (int err) {
      return err;
    }
  }

  template <bool checked>
  static auto
  marshall(js_env_t *env, const std::tuple<T...> &tuple, js_value_t *&result) {
    return marshall<checked>(env, tuple, result, std::index_sequence_for<T...>());
  }

  template <bool checked, size_t... I>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::tuple<T...> &result, std::index_sequence<I...>) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    js_value_t *values[sizeof...(T)];
    uint32_t len;
    err = js_get_array_elements(env, value, values, sizeof...(T), 0, &len);
    if (err < 0) return err;

    assert(len == sizeof...(T));

    try {
      result = {
        js_unmarshall_untyped_value<checked, T>(env, values[I])...
      };

      return 0;
    } catch (int err) {
      return err;
    }
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::tuple<T...> &result) {
    return unmarshall<checked>(env, value, result, std::index_sequence_for<T...>());
  }
};

template <typename T>
struct js_type_info_t<std::span<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const std::span<T> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    T *data;
    err = js_create_arraybuffer(env, view.size_bytes(), (void **) &data, &arraybuffer);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return js_create_typedarray(env, js_typedarray_info_t<T>::type, view.size(), arraybuffer, 0, &result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::span<T> &result) {
    int err;

    if constexpr (checked) {
      err = js_check_value<js_is_typedarray<T>>(env, value, js_typedarray_info_t<T>::label);
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    err = js_get_typedarray_info(env, value, nullptr, (void **) &data, &len, nullptr, nullptr);
    if (err < 0) return err;

    result = std::span(data, len);

    return 0;
  }
};

template <typename T>
struct js_type_info_t<std::optional<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_type_info_t<T>::signature;

  template <bool checked>
  static auto
  marshall(js_env_t *env, const std::optional<T> &value, js_value_t *&result) {
    if (value) {
      return js_type_info_t<T>::template marshall<checked>(env, *value, result);
    }

    return js_get_undefined(env, &result);
  }

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::optional<T> &result) {
    int err;

    bool is_undefined;
    err = js_is_undefined(env, value, &is_undefined);
    if (err < 0) return err;

    if (is_undefined) {
      result = std::nullopt;
    } else {
      T unmarshalled;
      err = js_type_info_t<T>::template unmarshall<checked>(env, value, unmarshalled);
      if (err < 0) return err;

      result = std::optional(unmarshalled);
    }

    return 0;
  }
};

template <typename T>
struct js_property_t {
  std::string name;
  T value;

  js_property_t(const std::string &name, T value) : name(name), value(value) {}

  template <size_t N>
  js_property_t(const char name[N], T value) : name(name, N), value(value) {}

  js_property_t(const char *name, T value) : name(name), value(value) {}
};

template <typename T>
static inline auto
js_marshall_typed_value(T value) {
  int err;

  typename js_type_info_t<T>::type result;
  err = js_type_info_t<T>::marshall(value, result);
  if (err < 0) throw err;

  return result;
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_marshall_typed_value(js_env_t *env, T value) {
  int err;

  typename js_type_info_t<T>::type result;
  err = js_type_info_t<T>::template marshall<checked>(env, value, result);
  if (err < 0) throw err;

  return result;
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_marshall_untyped_value(js_env_t *env, T value) {
  int err;

  js_value_t *result;
  err = js_type_info_t<T>::template marshall<checked>(env, value, result);
  if (err < 0) throw err;

  return result;
}

template <bool checked = js_is_debug>
static inline auto
js_marshall_untyped_value(js_env_t *env) {
  int err;

  js_value_t *result;
  err = js_type_info_t<void>::template marshall<checked>(env, result);
  if (err < 0) throw err;

  return result;
}

template <typename T>
static inline auto
js_unmarshall_typed_value(typename js_type_info_t<T>::type value) {
  int err;

  T result;
  err = js_type_info_t<T>::unmarshall(value, result);
  if (err < 0) throw err;

  return result;
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_unmarshall_typed_value(js_env_t *env, typename js_type_info_t<T>::type value) {
  int err;

  T result;
  err = js_type_info_t<T>::template unmarshall<checked>(env, value, result);
  if (err < 0) throw err;

  return result;
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_unmarshall_untyped_value(js_env_t *env, js_value_t *value) {
  int err;

  T result;
  err = js_type_info_t<T>::template unmarshall<checked>(env, value, result);
  if (err < 0) throw err;

  return result;
}

template <typename...>
struct js_argument_info_t;

template <>
struct js_argument_info_t<> {
  static constexpr bool has_receiver = false;
};

template <typename T, typename... R>
struct js_argument_info_t<T, R...> {
  static constexpr bool has_receiver = std::is_same<T, js_receiver_t>();
};

template <auto fn>
struct js_typed_callback_t;

template <typename R, typename... A, R fn(A...)>
struct js_typed_callback_t<fn> {
  template <bool scoped, bool checked>
  static inline auto
  create() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> typename js_type_info_t<R>::type {
      return js_marshall_typed_value<R>(fn(js_unmarshall_typed_value<A>(args)...));
    };
  }
};

template <typename R, typename... A, R fn(js_env_t *, A...)>
struct js_typed_callback_t<fn> {
  template <bool scoped, bool checked>
  static inline auto
  create() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> typename js_type_info_t<R>::type {
      int err;

      js_env_t *env;
      err = js_get_typed_callback_info(info, &env, nullptr);
      assert(err == 0);

      js_escapable_handle_scope_t *scope;

      if constexpr (scoped) {
        err = js_open_escapable_handle_scope(env, &scope);
        assert(err == 0);
      }

      typename js_type_info_t<R>::type result;

      try {
        result = js_marshall_typed_value<checked, R>(env, fn(env, js_unmarshall_typed_value<checked, A>(env, args)...));

        if constexpr (scoped && std::is_same<decltype(result), js_value_t *>()) {
          err = js_escape_handle(env, scope, result, &result);
          assert(err == 0);
        }
      } catch (int err) {
        assert(err != 0);
      }

      if constexpr (scoped) {
        err = js_close_escapable_handle_scope(env, scope);
        assert(err == 0);
      }

      return result;
    };
  }
};

template <typename... A, void fn(A...)>
struct js_typed_callback_t<fn> {
  template <bool scoped, bool checked>
  static inline auto
  create() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> void {
      fn(js_unmarshall_typed_value<A>(args)...);
    };
  }
};

template <typename... A, void fn(js_env_t *, A...)>
struct js_typed_callback_t<fn> {
  template <bool scoped, bool checked>
  static inline auto
  create() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> void {
      int err;

      js_env_t *env;
      err = js_get_typed_callback_info(info, &env, nullptr);
      assert(err == 0);

      js_handle_scope_t *scope;

      if constexpr (scoped) {
        err = js_open_handle_scope(env, &scope);
        assert(err == 0);
      }

      try {
        fn(env, js_unmarshall_typed_value<checked, A>(env, args)...);
      } catch (int err) {
        assert(err != 0);
      }

      if constexpr (scoped) {
        err = js_close_handle_scope(env, scope);
        assert(err == 0);
      }
    };
  }
};

template <auto fn>
struct js_untyped_callback_t;

template <typename R, typename... A, R fn(A...)>
struct js_untyped_callback_t<fn> {
  template <bool scoped, bool checked, size_t... I>
  static inline auto
  create(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      js_handle_scope_t *scope;

      if constexpr (scoped) {
        err = js_open_handle_scope(env, &scope);
        assert(err == 0);
      }

      size_t argc = sizeof...(A);
      js_value_t *argv[sizeof...(A)];

      if constexpr (js_argument_info_t<A...>::has_receiver) {
        argc--;

        err = js_get_callback_info(env, info, &argc, &argv[1], &argv[0], nullptr);
        assert(err == 0);

        argc++;
      } else {
        err = js_get_callback_info(env, info, &argc, argv, nullptr, nullptr);
        assert(err == 0);
      }

      assert(argc == sizeof...(A));

      js_value_t *result;

      try {
        result = js_marshall_untyped_value<checked, R>(env, fn(js_unmarshall_untyped_value<checked, A>(env, argv[I])...));
      } catch (int err) {
        assert(err != 0);
      }

      if constexpr (scoped) {
        err = js_close_handle_scope(env, scope);
        assert(err == 0);
      }

      return result;
    };
  }

  template <bool scoped, bool checked>
  static inline auto
  create() {
    return create<scoped, checked>(std::index_sequence_for<A...>());
  }
};

template <typename R, typename... A, R fn(js_env_t *, A...)>
struct js_untyped_callback_t<fn> {
  template <bool scoped, bool checked, size_t... I>
  static inline auto
  create(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      js_escapable_handle_scope_t *scope;

      if constexpr (scoped) {
        err = js_open_escapable_handle_scope(env, &scope);
        assert(err == 0);
      }

      size_t argc = sizeof...(A);
      js_value_t *argv[sizeof...(A)];

      if constexpr (js_argument_info_t<A...>::has_receiver) {
        argc--;

        err = js_get_callback_info(env, info, &argc, &argv[1], &argv[0], nullptr);
        assert(err == 0);

        argc++;
      } else {
        err = js_get_callback_info(env, info, &argc, argv, nullptr, nullptr);
        assert(err == 0);
      }

      assert(argc == sizeof...(A));

      js_value_t *result;

      try {
        result = js_marshall_untyped_value<checked, R>(env, fn(env, js_unmarshall_untyped_value<checked, A>(env, argv[I])...));

        if constexpr (scoped) {
          err = js_escape_handle(env, scope, result, &result);
          assert(err == 0);
        }
      } catch (int err) {
        assert(err != 0);
      }

      if constexpr (scoped) {
        err = js_close_escapable_handle_scope(env, scope);
        assert(err == 0);
      }

      return result;
    };
  }

  template <bool scoped, bool checked>
  static inline auto
  create() {
    return create<scoped, checked>(std::index_sequence_for<A...>());
  }
};

template <typename... A, void fn(A...)>
struct js_untyped_callback_t<fn> {
  template <bool scoped, bool checked, size_t... I>
  static inline auto
  create(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      js_handle_scope_t *scope;

      if constexpr (scoped) {
        err = js_open_handle_scope(env, &scope);
        assert(err == 0);
      }

      size_t argc = sizeof...(A);
      js_value_t *argv[sizeof...(A)];

      if constexpr (js_argument_info_t<A...>::has_receiver) {
        argc--;

        err = js_get_callback_info(env, info, &argc, &argv[1], &argv[0], nullptr);
        assert(err == 0);

        argc++;
      } else {
        err = js_get_callback_info(env, info, &argc, argv, nullptr, nullptr);
        assert(err == 0);
      }

      assert(argc == sizeof...(A));

      try {
        fn(js_unmarshall_untyped_value<checked, A>(env, argv[I])...);
      } catch (int err) {
        assert(err != 0);
      }

      if constexpr (scoped) {
        err = js_close_handle_scope(env, scope);
        assert(err == 0);
      }

      return js_marshall_untyped_value<checked>(env);
    };
  }

  template <bool scoped, bool checked>
  static inline auto
  create() {
    return create<scoped, checked>(std::index_sequence_for<A...>());
  }
};

template <typename... A, void fn(js_env_t *, A...)>
struct js_untyped_callback_t<fn> {
  template <bool scoped, bool checked, size_t... I>
  static inline auto
  create(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      js_handle_scope_t *scope;

      if constexpr (scoped) {
        err = js_open_handle_scope(env, &scope);
        assert(err == 0);
      }

      size_t argc = sizeof...(A);
      js_value_t *argv[sizeof...(A)];

      if constexpr (js_argument_info_t<A...>::has_receiver) {
        argc--;

        err = js_get_callback_info(env, info, &argc, &argv[1], &argv[0], nullptr);
        assert(err == 0);

        argc++;
      } else {
        err = js_get_callback_info(env, info, &argc, argv, nullptr, nullptr);
        assert(err == 0);
      }

      assert(argc == sizeof...(A));

      try {
        fn(env, js_unmarshall_untyped_value<checked, A>(env, argv[I])...);
      } catch (int err) {
        assert(err != 0);
      }

      if constexpr (scoped) {
        err = js_close_handle_scope(env, scope);
        assert(err == 0);
      }

      return js_marshall_untyped_value<checked>(env);
    };
  }

  template <bool scoped, bool checked>
  static inline auto
  create() {
    return create<scoped, checked>(std::index_sequence_for<A...>());
  }
};

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_typed_callback() {
  return js_typed_callback_t<fn>::template create<scoped, checked>();
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_untyped_callback() {
  return js_untyped_callback_t<fn>::template create<scoped, checked>();
}

template <auto fn>
struct js_function_info_t;

template <typename R, typename... A, R fn(A...)>
struct js_function_info_t<fn> {
  using type = js_function_t<R, A...>;

  template <bool scoped, bool checked>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_function_t<R, A...> &result) {
    auto typed = js_typed_callback<fn, scoped, checked>();

    auto untyped = js_untyped_callback<fn, scoped, checked>();

    js_callback_signature_t signature;

    int args[] = {
      js_type_info_t<A>::signature...
    };

    signature.version = 0;
    signature.result = js_type_info_t<R>::signature;
    signature.args_len = sizeof...(A);
    signature.args = args;

    return js_create_typed_function(env, name, len, untyped, &signature, (const void *) typed, nullptr, &result.value);
  }

  template <bool scoped, bool checked>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_handle_t &result) {
    int err;

    js_function_t<R, A...> value;
    err = marshall<scoped, checked>(env, name, len, value);
    if (err < 0) return err;

    result = value;

    return 0;
  }
};

template <typename R, typename... A, R fn(js_env_t *, A...)>
struct js_function_info_t<fn> {
  using type = js_function_t<R, A...>;

  template <bool scoped, bool checked>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_function_t<R, A...> &result) {
    auto typed = js_typed_callback<fn, scoped, checked>();

    auto untyped = js_untyped_callback<fn, scoped, checked>();

    js_callback_signature_t signature;

    int args[] = {
      js_type_info_t<A>::signature...
    };

    signature.version = 0;
    signature.result = js_type_info_t<R>::signature;
    signature.args_len = sizeof...(A);
    signature.args = args;

    return js_create_typed_function(env, name, len, untyped, &signature, (const void *) typed, nullptr, &result.value);
  }

  template <bool scoped, bool checked>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_handle_t &result) {
    int err;

    js_function_t<R, A...> value;
    err = marshall<scoped, checked>(env, name, len, value);
    if (err < 0) return err;

    result = value;

    return 0;
  }
};

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_create_function(js_env_t *env, const char *name, size_t len, typename js_function_info_t<fn>::type &result) {
  return js_function_info_t<fn>::template marshall<scoped, checked>(env, name, len, result);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_create_function(js_env_t *env, const std::string &name, typename js_function_info_t<fn>::type &result) {
  return js_function_info_t<fn>::template marshall<scoped, checked>(env, name.data(), name.size(), result);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_create_function(js_env_t *env, typename js_function_info_t<fn>::type &result) {
  return js_function_info_t<fn>::template marshall<scoped, checked>(env, nullptr, 0, result);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_create_function(js_env_t *env, const char *name, size_t len, js_handle_t &result) {
  return js_function_info_t<fn>::template marshall<scoped, checked>(env, name, len, result);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_create_function(js_env_t *env, std::string name, js_handle_t &result) {
  return js_function_info_t<fn>::template marshall<scoped, checked>(env, name.data(), name.length(), result);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_create_function(js_env_t *env, js_handle_t &result) {
  return js_function_info_t<fn>::template marshall<scoped, checked>(env, nullptr, 0, result);
}

template <bool checked = js_is_debug, typename... A>
static inline auto
js_call_function(js_env_t *env, const js_function_t<void, A...> &function, const A &...args) {
  int err;

  size_t argc = sizeof...(A);

  try {
    js_value_t *argv[] = {
      js_marshall_untyped_value<checked, A>(env, args...)...
    };

    js_value_t *receiver;

    size_t offset = 0;

    if constexpr (js_argument_info_t<A...>::has_receiver) {
      receiver = argv[0];
      offset = 1;
    } else {
      err = js_get_global(env, &receiver);
      assert(err == 0);
    }

    return js_call_function(env, receiver, function.value, argc - offset, &argv[offset], nullptr);
  } catch (int err) {
    return err;
  }
}

template <bool checked = js_is_debug, typename R, typename... A>
static inline auto
js_call_function(js_env_t *env, const js_function_t<R, A...> &function, const A &...args, R &result) {
  int err;

  size_t argc = sizeof...(A);

  try {
    js_value_t *argv[] = {
      js_marshall_untyped_value<checked, A>(env, args...)...
    };

    js_value_t *receiver;

    size_t offset = 0;

    if constexpr (js_argument_info_t<A...>::has_receiver) {
      receiver = argv[0];
      offset = 1;
    } else {
      err = js_get_global(env, &receiver);
      assert(err == 0);
    }

    js_value_t *value;
    err = js_call_function(env, receiver, function.value, argc - offset, &argv[offset], &value);
    if (err < 0) return err;

    result = js_unmarshall_untyped_value<checked, R>(env, value);

    return 0;
  } catch (int err) {
    return err;
  }
}

static inline auto
js_create_object(js_env_t *env, js_object_t &result) {
  return js_create_object(env, &result.value);
}

template <typename... T>
static inline auto
js_create_object(js_env_t *env, js_object_t &result, const js_property_t<T>... properties) {
  int err;
  err = js_create_object(env, result);
  if (err < 0) return err;

  return js_define_properties(env, result, properties...);
}

static inline auto
js_create_array(js_env_t *env, js_array_t &result) {
  return js_create_array(env, &result.value);
}

static inline auto
js_create_array(js_env_t *env, size_t len, js_array_t &result) {
  return js_create_array_with_length(env, len, &result.value);
}

template <typename T, size_t N>
static inline auto
js_create_array(js_env_t *env, const T values[N], js_array_t &result) {
  int err;
  err = js_create_array(env, N, result);
  if (err < 0) return err;

  return js_set_array_elements(env, result, values);
}

template <typename T, size_t N>
static inline auto
js_create_array(js_env_t *env, const std::array<T, N> values, js_array_t &result) {
  int err;
  err = js_create_array(env, N, result);
  if (err < 0) return err;

  return js_set_array_elements(env, result, values);
}

template <typename T>
static inline auto
js_create_array(js_env_t *env, const std::vector<T> values, js_array_t &result) {
  int err;
  err = js_create_array(env, values.size(), result);
  if (err < 0) return err;

  return js_set_array_elements(env, result, values);
}

static inline auto
js_create_bigint(js_env_t *env, int64_t value, js_bigint_t &result) {
  return js_create_bigint_int64(env, value, &result.value);
}

static inline auto
js_create_bigint(js_env_t *env, uint64_t value, js_bigint_t &result) {
  return js_create_bigint_uint64(env, value, &result.value);
}

template <size_t N>
static inline auto
js_create_string(js_env_t *env, const char value[N], js_string_t &result) {
  return js_create_string_utf8(env, value, N, &result.value);
}

static inline auto
js_create_string(js_env_t *env, const utf8_t *value, size_t len, js_string_t &result) {
  return js_create_string_utf8(env, value, len, &result.value);
}

static inline auto
js_create_string(js_env_t *env, const std::string &value, js_string_t &result) {
  return js_create_string_utf8(env, (const utf8_t *) value.data(), value.length(), &result.value);
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, size_t len, T *&data, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, len * sizeof(T), (void **) &data, &result.value);
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, T *&data, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, sizeof(T), (void **) &data, &result.value);
}

static inline auto
js_create_arraybuffer(js_env_t *env, size_t len, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, len, nullptr, &result.value);
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, size_t len, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, len * sizeof(T), nullptr, &result.value);
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, size_t len, std::span<T> &view, js_arraybuffer_t &result) {
  int err;

  T *data;
  err = js_create_arraybuffer(env, len, data, result);
  if (err < 0) return err;

  view = std::span(data, len);

  return 0;
}

template <typename T, size_t N>
static inline auto
js_create_arraybuffer(js_env_t *env, const T data[N], js_arraybuffer_t &result) {
  int err;

  std::span<T> view;
  err = js_create_arraybuffer(env, N, view, result);
  if (err < 0) return err;

  std::copy(data, data + N, view.begin());

  return 0;
}

template <typename T, size_t N>
static inline auto
js_create_arraybuffer(js_env_t *env, const std::array<T, N> data, js_arraybuffer_t &result) {
  int err;

  std::span<T> view;
  err = js_create_arraybuffer(env, N, view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view.begin());

  return 0;
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, const std::span<T> &data, js_arraybuffer_t &result) {
  int err;

  std::span<T> view;
  err = js_create_arraybuffer(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view.begin());

  return 0;
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, const std::vector<T> &data, js_arraybuffer_t &result) {
  int err;

  std::span<T> view;
  err = js_create_arraybuffer(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view.begin());

  return 0;
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, const js_arraybuffer_t &arraybuffer, size_t offset, js_typedarray_t<T> &result) {
  return js_create_typedarray(env, js_typedarray_info_t<T>::type, len, arraybuffer.value, offset, &result.value);
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, const js_arraybuffer_t &arraybuffer, js_typedarray_t<T> &result) {
  return js_create_typedarray(env, len, arraybuffer, 0, result);
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, T *&data, js_typedarray_t<T> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer(env, len, data, arraybuffer);
  if (err < 0) return err;

  return js_create_typedarray(env, len, arraybuffer, result);
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, std::span<T> &view, js_typedarray_t<T> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer(env, len, view, arraybuffer);
  if (err < 0) return err;

  return js_create_typedarray(env, len, arraybuffer, result);
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, js_typedarray_t<T> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer<T>(env, len, arraybuffer);
  if (err < 0) return err;

  return js_create_typedarray(env, len, arraybuffer, result);
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, T *&data, js_typedarray_t<uint8_t> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer(env, data, arraybuffer);
  if (err < 0) return err;

  return js_create_typedarray(env, sizeof(T), arraybuffer, result);
}

template <typename T, size_t N>
static inline auto
js_create_typedarray(js_env_t *env, const T data[N], js_typedarray_t<T> &result) {
  int err;

  std::span<T> view;
  err = js_create_typedarray(env, N, view, result);
  if (err < 0) return err;

  std::copy(data, data + N, view.begin());

  return 0;
}

template <typename T, size_t N>
static inline auto
js_create_typedarray(js_env_t *env, const std::array<T, N> data, js_typedarray_t<T> &result) {
  int err;

  std::span<T> view;
  err = js_create_typedarray(env, N, view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view.begin());

  return 0;
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, const std::span<T> &data, js_typedarray_t<T> &result) {
  int err;

  std::span<T> view;
  err = js_create_typedarray(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view.begin());

  return 0;
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, const std::vector<T> &data, js_typedarray_t<T> &result) {
  int err;

  std::span<T> view;
  err = js_create_typedarray(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view.begin());

  return 0;
}

template <typename T>
static inline auto
js_get_arraybuffer_info(js_env_t *env, const js_arraybuffer_t &arraybuffer, T *&data, size_t &len) {
  int err;
  err = js_get_arraybuffer_info(env, arraybuffer.value, (void **) &data, &len);
  if (err < 0) return err;

  assert(len % sizeof(T) == 0);

  len /= sizeof(T);

  return 0;
}

template <typename T>
static inline auto
js_get_arraybuffer_info(js_env_t *env, const js_arraybuffer_t &arraybuffer, T *&data) {
  int err;

  size_t len;
  err = js_get_arraybuffer_info(env, arraybuffer.value, (void **) &data, &len);
  if (err < 0) return err;

  assert(len == sizeof(T));

  return 0;
}

template <typename T>
static inline auto
js_get_arraybuffer_info(js_env_t *env, const js_arraybuffer_t &arraybuffer, std::span<T> &view) {
  int err;

  T *data;
  size_t len;
  err = js_get_arraybuffer_info(env, arraybuffer, data, len);
  if (err < 0) return err;

  view = std::span(data, len);

  return 0;
}

template <typename T>
static inline auto
js_get_typedarray_info(js_env_t *env, js_typedarray_t<T> &typedarray, T *&data, size_t &len) {
  return js_get_typedarray_info(env, typedarray.value, nullptr, (void **) &data, &len, nullptr, nullptr);
}

template <typename T>
static inline auto
js_get_typedarray_info(js_env_t *env, js_typedarray_t<uint8_t> &typedarray, T *&data) {
  int err;

  size_t len;
  err = js_get_typedarray_info(env, typedarray.value, nullptr, (void **) &data, &len, nullptr, nullptr);
  if (err < 0) return err;

  assert(len == sizeof(T));

  return 0;
}

template <typename T>
static inline auto
js_get_typedarray_info(js_env_t *env, js_typedarray_t<T> &typedarray, std::span<T> &view) {
  int err;

  T *data;
  size_t len;
  err = js_get_typedarray_info(env, typedarray, data, len);
  if (err < 0) return err;

  view = std::span(data, len);

  return 0;
}

static inline auto
js_get_value_bigint(js_env_t *env, const js_bigint_t &bigint, int64_t &result) {
  return js_get_value_bigint_int64(env, bigint.value, &result, nullptr);
}

static inline auto
js_get_value_bigint(js_env_t *env, const js_bigint_t &bigint, int64_t &result, bool &lossless) {
  return js_get_value_bigint_int64(env, bigint.value, &result, &lossless);
}

static inline auto
js_get_value_bigint(js_env_t *env, const js_bigint_t &bigint, uint64_t &result) {
  return js_get_value_bigint_uint64(env, bigint.value, &result, nullptr);
}

static inline auto
js_get_value_bigint(js_env_t *env, const js_bigint_t &bigint, uint64_t &result, bool &lossless) {
  return js_get_value_bigint_uint64(env, bigint.value, &result, &lossless);
}

static inline auto
js_get_value_string(js_env_t *env, const js_string_t &string, std::string &result) {
  int err;

  size_t len;
  err = js_get_value_string_utf8(env, string.value, nullptr, 0, &len);
  if (err < 0) return err;

  result.resize(len);

  return js_get_value_string_utf8(env, string.value, (utf8_t *) result.data(), result.length(), nullptr);
}

static inline auto
js_get_global(js_env_t *env, js_object_t &result) {
  return js_get_global(env, &result.value);
}

static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const js_name_t &name, js_handle_t &result) {
  return js_get_property(env, object.value, name.value, &result.value);
}

static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const char *name, js_handle_t &result) {
  return js_get_named_property(env, object.value, name, &result.value);
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const js_name_t &name, T &result) {
  int err;

  js_value_t *value;
  err = js_get_property(env, object.value, name.value, &value);
  if (err < 0) return err;

  return js_type_info_t<T>::template unmarshall<checked>(env, value, result);
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const char *name, T &result) {
  int err;

  js_value_t *value;
  err = js_get_named_property(env, object.value, name, &value);
  if (err < 0) return err;

  return js_type_info_t<T>::template unmarshall<checked>(env, value, result);
}

static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const js_name_t &name, const js_handle_t &value) {
  return js_set_property(env, object.value, name.value, value.value);
}

static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const char *name, const js_handle_t &value) {
  return js_set_named_property(env, object.value, name, value.value);
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const js_name_t &name, const T &value) {
  int err;

  js_value_t *marshalled;
  err = js_type_info_t<T>::template marshall<checked>(env, value, marshalled);
  if (err < 0) return err;

  return js_set_property(env, object.value, name.value, marshalled);
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const char *name, const T &value) {
  int err;

  js_value_t *marshalled;
  err = js_type_info_t<T>::template marshall<checked>(env, value, marshalled);
  if (err < 0) return err;

  return js_set_named_property(env, object.value, name, marshalled);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const js_name_t &name) {
  int err;

  js_handle_t value;
  err = js_function_info_t<fn>::template marshall<scoped, checked>(env, nullptr, 0, value);
  if (err < 0) return err;

  return js_set_property(env, object, name, value);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const char *name) {
  int err;

  js_handle_t value;
  err = js_function_info_t<fn>::template marshall<scoped, checked>(env, name, -1, value);
  if (err < 0) return err;

  return js_set_named_property(env, object, name, value);
}

static inline auto
js_get_element(js_env_t *env, const js_object_t &object, uint32_t index, js_handle_t &result) {
  return js_get_element(env, object.value, index, &result.value);
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_get_element(js_env_t *env, const js_object_t &object, uint32_t index, T &result) {
  int err;

  js_value_t *value;
  err = js_get_element(env, object.value, index, &value);
  if (err < 0) return err;

  return js_type_info_t<T>::template unmarshall<checked>(env, value, result);
}

static inline auto
js_set_element(js_env_t *env, const js_object_t &object, uint32_t index, const js_handle_t &value) {
  return js_set_element(env, object.value, index, value.value);
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_set_element(js_env_t *env, const js_object_t &object, uint32_t index, const T &value) {
  int err;

  js_value_t *marshalled;
  err = js_type_info_t<T>::template marshall<checked>(env, value, marshalled);
  if (err < 0) return err;

  return js_set_element(env, object.value, index, marshalled);
}

template <auto fn, bool scoped = true, bool checked = js_is_debug>
static inline auto
js_set_element(js_env_t *env, const js_object_t &object, uint32_t index) {
  int err;

  js_handle_t value;
  err = js_function_info_t<fn>::template marshall<scoped, checked>(env, nullptr, 0, value);
  if (err < 0) return err;

  return js_set_element(env, object, index, value);
}

template <bool checked = js_is_debug, typename T, size_t N>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, T result[N]) {
  int err;

  js_value_t *values[N];
  uint32_t len;
  err = js_get_array_elements(env, array.value, values, N, 0, &len);
  if (err < 0) return err;

  assert(len == N);

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template unmarshall<checked>(env, values[i], result[i]);
    if (err < 0) return err;
  }

  return 0;
}

template <bool checked = js_is_debug, typename T, size_t N>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::array<T, N> &result) {
  int err;

  js_value_t *values[N];
  uint32_t len;
  err = js_get_array_elements(env, array.value, values, N, 0, &len);
  if (err < 0) return err;

  assert(len == N);

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template unmarshall<checked>(env, values[i], result[i]);
    if (err < 0) return err;
  }

  return 0;
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::vector<T> &result) {
  int err;

  uint32_t len;
  err = js_get_array_length(env, array.value, &len);
  if (err < 0) return err;

  std::vector<js_value_t *> values(len);
  err = js_get_array_elements(env, array.value, values.data(), len, 0, &len);
  if (err < 0) return err;

  result.resize(len);

  for (uint32_t i = 0; i < len; i++) {
    err = js_type_info_t<T>::template unmarshall<checked>(env, values[i], result[i]);
    if (err < 0) return err;
  }

  return 0;
}

template <bool checked = js_is_debug, typename... T, size_t... I>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::tuple<T...> &result, std::index_sequence<I...>) {
  int err;

  js_value_t *values[sizeof...(T)];
  uint32_t len;
  err = js_get_array_elements(env, array.value, values, sizeof...(T), 0, &len);
  if (err < 0) return err;

  assert(len == sizeof...(T));

  try {
    result = {
      js_unmarshall_untyped_value<checked, T>(env, values[I])...
    };

    return 0;
  } catch (int err) {
    return err;
  }
}

template <bool checked = js_is_debug, typename... T>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::tuple<T...> &result) {
  return js_get_array_elements(env, array, result, std::index_sequence_for<T...>());
}

template <bool checked = js_is_debug, typename T, size_t N>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const T values[N], size_t offset = 0) {
  int err;

  js_value_t *marshalled[N];

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template marshall<checked>(env, values[i], marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, array.value, (const js_value_t **) marshalled, N, offset);
}

template <bool checked = js_is_debug, typename T, size_t N>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::array<T, N> &values, size_t offset = 0) {
  int err;

  js_value_t *marshalled[N];

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template marshall<checked>(env, values[i], marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, array.value, (const js_value_t **) marshalled, N, offset);
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::vector<T> &values, size_t offset = 0) {
  int err;

  auto len = values.size();

  std::vector<js_value_t *> marshalled(len);

  for (uint32_t i = 0; i < len; i++) {
    err = js_type_info_t<T>::template marshall<checked>(env, values[i], marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, array.value, (const js_value_t **) marshalled.data(), len, offset);
}

template <bool checked = js_is_debug, typename... T, size_t... I>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::tuple<T...> &values, size_t offset, std::index_sequence<I...>) {
  try {
    js_value_t *values[] = {
      js_marshall_untyped_value<checked, T>(env, std::get<I>(values))...
    };

    return js_set_array_elements(env, array.value, (const js_value_t **) values, sizeof...(T), offset);
  } catch (int err) {
    return err;
  }
}

template <bool checked = js_is_debug, typename... T>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::tuple<T...> &values, size_t offset = 0) {
  return js_set_array_elements<checked>(env, array, values, offset, std::index_sequence_for<T...>());
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_create_property_descriptor(js_env_t *env, const js_property_t<T> &property, js_property_descriptor_t &result) {
  int err;

  js_property_descriptor_t descriptor;

  descriptor.version = 0;
  descriptor.data = nullptr;
  descriptor.attributes = js_writable | js_enumerable | js_configurable;
  descriptor.method = nullptr;
  descriptor.getter = nullptr;
  descriptor.setter = nullptr;

  const auto &name = property.name;

  err = js_create_string_utf8(env, (const utf8_t *) name.data(), name.length(), &descriptor.name);
  if (err < 0) return err;

  err = js_type_info_t<T>::template marshall<checked>(env, property.value, descriptor.value);
  if (err < 0) return err;

  result = descriptor;

  return 0;
}

template <bool checked = js_is_debug, typename T>
static inline auto
js_create_property_descriptor(js_env_t *env, const js_property_t<T> &property) {
  int err;

  js_property_descriptor_t descriptor;
  err = js_create_property_descriptor<checked>(env, property, descriptor);
  assert(err == 0);

  return descriptor;
}

template <bool checked = js_is_debug, typename... T>
static inline auto
js_define_properties(js_env_t *env, const js_object_t &object, const js_property_t<T>... properties) {
  js_property_descriptor_t descriptors[] = {
    js_create_property_descriptor<checked>(env, properties)...
  };

  return js_define_properties(env, object.value, descriptors, sizeof...(T));
}

static inline auto
js_run_script(js_env_t *env, const char *file, size_t len, int offset, const js_string_t &source, js_handle_t &result) {
  return js_run_script(env, file, len, offset, source.value, &result.value);
}

static inline auto
js_run_script(js_env_t *env, const std::string &file, int offset, const js_string_t &source, js_handle_t &result) {
  return js_run_script(env, file.data(), file.length(), offset, source.value, &result.value);
}

static inline auto
js_run_script(js_env_t *env, const js_string_t &source, js_handle_t &result) {
  return js_run_script(env, nullptr, 0, 0, source.value, &result.value);
}

template <typename T>
static inline auto
js_create_reference(js_env_t *env, const T &value, js_persistent_t<T> &result) {
  result.env = env;

  return js_create_reference(env, value.value, 1, &result.ref);
}

template <typename T>
static inline auto
js_create_weak_reference(js_env_t *env, const T &value, js_persistent_t<T> &result) {
  result.env = env;

  return js_create_reference(env, value.value, 0, &result.ref);
}

template <typename T>
static inline auto
js_get_reference_value(js_env_t *env, const js_persistent_t<T> &reference, T &result) {
  int err;

  js_value_t *value;
  err = js_get_reference_value(env, reference.ref, &value);
  if (err < 0) return err;

  assert(value != nullptr);

  result = T(value);

  return 0;
}

template <typename T>
static inline auto
js_get_reference_value(js_env_t *env, const js_persistent_t<T> &reference, std::optional<T> &result) {
  int err;

  js_value_t *value;
  err = js_get_reference_value(env, reference.ref, &value);
  if (err < 0) return err;

  result = value == nullptr ? std::nullopt : std::optional(T(value));

  return 0;
}

template <typename T>
static inline auto
js_reset_reference(js_env_t *env, js_persistent_t<T> &reference) {
  int err;

  if (reference.ref == nullptr) return 0;

  err = js_delete_reference(env, reference.ref);
  if (err < 0) return err;

  reference.ref = nullptr;

  return 0;
}
