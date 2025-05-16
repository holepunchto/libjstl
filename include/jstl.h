#pragma once

#include <array>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <js.h>
#include <stdbool.h>
#include <stdint.h>
#include <utf.h>

#ifndef NDEBUG
constexpr bool js_is_debug = true;
#else
constexpr bool js_is_debug = false;
#endif

template <typename A, typename B>
constexpr bool js_is_same = false;

template <typename A>
constexpr bool js_is_same<A, A> = true;

template <typename T>
concept js_typedarray_element_t =
  js_is_same<T, int8_t> ||
  js_is_same<T, uint8_t> ||
  js_is_same<T, int16_t> ||
  js_is_same<T, uint16_t> ||
  js_is_same<T, int32_t> ||
  js_is_same<T, uint32_t> ||
  js_is_same<T, int64_t> ||
  js_is_same<T, uint64_t> ||
  js_is_same<T, float> ||
  js_is_same<T, double>;

struct js_typedarray_element_any_t;

struct js_handle_t {
  js_handle_t() : value_(nullptr) {}

  explicit js_handle_t(js_value_t *value) : value_(value) {}

  js_handle_t(const js_handle_t &value) : js_handle_t(value.value_) {}

  virtual ~js_handle_t() = default;

  explicit operator js_value_t *() const {
    return value_;
  }

  explicit operator js_value_t **() {
    return &value_;
  }

  bool
  empty() {
    return value_ == nullptr;
  }

private:
  js_value_t *value_;
};

struct js_primitive_t : js_handle_t {
  js_primitive_t() : js_handle_t() {}

  explicit js_primitive_t(js_value_t *value) : js_handle_t(value) {}
};

struct js_boolean_t : js_primitive_t {
  js_boolean_t() : js_primitive_t() {}

  explicit js_boolean_t(js_value_t *value) : js_primitive_t(value) {}
};

struct js_numeric_t : js_primitive_t {
  js_numeric_t() : js_primitive_t() {}

  explicit js_numeric_t(js_value_t *value) : js_primitive_t(value) {}
};

struct js_number_t : js_numeric_t {
  js_number_t() : js_numeric_t() {}

  explicit js_number_t(js_value_t *value) : js_numeric_t(value) {}
};

struct js_integer_t : js_number_t {
  js_integer_t() : js_number_t() {}

  explicit js_integer_t(js_value_t *value) : js_number_t(value) {}
};

struct js_bigint_t : js_numeric_t {
  js_bigint_t() : js_numeric_t() {}

  explicit js_bigint_t(js_value_t *value) : js_numeric_t(value) {}
};

struct js_name_t : js_primitive_t {
  js_name_t() : js_primitive_t() {}

  explicit js_name_t(js_value_t *value) : js_primitive_t(value) {}
};

struct js_string_t : js_name_t {
  js_string_t() : js_name_t() {}

  explicit js_string_t(js_value_t *value) : js_name_t(value) {}
};

struct js_symbol_t : js_name_t {
  js_symbol_t() : js_name_t() {}

  explicit js_symbol_t(js_value_t *value) : js_name_t(value) {}
};

struct js_object_t : js_handle_t {
  js_object_t() : js_handle_t() {}

  explicit js_object_t(js_value_t *value) : js_handle_t(value) {}
};

struct js_array_t : js_object_t {
  js_array_t() : js_object_t() {}

  explicit js_array_t(js_value_t *value) : js_object_t(value) {}
};

struct js_arraybuffer_t : js_object_t {
  js_arraybuffer_t() : js_object_t() {}

  explicit js_arraybuffer_t(js_value_t *value) : js_object_t(value) {}
};

template <typename T = js_typedarray_element_any_t>
struct js_typedarray_t : js_object_t {
  js_typedarray_t() : js_object_t() {}

  explicit js_typedarray_t(js_value_t *value) : js_object_t(value) {}
};

using js_int8array_t = js_typedarray_t<int8_t>;

using js_uint8array_t = js_typedarray_t<uint8_t>;

using js_int16array_t = js_typedarray_t<int16_t>;

using js_uint16array_t = js_typedarray_t<uint16_t>;

using js_int32array_t = js_typedarray_t<int32_t>;

using js_uint32array_t = js_typedarray_t<uint32_t>;

using js_bigint64array_t = js_typedarray_t<int64_t>;

using js_biguint64array_t = js_typedarray_t<uint64_t>;

using js_float32array_t = js_typedarray_t<float>;

using js_float64array_t = js_typedarray_t<double>;

struct js_receiver_t : js_handle_t {
  js_receiver_t() : js_handle_t() {}

  explicit js_receiver_t(js_value_t *value) : js_handle_t(value) {}

  js_receiver_t(const js_handle_t &value) : js_handle_t(value) {}
};

template <typename R, typename... A>
struct js_function_t : js_object_t {
  js_function_t() : js_object_t() {}

  explicit js_function_t(js_value_t *value) : js_object_t(value) {}
};

template <typename T>
struct js_external_t : js_handle_t {
  js_external_t() : js_handle_t() {}

  explicit js_external_t(js_value_t *value) : js_handle_t(value) {}
};

template <typename T>
struct js_persistent_t {
  js_persistent_t() : env_(nullptr), ref_(nullptr) {}

  js_persistent_t(js_env_t *env, js_ref_t *ref) : env_(env), ref_(ref) {};

  js_persistent_t(js_persistent_t &&that) : env_(that.env_), ref_(that.ref_) {
    that.ref_ = nullptr;
  }

  js_persistent_t(const js_persistent_t &) = delete;

  ~js_persistent_t() {
    reset();
  }

  void
  operator=(js_persistent_t &&that) {
    this->env_ = that.env_;
    this->ref_ = that.ref_;

    that.ref_ = nullptr;
  }

  void
  operator=(const js_persistent_t &) = delete;

  explicit operator js_ref_t *() const {
    return ref_;
  }

  void
  reset() {
    if (ref_ == nullptr) return;

    int err;
    err = js_delete_reference(env_, ref_);
    assert(err == 0);

    ref_ = nullptr;
  }

  bool
  empty() {
    return ref_ == nullptr;
  }

private:
  js_env_t *env_;
  js_ref_t *ref_;
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
  err = check(env, js_handle_t(value), is_type);
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

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_int8array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<uint8_t> {
  static constexpr auto type = js_uint8array;

  static constexpr auto label = "uint8array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_uint8array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<int16_t> {
  static constexpr auto type = js_int16array;

  static constexpr auto label = "int16array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_int16array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<uint16_t> {
  static constexpr auto type = js_uint16array;

  static constexpr auto label = "uint16array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_uint16array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<int32_t> {
  static constexpr auto type = js_int32array;

  static constexpr auto label = "int32array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_int32array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<uint32_t> {
  static constexpr auto type = js_uint32array;

  static constexpr auto label = "uint32array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_uint32array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<int64_t> {
  static constexpr auto type = js_bigint64array;

  static constexpr auto label = "bigint64array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_bigint64array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<uint64_t> {
  static constexpr auto type = js_biguint64array;

  static constexpr auto label = "biguint64array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_biguint64array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<float> {
  static constexpr auto type = js_float32array;

  static constexpr auto label = "float32array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_float32array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<double> {
  static constexpr auto type = js_float64array;

  static constexpr auto label = "float64array";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_float64array(env, static_cast<js_value_t *>(value), &result);
  }
};

template <>
struct js_typedarray_info_t<js_typedarray_element_any_t> {
  static constexpr auto label = "typedarray";

  static auto
  is(js_env_t *env, const js_handle_t &value, bool &result) {
    return js_is_typedarray(env, static_cast<js_value_t *>(value), &result);
  }
};

template <typename T = js_typedarray_element_any_t>
static inline auto
js_is_typedarray(js_env_t *env, const js_handle_t &value, bool &result) {
  return js_typedarray_info_t<T>::is(env, value, result);
}

struct js_type_options_t {
  bool checked = js_is_debug;
};

template <typename T>
struct js_type_info_t;

template <>
struct js_type_info_t<void> {
  using type = void;

  static constexpr auto signature = js_undefined;

  template <js_type_options_t options>
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, bool value, bool &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, bool value, js_value_t *&result) {
    return js_get_boolean(env, value, &result);
  }

  static auto
  unmarshall(bool value, bool &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, bool value, bool &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, bool &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, int32_t value, int32_t &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, int32_t value, js_value_t *&result) {
    return js_create_int32(env, value, &result);
  }

  static auto
  unmarshall(int32_t value, int32_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, int32_t value, int32_t &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, int32_t &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, uint32_t value, uint32_t &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, uint32_t value, js_value_t *&result) {
    return js_create_uint32(env, value, &result);
  }

  static auto
  unmarshall(uint32_t value, uint32_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, uint32_t value, uint32_t &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, uint32_t &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, int64_t value, int64_t &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, int64_t value, js_value_t *&result) {
    return js_create_int64(env, value, &result);
  }

  static auto
  unmarshall(int64_t value, int64_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, int64_t value, int64_t &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, int64_t &result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_number>(env, value, "int64");
      if (err < 0) return err;
    }

    return js_get_value_int64(env, value, &result);
  }
};

template <>
struct js_type_info_t<uint64_t> {
  using type = uint64_t;

  static constexpr auto signature = js_uint64;

  static auto
  marshall(uint64_t value, uint64_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, uint64_t value, uint64_t &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, uint64_t value, js_value_t *&result) {
    return js_create_int64(env, value, &result);
  }

  static auto
  unmarshall(uint64_t value, uint64_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, uint64_t value, uint64_t &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, uint64_t &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_number>(env, value, "int64");
      if (err < 0) return err;
    }

    int64_t unmarshalled;
    err = js_get_value_int64(env, value, &unmarshalled);
    if (err < 0) return err;

    result = unmarshalled;

    return 0;
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, double value, double &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, double value, js_value_t *&result) {
    return js_create_double(env, value, &result);
  }

  static auto
  unmarshall(double value, double &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, double value, double &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, double &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_bigint_t &bigint, js_value_t *&result) {
    result = static_cast<js_value_t *>(bigint);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_bigint_t &result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_bigint>(env, value, "bigint");
      if (err < 0) return err;
    }

    result = js_bigint_t(value);

    return 0;
  }
};

struct js_bigint64_t {
  js_bigint64_t() : value_(0) {}

  js_bigint64_t(int64_t value) : value_(value) {}

  operator int64_t() const {
    return value_;
  }

private:
  int64_t value_;
};

template <>
struct js_type_info_t<js_bigint64_t> {
  using type = int64_t;

  static constexpr auto signature = js_bigint64;

  static auto
  marshall(js_bigint64_t value, int64_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, js_bigint64_t value, int64_t &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, js_bigint64_t value, js_value_t *&result) {
    return js_create_bigint_int64(env, value, &result);
  }

  static auto
  unmarshall(int64_t value, js_bigint64_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, int64_t value, js_bigint64_t &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_bigint64_t &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_bigint>(env, value, "bigint");
      if (err < 0) return err;
    }

    int64_t bigint;
    err = js_get_value_bigint_int64(env, value, &bigint, nullptr);
    if (err < 0) return err;

    result = bigint;

    return 0;
  }
};

struct js_biguint64_t {
  js_biguint64_t() : value_(0) {}

  js_biguint64_t(uint64_t value) : value_(value) {}

  operator uint64_t() const {
    return value_;
  }

private:
  uint64_t value_;
};

template <>
struct js_type_info_t<js_biguint64_t> {
  using type = uint64_t;

  static constexpr auto signature = js_biguint64;

  static auto
  marshall(uint64_t value, uint64_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, js_biguint64_t value, uint64_t &result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, js_biguint64_t value, js_value_t *&result) {
    return js_create_bigint_uint64(env, value, &result);
  }

  static auto
  unmarshall(uint64_t value, js_biguint64_t &result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, uint64_t value, js_biguint64_t &result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_biguint64_t &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_bigint>(env, value, "bigint");
      if (err < 0) return err;
    }

    uint64_t bigint;
    err = js_get_value_bigint_uint64(env, value, &bigint, nullptr);
    if (err < 0) return err;

    result = bigint;

    return 0;
  }
};

template <>
struct js_type_info_t<js_string_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_string_t &string, js_value_t *&result) {
    result = static_cast<js_value_t *>(string);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_string_t &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_symbol_t &symbol, js_value_t *&result) {
    result = static_cast<js_value_t *>(symbol);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_symbol_t &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_object_t &object, js_value_t *&result) {
    result = static_cast<js_value_t *>(object);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_object_t &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_array_t &array, js_value_t *&result) {
    result = static_cast<js_value_t *>(array);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_array_t &result) {
    if constexpr (options.checked) {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_arraybuffer_t &arraybuffer, js_value_t *&result) {
    result = static_cast<js_value_t *>(arraybuffer);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_arraybuffer_t &result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_arraybuffer>(env, value, "arraybuffer");
      if (err < 0) return err;
    }

    result = js_arraybuffer_t(value);

    return 0;
  }
};

namespace {

template <typename T>
static T js_arraybuffer_span_nil[1] = {T()};

} // namespace

struct js_arraybuffer_span_t {
  js_arraybuffer_span_t() : data_(js_arraybuffer_span_nil<uint8_t>), size_(0) {}

  js_arraybuffer_span_t(uint8_t *data, size_t len) : data_(len == 0 ? js_arraybuffer_span_nil<uint8_t> : data), size_(len) {}

  uint8_t &
  operator[](size_t i) {
    return data_[i];
  }

  const uint8_t
  operator[](size_t i) const {
    return data_[i];
  }

  uint8_t *
  data() const {
    return data_;
  }

  size_t
  size() const {
    return size_;
  }

  template <typename T>
  size_t
  size() const {
    return size_ / sizeof(T);
  }

  size_t
  size_bytes() const {
    return size_;
  }

  bool
  empty() const {
    return size_ == 0;
  }

  uint8_t *
  begin() const {
    return data_;
  }

  uint8_t *
  end() const {
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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const js_arraybuffer_span_t &view, js_value_t *&result) {
    int err;

    uint8_t *data;
    err = js_create_arraybuffer(env, view.size(), reinterpret_cast<void **>(&data), &result);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_arraybuffer_span_t &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_arraybuffer>(env, value, "arraybuffer");
      if (err < 0) return err;
    }

    uint8_t *data;
    size_t len;
    err = js_get_arraybuffer_info(env, value, reinterpret_cast<void **>(&data), &len);
    if (err < 0) return err;

    result = js_arraybuffer_span_t(data, len);

    return 0;
  }
};

constexpr size_t js_arraybuffer_span_dynamic = -1;

template <typename T, size_t N = js_arraybuffer_span_dynamic>
struct js_arraybuffer_span_of_t;

template <typename T>
struct js_arraybuffer_span_of_t<T, 1> {
  js_arraybuffer_span_of_t() : data_(nullptr) {}

  js_arraybuffer_span_of_t(T *data) : data_(data) {}

  T *
  operator->() const {
    return data_;
  }

  T &
  operator*() {
    return *data_;
  }

  const T &
  operator*() const {
    return *data_;
  }

private:
  T *data_;
};

template <typename T>
js_arraybuffer_span_of_t(T *data) -> js_arraybuffer_span_of_t<T, 1>;

template <typename T>
struct js_arraybuffer_span_of_t<T, js_arraybuffer_span_dynamic> {
  js_arraybuffer_span_of_t() : data_(js_arraybuffer_span_nil<T>), size_(0) {}

  js_arraybuffer_span_of_t(T *data, size_t len) : data_(len == 0 ? js_arraybuffer_span_nil<T> : data), size_(len) {}

  T &
  operator[](size_t i) {
    return data_[i];
  }

  const T
  operator[](size_t i) const {
    return data_[i];
  }

  T *
  data() const {
    return data_;
  }

  size_t
  size() const {
    return size_;
  }

  size_t
  size_bytes() const {
    return size_ * sizeof(T);
  }

  bool
  empty() const {
    return size_ == 0;
  }

  T *
  begin() const {
    return data_;
  }

  T *
  end() const {
    return data_ + size_;
  }

private:
  T *data_;
  size_t size_;
};

template <typename T>
js_arraybuffer_span_of_t(T *data, size_t len) -> js_arraybuffer_span_of_t<T>;

template <typename T>
struct js_type_info_t<js_arraybuffer_span_of_t<T, 1>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const js_arraybuffer_span_of_t<T, 1> &view, js_value_t *&result) {
    int err;

    T *data;
    err = js_create_arraybuffer(env, sizeof(T), reinterpret_cast<void **>(&data), &result);
    if (err < 0) return err;

    *data = *view;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_arraybuffer_span_of_t<T, 1> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_arraybuffer>(env, value, "arraybuffer");
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    err = js_get_arraybuffer_info(env, value, reinterpret_cast<void **>(&data), &len);
    if (err < 0) return err;

    assert(len == sizeof(T));

    result = js_arraybuffer_span_of_t<T, 1>(data);

    return 0;
  }
};

template <typename T>
struct js_type_info_t<js_arraybuffer_span_of_t<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const js_arraybuffer_span_of_t<T> &view, js_value_t *&result) {
    int err;

    T *data;
    err = js_create_arraybuffer(env, view.size_bytes(), reinterpret_cast<void **>(&data), &result);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_arraybuffer_span_of_t<T> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_arraybuffer>(env, value, "arraybuffer");
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    err = js_get_arraybuffer_info(env, value, reinterpret_cast<void **>(&data), &len);
    if (err < 0) return err;

    assert(len % sizeof(T) == 0);

    result = js_arraybuffer_span_of_t<T>(data, len / sizeof(T));

    return 0;
  }
};

template <js_typedarray_element_t T>
struct js_type_info_t<js_typedarray_t<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_typedarray_t<T> &typedarray, js_value_t *&result) {
    result = static_cast<js_value_t *>(typedarray);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_t<T> &result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_typedarray<T>>(env, value, js_typedarray_info_t<T>::label);
      if (err < 0) return err;
    }

    result = js_typedarray_t<T>(value);

    return 0;
  }
};

template <>
struct js_type_info_t<js_typedarray_t<>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_typedarray_t<> &typedarray, js_value_t *&result) {
    result = static_cast<js_value_t *>(typedarray);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_t<> &result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_typedarray<>>(env, value, "typedarray");
      if (err < 0) return err;
    }

    result = js_typedarray_t<>(value);

    return 0;
  }
};

static inline size_t
js_typedarray_element_size(js_typedarray_type_t type) {
  switch (type) {
  case js_int8array:
  case js_uint8array:
  case js_uint8clampedarray:
  default:
    return 1;
  case js_int16array:
  case js_uint16array:
  case js_float16array:
    return 2;
  case js_int32array:
  case js_uint32array:
  case js_float32array:
    return 4;
  case js_bigint64array:
  case js_biguint64array:
  case js_float64array:
    return 8;
  }
}

namespace {

template <typename T>
static T js_typedarray_span_nil[1] = {T()};

}

template <typename T = js_typedarray_element_any_t>
struct js_typedarray_span_t;

template <js_typedarray_element_t T>
struct js_typedarray_span_t<T> {
  js_typedarray_span_t() : data_(js_typedarray_span_nil<T>), size_(0) {}

  js_typedarray_span_t(T *data, size_t len) : data_(len == 0 ? js_typedarray_span_nil<T> : data), size_(len) {}

  T &
  operator[](size_t i) {
    return data_[i];
  }

  const T
  operator[](size_t i) const {
    return data_[i];
  }

  T *
  data() const {
    return data_;
  }

  size_t
  size() const {
    return size_;
  }

  template <typename U>
  size_t
  size() const {
    return size_ * sizeof(T) / sizeof(U);
  }

  size_t
  size_bytes() const {
    return size_ * sizeof(T);
  }

  bool
  empty() const {
    return size_ == 0;
  }

  T *
  begin() const {
    return data_;
  }

  T *
  end() const {
    return data_ + size_;
  }

private:
  T *data_;
  size_t size_;
};

template <js_typedarray_element_t T>
js_typedarray_span_t(T *data, size_t len) -> js_typedarray_span_t<T>;

using js_int8array_span_t = js_typedarray_span_t<int8_t>;

using js_uint8array_span_t = js_typedarray_span_t<uint8_t>;

using js_int16array_span_t = js_typedarray_span_t<int16_t>;

using js_uint16array_span_t = js_typedarray_span_t<uint16_t>;

using js_int32array_span_t = js_typedarray_span_t<int32_t>;

using js_uint32array_span_t = js_typedarray_span_t<uint32_t>;

using js_bigint64array_span_t = js_typedarray_span_t<int64_t>;

using js_biguint64array_span_t = js_typedarray_span_t<uint64_t>;

using js_float32array_span_t = js_typedarray_span_t<float>;

using js_float64array_span_t = js_typedarray_span_t<double>;

template <>
struct js_typedarray_span_t<js_typedarray_element_any_t> {
  js_typedarray_span_t() : data_(nullptr), size_(0) {}

  js_typedarray_span_t(void *data, size_t len, js_typedarray_type_t type) : data_(len == 0 ? nullptr : data), size_(len * js_typedarray_element_size(type)) {}

  template <typename T>
  js_typedarray_span_t(T *data, size_t len) : data_(len == 0 ? nullptr : data), size_(len * sizeof(T)) {}

  template <typename T>
  js_typedarray_span_t(const js_typedarray_span_t<T> &span) : data_(span.data()), size_(span.size_bytes()) {}

  template <typename T = uint8_t>
  T *
  data() const {
    return static_cast<T *>(data_);
  }

  template <typename T = uint8_t>
  size_t
  size() const {
    return size_ / sizeof(T);
  }

  size_t
  size_bytes() const {
    return size_;
  }

  bool
  empty() const {
    return size_ == 0;
  }

  template <typename T = uint8_t>
  T *
  begin() const {
    return data<T>();
  }

  template <typename T = uint8_t>
  T *
  end() const {
    return data<T>() + size<T>();
  }

private:
  void *data_;
  size_t size_;
};

js_typedarray_span_t(void *data, size_t len, js_typedarray_type_t type) -> js_typedarray_span_t<js_typedarray_element_any_t>;

template <typename T>
js_typedarray_span_t(T *data, size_t len) -> js_typedarray_span_t<js_typedarray_element_any_t>;

template <typename T>
struct js_type_info_t<js_typedarray_span_t<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const js_typedarray_span_t<T> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    T *data;
    err = js_create_arraybuffer(env, view.size_bytes(), reinterpret_cast<void **>(&data), &arraybuffer);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return js_create_typedarray(env, js_typedarray_info_t<T>::type, view.size(), arraybuffer, 0, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_span_t<T> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_typedarray<T>>(env, value, js_typedarray_info_t<T>::label);
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    err = js_get_typedarray_info(env, value, nullptr, reinterpret_cast<void **>(&data), &len, nullptr, nullptr);
    if (err < 0) return err;

    result = js_typedarray_span_t(data, len);

    return 0;
  }
};

template <>
struct js_type_info_t<js_typedarray_span_t<>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const js_typedarray_span_t<> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    void *data;
    err = js_create_arraybuffer(env, view.size_bytes(), &data, &arraybuffer);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), static_cast<uint8_t *>(data));

    return js_create_typedarray(env, js_uint8array, view.size_bytes(), arraybuffer, 0, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_span_t<> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_typedarray<>>(env, value, "typedarray");
      if (err < 0) return err;
    }

    void *data;
    size_t len;
    js_typedarray_type_t type;
    err = js_get_typedarray_info(env, value, &type, &data, &len, nullptr, nullptr);
    if (err < 0) return err;

    result = js_typedarray_span_t(data, len, type);

    return 0;
  }
};

constexpr size_t js_typedarray_span_dynamic = -1;

template <typename T, size_t N = js_typedarray_span_dynamic>
struct js_typedarray_span_of_t;

template <typename T>
struct js_typedarray_span_of_t<T, 1> {
  js_typedarray_span_of_t() : data_(nullptr) {}

  js_typedarray_span_of_t(T *data) : data_(data) {}

  T *
  operator->() const {
    return data_;
  }

  T &
  operator*() {
    return *data_;
  }

  const T &
  operator*() const {
    return *data_;
  }

private:
  T *data_;
};

template <typename T>
js_typedarray_span_of_t(T *data) -> js_typedarray_span_of_t<T, 1>;

template <typename T>
struct js_typedarray_span_of_t<T, js_typedarray_span_dynamic> {
  js_typedarray_span_of_t() : data_(js_typedarray_span_nil<T>), size_(0) {}

  js_typedarray_span_of_t(T *data, size_t len) : data_(len == 0 ? js_typedarray_span_nil<T> : data), size_(len) {}

  T &
  operator[](size_t i) {
    return data_[i];
  }

  const T
  operator[](size_t i) const {
    return data_[i];
  }

  T *
  data() const {
    return data_;
  }

  size_t
  size() const {
    return size_;
  }

  size_t
  size_bytes() const {
    return size_ * sizeof(T);
  }

  bool
  empty() const {
    return size_ == 0;
  }

  T *
  begin() const {
    return data_;
  }

  T *
  end() const {
    return data_ + size_;
  }

private:
  T *data_;
  size_t size_;
};

template <typename T>
js_typedarray_span_of_t(T *data, size_t len) -> js_typedarray_span_of_t<T>;

template <typename T>
struct js_type_info_t<js_typedarray_span_of_t<T, 1>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const js_typedarray_span_of_t<T, 1> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    T *data;
    err = js_create_arraybuffer(env, sizeof(T), reinterpret_cast<void **>(&data), &arraybuffer);
    if (err < 0) return err;

    err = js_create_typedarray(env, js_uint8array, sizeof(T), arraybuffer, 0, &result);
    if (err < 0) return err;

    *data = *view;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_span_of_t<T, 1> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_typedarray<>>(env, value, "typedarray");
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    js_typedarray_type_t type;
    err = js_get_typedarray_info(env, value, &type, reinterpret_cast<void **>(&data), &len, nullptr, nullptr);
    if (err < 0) return err;

    len *= js_typedarray_element_size(type);

    assert(len == sizeof(T));

    result = js_typedarray_span_of_t<T, 1>(data);

    return 0;
  }
};

template <typename T>
struct js_type_info_t<js_typedarray_span_of_t<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const js_typedarray_span_of_t<T> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    T *data;
    err = js_create_arraybuffer(env, view.size_bytes(), reinterpret_cast<void **>(&data), &arraybuffer);
    if (err < 0) return err;

    err = js_create_typedarray(env, js_uint8array, view.size_bytes(), arraybuffer, 0, &result);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_typedarray_span_of_t<T> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_typedarray<>>(env, value, "typedarray");
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    js_typedarray_type_t type;
    err = js_get_typedarray_info(env, value, &type, reinterpret_cast<void **>(&data), &len, nullptr, nullptr);
    if (err < 0) return err;

    len *= js_typedarray_element_size(type);
    if (err < 0) return err;

    assert(len % sizeof(T) == 0);

    result = js_typedarray_span_of_t<T>(data, len / sizeof(T));

    return 0;
  }
};

template <>
struct js_type_info_t<js_receiver_t> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  static auto
  marshall(const js_receiver_t &receiver, js_value_t *&result) {
    result = static_cast<js_value_t *>(receiver);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_receiver_t &receiver, js_value_t *&result) {
    return marshall(receiver, result);
  }

  static auto
  unmarshall(js_value_t *value, js_receiver_t &result) {
    result = js_receiver_t(value);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, js_value_t *value, js_receiver_t &result) {
    return unmarshall(value, result);
  }
};

template <typename R, typename... A>
struct js_type_info_t<js_function_t<R, A...>> {
  using type = js_value_t *;

  static constexpr auto signature = js_function;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_function_t<R, A...> &function, js_value_t *&result) {
    result = static_cast<js_value_t *>(function);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_function_t<R, A...> &result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_function>(env, value, "function");
      if (err < 0) return err;
    }

    result = js_function_t<R, A...>(value);

    return 0;
  }
};

template <typename T>
struct js_type_info_t<js_external_t<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_external;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, const js_external_t<T> &external, js_value_t *&result) {
    result = static_cast<js_value_t *>(external);

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, js_external_t<T> &result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_external>(env, value, "external");
      if (err < 0) return err;
    }

    result = js_external_t<T>(value);

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

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *, T *value, T *&result) {
    return marshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, T *value, js_value_t *&result) {
    return js_create_external(env, reinterpret_cast<void *>(value), nullptr, nullptr, &result);
  }

  static auto
  unmarshall(T *value, T *&result) {
    result = value;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *, T *value, T *&result) {
    return unmarshall(value, result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, T *&result) {
    if constexpr (options.checked) {
      int err;
      err = js_check_value<js_is_external>(env, value, "external");
      if (err < 0) return err;
    }

    return js_get_value_external(env, value, reinterpret_cast<void **>(&result));
  }
};

template <size_t N>
struct js_type_info_t<utf8_t[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf8_t value[N], js_value_t *&result) {
    return js_create_string_utf8(env, value, N, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, utf8_t result[N]) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf8(env, value, result, N, &len);
    if (err < 0) return err;

    assert(len == N);

    return 0;
  }
};

template <size_t N>
struct js_type_info_t<const utf8_t[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf8_t value[N], js_value_t *&result) {
    return js_create_string_utf8(env, value, N, &result);
  }
};

template <>
struct js_type_info_t<utf8_t *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf8_t *value, js_value_t *&result) {
    return js_create_string_utf8(env, value, -1, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, utf8_t *&result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (err < 0) return err;

    len += 1 /* NULL */;
    result = new utf8_t[len];

    return js_get_value_string_utf8(env, value, result, len, nullptr);
  }
};

template <>
struct js_type_info_t<const utf8_t *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf8_t *value, js_value_t *&result) {
    return js_create_string_utf8(env, value, -1, &result);
  }
};

template <size_t N>
struct js_type_info_t<utf16_t[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf16_t value[N], js_value_t *&result) {
    return js_create_string_utf16le(env, value, N, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, utf16_t result[N]) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf16le(env, value, result, N, &len);
    if (err < 0) return err;

    assert(len == N);

    return 0;
  }
};

template <size_t N>
struct js_type_info_t<const utf16_t[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf16_t value[N], js_value_t *&result) {
    return js_create_string_utf16le(env, value, N, &result);
  }
};

template <>
struct js_type_info_t<utf16_t *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf16_t *value, js_value_t *&result) {
    return js_create_string_utf16le(env, value, -1, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, utf16_t *&result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf16le(env, value, nullptr, 0, &len);
    if (err < 0) return err;

    len += 1 /* NULL */;
    result = new utf16_t[len];

    return js_get_value_string_utf16le(env, value, result, len, nullptr);
  }
};

template <>
struct js_type_info_t<const utf16_t *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const utf16_t *value, js_value_t *&result) {
    return js_create_string_utf16le(env, value, -1, &result);
  }
};

template <size_t N>
struct js_type_info_t<char[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const char value[N], js_value_t *&result) {
    return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value), N, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, char result[N]) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf8(env, value, reinterpret_cast<utf8_t *>(result), N, &len);
    if (err < 0) return err;

    assert(len == N);

    return 0;
  }
};

template <size_t N>
struct js_type_info_t<const char[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const char value[N], js_value_t *&result) {
    return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value), N, &result);
  }
};

template <>
struct js_type_info_t<char *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const char *value, js_value_t *&result) {
    return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value), -1, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, char *&result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (err < 0) return err;

    len += 1 /* NULL */;
    result = new char[len];

    return js_get_value_string_utf8(env, value, reinterpret_cast<utf8_t *>(result), len, nullptr);
  }
};

template <>
struct js_type_info_t<const char *> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const char *value, js_value_t *&result) {
    return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value), -1, &result);
  }
};

template <>
struct js_type_info_t<std::string> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const std::string &value, js_value_t *&result) {
    return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value.data()), value.length(), &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::string &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (err < 0) return err;

    result.resize(len);

    return js_get_value_string_utf8(env, value, reinterpret_cast<utf8_t *>(result.data()), result.length(), nullptr);
  }
};

template <typename T, size_t N>
struct js_type_info_t<T[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const T array[N], js_value_t *&result) {
    int err;

    err = js_create_array_with_length(env, N, &result);
    assert(err == 0);

    js_value_t *values[N];

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template marshall<options>(env, array[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, const_cast<const js_value_t **>(values), N, 0);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, T result[N]) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    js_value_t *values[N];
    uint32_t len;
    err = js_get_array_elements(env, value, values, N, 0, &len);
    if (err < 0) return err;

    assert(len == N);

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template unmarshall<options>(env, values[i], result[i]);
      if (err < 0) return err;
    }

    return 0;
  }
};

template <typename T, size_t N>
struct js_type_info_t<const T[N]> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const T array[N], js_value_t *&result) {
    int err;

    err = js_create_array_with_length(env, N, &result);
    assert(err == 0);

    js_value_t *values[N];

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template marshall<options>(env, array[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, const_cast<const js_value_t **>(values), N, 0);
  }
};

template <typename T, size_t N>
struct js_type_info_t<std::array<T, N>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const std::array<T, N> &array, js_value_t *&result) {
    int err;

    err = js_create_array_with_length(env, N, &result);
    assert(err == 0);

    js_value_t *values[N];

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template marshall<options>(env, array[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, const_cast<const js_value_t **>(values), N, 0);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::array<T, N> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    js_value_t *values[N];
    uint32_t len;
    err = js_get_array_elements(env, value, values, N, 0, &len);
    if (err < 0) return err;

    assert(len == N);

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template unmarshall<options>(env, values[i], result[i]);
      if (err < 0) return err;
    }

    return 0;
  }
};

template <typename T>
struct js_type_info_t<std::vector<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const std::vector<T> &vector, js_value_t *&result) {
    int err;

    auto len = vector.size();

    err = js_create_array_with_length(env, len, &result);
    assert(err == 0);

    std::vector<js_value_t *> values(len);

    for (uint32_t i = 0; i < len; i++) {
      err = js_type_info_t<T>::template marshall<options>(env, vector[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, const_cast<const js_value_t **>(values.data()), len, 0);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::vector<T> &result) {
    int err;

    if constexpr (options.checked) {
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
      err = js_type_info_t<T>::template unmarshall<options>(env, values[i], result[i]);
      if (err < 0) return err;
    }

    return 0;
  }
};

template <typename... T>
struct js_type_info_t<std::tuple<T...>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options, size_t... I>
  static auto
  marshall(js_env_t *env, const std::tuple<T...> &tuple, js_value_t *&result, std::index_sequence<I...>) {
    int err;
    err = js_create_array_with_length(env, sizeof...(T), &result);
    assert(err == 0);

    try {
      js_value_t *values[] = {
        js_marshall_untyped_value<options, T>(env, std::get<I>(tuple))...
      };

      return js_set_array_elements(env, result, const_cast<const js_value_t **>(values), sizeof...(T), 0);
    } catch (int err) {
      return err;
    }
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const std::tuple<T...> &tuple, js_value_t *&result) {
    return marshall<options>(env, tuple, result, std::index_sequence_for<T...>());
  }

  template <js_type_options_t options, size_t... I>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::tuple<T...> &result, std::index_sequence<I...>) {
    int err;

    if constexpr (options.checked) {
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
        js_unmarshall_untyped_value<options, T>(env, values[I])...
      };

      return 0;
    } catch (int err) {
      return err;
    }
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::tuple<T...> &result) {
    return unmarshall<options>(env, value, result, std::index_sequence_for<T...>());
  }
};

template <typename T>
struct js_type_info_t<std::span<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const std::span<T> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    T *data;
    err = js_create_arraybuffer(env, view.size_bytes(), reinterpret_cast<void **>(&data), &arraybuffer);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return js_create_typedarray(env, js_typedarray_info_t<T>::type, view.size(), arraybuffer, 0, &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::span<T> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_typedarray<T>>(env, value, js_typedarray_info_t<T>::label);
      if (err < 0) return err;
    }

    T *data;
    size_t len;
    err = js_get_typedarray_info(env, value, nullptr, reinterpret_cast<void **>(&data), &len, nullptr, nullptr);
    if (err < 0) return err;

    result = std::span(data, len);

    return 0;
  }
};

template <typename T>
struct js_type_info_t<std::optional<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_type_info_t<T>::signature;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const std::optional<T> &value, js_value_t *&result) {
    if (value) {
      return js_type_info_t<T>::template marshall<options>(env, *value, result);
    }

    return js_get_undefined(env, &result);
  }

  template <js_type_options_t options>
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
      err = js_type_info_t<T>::template unmarshall<options>(env, value, unmarshalled);
      if (err < 0) return err;

      result = std::optional(unmarshalled);
    }

    return 0;
  }
};

template <typename T>
struct js_property_t {
  js_property_t(const std::string &name, T value) : name_(name), value_(value) {}

  template <size_t N>
  js_property_t(const char name[N], T value) : name_(name, N), value_(value) {}

  js_property_t(const char *name, T value) : name_(name), value_(value) {}

  const std::string &
  name() const {
    return name_;
  }

  const T &
  value() const {
    return value_;
  }

private:
  std::string name_;
  T value_;
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

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_marshall_typed_value(js_env_t *env, T value) {
  int err;

  typename js_type_info_t<T>::type result;
  err = js_type_info_t<T>::template marshall<options>(env, value, result);
  if (err < 0) throw err;

  return result;
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_marshall_untyped_value(js_env_t *env, T value) {
  int err;

  js_value_t *result;
  err = js_type_info_t<T>::template marshall<options>(env, value, result);
  if (err < 0) throw err;

  return result;
}

template <js_type_options_t options = js_type_options_t{}>
static inline auto
js_marshall_untyped_value(js_env_t *env) {
  int err;

  js_value_t *result;
  err = js_type_info_t<void>::template marshall<options>(env, result);
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

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_unmarshall_typed_value(js_env_t *env, typename js_type_info_t<T>::type value) {
  int err;

  T result;
  err = js_type_info_t<T>::template unmarshall<options>(env, value, result);
  if (err < 0) throw err;

  return result;
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_unmarshall_untyped_value(js_env_t *env, js_value_t *value) {
  int err;

  T result;
  err = js_type_info_t<T>::template unmarshall<options>(env, value, result);
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
  static constexpr bool has_receiver = js_is_same<T, js_receiver_t>;
};

struct js_function_call_t {
  enum type_t {
    typed,
    untyped
  };

  type_t type;
};

struct js_function_statistics_t {
  auto
  calls() {
    return typed_calls_ + untyped_calls_;
  }

  auto
  calls(js_function_call_t::type_t type) {
    switch (type) {
    case js_function_call_t::typed:
      return typed_calls_;
    case js_function_call_t::untyped:
      return untyped_calls_;
    }
  }

  auto
  event(js_function_call_t call) {
    switch (call.type) {
    case js_function_call_t::typed:
      typed_calls_++;
      break;
    case js_function_call_t::untyped:
      untyped_calls_++;
      break;
    }
  }

private:
  uint64_t typed_calls_ = 0;
  uint64_t untyped_calls_ = 0;
};

struct js_function_options_t : js_type_options_t {
  bool scoped = true;

  js_function_statistics_t *statistics = nullptr;
};

template <auto fn>
struct js_typed_callback_t;

template <typename R, typename... A, R fn(A...)>
struct js_typed_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> typename js_type_info_t<R>::type {
      if constexpr (options.statistics) options.statistics->event({js_function_call_t::typed});

      return js_marshall_typed_value<R>(fn(js_unmarshall_typed_value<A>(args)...));
    };
  }
};

template <typename R, typename... A, R fn(js_env_t *, A...)>
struct js_typed_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    if constexpr (options.scoped) {
      if constexpr (js_is_same<typename js_type_info_t<R>::type, js_value_t *>) {
        return create_with_escapable_scope<options>();
      } else {
        return create_with_scope<options>();
      }
    } else {
      return create_without_scope<options>();
    }
  }

private:
  template <js_function_options_t options>
  static auto
  create_with_scope() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> typename js_type_info_t<R>::type {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::typed});

      js_env_t *env;
      err = js_get_typed_callback_info(info, &env, nullptr);
      assert(err == 0);

      js_handle_scope_t *scope;
      err = js_open_handle_scope(env, &scope);
      assert(err == 0);

      typename js_type_info_t<R>::type result;

      try {
        result = js_marshall_typed_value<static_cast<js_type_options_t>(options), R>(env, fn(env, js_unmarshall_typed_value<static_cast<js_type_options_t>(options), A>(env, args)...));
      } catch (int err) {
        assert(err != 0);
      }

      err = js_close_handle_scope(env, scope);
      assert(err == 0);

      return result;
    };
  }

  template <js_function_options_t options>
  static auto
  create_with_escapable_scope() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> typename js_type_info_t<R>::type {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::typed});

      js_env_t *env;
      err = js_get_typed_callback_info(info, &env, nullptr);
      assert(err == 0);

      js_escapable_handle_scope_t *scope;
      err = js_open_escapable_handle_scope(env, &scope);
      assert(err == 0);

      typename js_type_info_t<R>::type result;

      try {
        result = js_marshall_typed_value<static_cast<js_type_options_t>(options), R>(env, fn(env, js_unmarshall_typed_value<static_cast<js_type_options_t>(options), A>(env, args)...));

        err = js_escape_handle(env, scope, result, &result);
        assert(err == 0);
      } catch (int err) {
        assert(err != 0);
      }

      err = js_close_escapable_handle_scope(env, scope);
      assert(err == 0);

      return result;
    };
  }

  template <js_function_options_t options>
  static auto
  create_without_scope() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> typename js_type_info_t<R>::type {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::typed});

      js_env_t *env;
      err = js_get_typed_callback_info(info, &env, nullptr);
      assert(err == 0);

      return js_marshall_typed_value<static_cast<js_type_options_t>(options), R>(env, fn(env, js_unmarshall_typed_value<static_cast<js_type_options_t>(options), A>(env, args)...));
    };
  }
};

template <typename... A, void fn(A...)>
struct js_typed_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> void {
      if constexpr (options.statistics) options.statistics->event({js_function_call_t::typed});

      fn(js_unmarshall_typed_value<A>(args)...);
    };
  }
};

template <typename... A, void fn(js_env_t *, A...)>
struct js_typed_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    if constexpr (options.scoped) {
      return create_with_scope<options>();
    } else {
      return create_without_scope<options>();
    }
  }

private:
  template <js_function_options_t options>
  static auto
  create_with_scope() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> void {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::typed});

      js_env_t *env;
      err = js_get_typed_callback_info(info, &env, nullptr);
      assert(err == 0);

      js_handle_scope_t *scope;
      err = js_open_handle_scope(env, &scope);
      assert(err == 0);

      try {
        fn(env, js_unmarshall_typed_value<static_cast<js_type_options_t>(options), A>(env, args)...);
      } catch (int err) {
        assert(err != 0);
      }

      err = js_close_handle_scope(env, scope);
      assert(err == 0);
    };
  }

  template <js_function_options_t options>
  static auto
  create_without_scope() {
    return +[](typename js_type_info_t<A>::type... args, js_typed_callback_info_t *info) -> void {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::typed});

      js_env_t *env;
      err = js_get_typed_callback_info(info, &env, nullptr);
      assert(err == 0);

      try {
        fn(env, js_unmarshall_typed_value<static_cast<js_type_options_t>(options), A>(env, args)...);
      } catch (int err) {
        assert(err != 0);
      }
    };
  }
};

template <auto fn>
struct js_untyped_callback_t;

template <typename R, typename... A, R fn(A...)>
struct js_untyped_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    return create<options>(std::index_sequence_for<A...>());
  }

private:
  template <js_function_options_t options, size_t... I>
  static auto
  create(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::untyped});

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
        result = js_marshall_untyped_value<static_cast<js_type_options_t>(options), R>(env, fn(js_unmarshall_untyped_value<static_cast<js_type_options_t>(options), A>(env, argv[I])...));
      } catch (int err) {
        assert(err != 0);
      }

      return result;
    };
  }
};

template <typename R, typename... A, R fn(js_env_t *, A...)>
struct js_untyped_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    if constexpr (options.scoped) {
      return create_with_scope<options>(std::index_sequence_for<A...>());
    } else {
      return create_without_scope<options>(std::index_sequence_for<A...>());
    }
  }

private:
  template <js_function_options_t options, size_t... I>
  static auto
  create_with_scope(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::untyped});

      js_escapable_handle_scope_t *scope;
      err = js_open_escapable_handle_scope(env, &scope);
      assert(err == 0);

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
        result = js_marshall_untyped_value<static_cast<js_type_options_t>(options), R>(env, fn(env, js_unmarshall_untyped_value<static_cast<js_type_options_t>(options), A>(env, argv[I])...));

        err = js_escape_handle(env, scope, result, &result);
        assert(err == 0);
      } catch (int err) {
        assert(err != 0);
      }

      err = js_close_escapable_handle_scope(env, scope);
      assert(err == 0);

      return result;
    };
  }

  template <js_function_options_t options, size_t... I>
  static auto
  create_without_scope(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::untyped});

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
        result = js_marshall_untyped_value<static_cast<js_type_options_t>(options), R>(env, fn(env, js_unmarshall_untyped_value<static_cast<js_type_options_t>(options), A>(env, argv[I])...));
      } catch (int err) {
        assert(err != 0);
      }

      return result;
    };
  }
};

template <typename... A, void fn(A...)>
struct js_untyped_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    return create<options>(std::index_sequence_for<A...>());
  }

private:
  template <js_function_options_t options, size_t... I>
  static auto
  create(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::untyped});

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
        fn(js_unmarshall_untyped_value<static_cast<js_type_options_t>(options), A>(env, argv[I])...);
      } catch (int err) {
        assert(err != 0);
      }

      return js_marshall_untyped_value<static_cast<js_type_options_t>(options)>(env);
    };
  }
};

template <typename... A, void fn(js_env_t *, A...)>
struct js_untyped_callback_t<fn> {
  template <js_function_options_t options>
  static auto
  create() {
    if constexpr (options.scoped) {
      return create_with_scope<options>(std::index_sequence_for<A...>());
    } else {
      return create_without_scope<options>(std::index_sequence_for<A...>());
    }
  }

private:
  template <js_function_options_t options, size_t... I>
  static auto
  create_with_scope(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::untyped});

      js_handle_scope_t *scope;
      err = js_open_handle_scope(env, &scope);
      assert(err == 0);

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
        fn(env, js_unmarshall_untyped_value<static_cast<js_type_options_t>(options), A>(env, argv[I])...);
      } catch (int err) {
        assert(err != 0);
      }

      err = js_close_handle_scope(env, scope);
      assert(err == 0);

      return js_marshall_untyped_value<static_cast<js_type_options_t>(options)>(env);
    };
  }

  template <js_function_options_t options, size_t... I>
  static auto
  create_without_scope(std::index_sequence<I...>) {
    return +[](js_env_t *env, js_callback_info_t *info) -> js_value_t * {
      int err;

      if constexpr (options.statistics) options.statistics->event({js_function_call_t::untyped});

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
        fn(env, js_unmarshall_untyped_value<static_cast<js_type_options_t>(options), A>(env, argv[I])...);
      } catch (int err) {
        assert(err != 0);
      }

      return js_marshall_untyped_value<static_cast<js_type_options_t>(options)>(env);
    };
  }
};

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_typed_callback() {
  return js_typed_callback_t<fn>::template create<options>();
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_untyped_callback() {
  return js_untyped_callback_t<fn>::template create<options>();
}

template <auto fn>
struct js_function_info_t;

template <typename R, typename... A, R fn(A...)>
struct js_function_info_t<fn> {
  using type = js_function_t<R, A...>;

  template <js_function_options_t options>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_function_t<R, A...> &result) {
    int err;

    auto typed = js_create_typed_callback<fn, options>();

    auto untyped = js_create_untyped_callback<fn, options>();

    js_callback_signature_t signature;

    int args[] = {
      js_type_info_t<A>::signature...
    };

    signature.version = 0;
    signature.result = js_type_info_t<R>::signature;
    signature.args_len = sizeof...(A);
    signature.args = args;

    return js_create_typed_function(env, name, len, untyped, &signature, reinterpret_cast<const void *>(typed), nullptr, static_cast<js_value_t **>(result));
  }

  template <js_function_options_t options>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_handle_t &result) {
    int err;

    js_function_t<R, A...> value;
    err = marshall<options>(env, name, len, value);
    if (err < 0) return err;

    result = value;

    return 0;
  }
};

template <typename R, typename... A, R fn(js_env_t *, A...)>
struct js_function_info_t<fn> {
  using type = js_function_t<R, A...>;

  template <js_function_options_t options>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_function_t<R, A...> &result) {
    int err;

    auto typed = js_create_typed_callback<fn, options>();

    auto untyped = js_create_untyped_callback<fn, options>();

    js_callback_signature_t signature;

    int args[] = {
      js_type_info_t<A>::signature...
    };

    signature.version = 0;
    signature.result = js_type_info_t<R>::signature;
    signature.args_len = sizeof...(A);
    signature.args = args;

    return js_create_typed_function(env, name, len, untyped, &signature, reinterpret_cast<const void *>(typed), nullptr, static_cast<js_value_t **>(result));
  }

  template <js_function_options_t options>
  static auto
  marshall(js_env_t *env, const char *name, size_t len, js_handle_t &result) {
    int err;

    js_function_t<R, A...> value;
    err = marshall<options>(env, name, len, value);
    if (err < 0) return err;

    result = value;

    return 0;
  }
};

template <auto fn, typename T = void, typename U = void>
struct js_finalizer_info_t;

template <void fn(js_env_t *)>
struct js_finalizer_info_t<fn> {
  static auto
  create() {
    return +[](js_env_t *env, void *data, void *finalize_hint) -> void {
      fn(env);
    };
  }
};

template <typename T, void fn(js_env_t *, T *)>
struct js_finalizer_info_t<fn, T> {
  static auto
  create() {
    return +[](js_env_t *env, void *data, void *finalize_hint) -> void {
      fn(env, static_cast<T *>(data));
    };
  }
};

template <typename T, typename U, void fn(js_env_t *, T *, U *)>
struct js_finalizer_info_t<fn, T, U> {
  static auto
  create() {
    return +[](js_env_t *env, void *data, void *finalize_hint) -> void {
      fn(env, static_cast<T *>(data), static_cast<U *>(finalize_hint));
    };
  }
};

template <auto fn, typename T = void, typename U = void>
static inline auto
js_create_finalizer() {
  return js_finalizer_info_t<fn, T, U>::create();
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_function(js_env_t *env, const char *name, size_t len, typename js_function_info_t<fn>::type &result) {
  return js_function_info_t<fn>::template marshall<options>(env, name, len, result);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_function(js_env_t *env, const std::string &name, typename js_function_info_t<fn>::type &result) {
  return js_function_info_t<fn>::template marshall<options>(env, name.data(), name.size(), result);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_function(js_env_t *env, typename js_function_info_t<fn>::type &result) {
  return js_function_info_t<fn>::template marshall<options>(env, nullptr, 0, result);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_function(js_env_t *env, const char *name, size_t len, js_handle_t &result) {
  return js_function_info_t<fn>::template marshall<options>(env, name, len, result);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_function(js_env_t *env, std::string name, js_handle_t &result) {
  return js_function_info_t<fn>::template marshall<options>(env, name.data(), name.length(), result);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_create_function(js_env_t *env, js_handle_t &result) {
  return js_function_info_t<fn>::template marshall<options>(env, nullptr, 0, result);
}

template <js_type_options_t options = js_type_options_t{}, typename... A>
static inline auto
js_call_function(js_env_t *env, const js_function_t<void, A...> &function, const A &...args) {
  int err;

  size_t argc = sizeof...(A);

  try {
    js_value_t *argv[] = {
      js_marshall_untyped_value<options, A>(env, args...)...
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

    return js_call_function(env, receiver, static_cast<js_value_t *>(function), argc - offset, &argv[offset], nullptr);
  } catch (int err) {
    return err;
  }
}

template <js_type_options_t options = js_type_options_t{}, typename R, typename... A>
static inline auto
js_call_function(js_env_t *env, const js_function_t<R, A...> &function, const A &...args, R &result) {
  int err;

  size_t argc = sizeof...(A);

  try {
    js_value_t *argv[] = {
      js_marshall_untyped_value<options, A>(env, args...)...
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
    err = js_call_function(env, receiver, static_cast<js_value_t *>(function), argc - offset, &argv[offset], &value);
    if (err < 0) return err;

    result = js_unmarshall_untyped_value<options, R>(env, value);

    return 0;
  } catch (int err) {
    return err;
  }
}

template <js_type_options_t options = js_type_options_t{}, typename... A>
static inline auto
js_call_function_with_checkpoint(js_env_t *env, const js_function_t<void, A...> &function, const A &...args) {
  int err;

  size_t argc = sizeof...(A);

  try {
    js_value_t *argv[] = {
      js_marshall_untyped_value<options, A>(env, args...)...
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

    return js_call_function_with_checkpoint(env, receiver, static_cast<js_value_t *>(function), argc - offset, &argv[offset], nullptr);
  } catch (int err) {
    return err;
  }
}

template <js_type_options_t options = js_type_options_t{}, typename R, typename... A>
static inline auto
js_call_function_with_checkpoint(js_env_t *env, const js_function_t<R, A...> &function, const A &...args, R &result) {
  int err;

  size_t argc = sizeof...(A);

  try {
    js_value_t *argv[] = {
      js_marshall_untyped_value<options, A>(env, args...)...
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
    err = js_call_function_with_checkpoint(env, receiver, static_cast<js_value_t *>(function), argc - offset, &argv[offset], &value);
    if (err < 0) return err;

    result = js_unmarshall_untyped_value<options, R>(env, value);

    return 0;
  } catch (int err) {
    return err;
  }
}

static inline auto
js_create_object(js_env_t *env, js_object_t &result) {
  return js_create_object(env, static_cast<js_value_t **>(result));
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
  return js_create_array(env, static_cast<js_value_t **>(result));
}

static inline auto
js_create_array(js_env_t *env, size_t len, js_array_t &result) {
  return js_create_array_with_length(env, len, static_cast<js_value_t **>(result));
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
  return js_create_bigint_int64(env, value, static_cast<js_value_t **>(result));
}

static inline auto
js_create_bigint(js_env_t *env, uint64_t value, js_bigint_t &result) {
  return js_create_bigint_uint64(env, value, static_cast<js_value_t **>(result));
}

template <size_t N>
static inline auto
js_create_string(js_env_t *env, const char value[N], js_string_t &result) {
  return js_create_string_utf8(env, value, N, static_cast<js_value_t **>(result));
}

static inline auto
js_create_string(js_env_t *env, const utf8_t *value, size_t len, js_string_t &result) {
  return js_create_string_utf8(env, value, len, static_cast<js_value_t **>(result));
}

static inline auto
js_create_string(js_env_t *env, const std::string &value, js_string_t &result) {
  return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value.data()), value.length(), static_cast<js_value_t **>(result));
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, size_t len, T *&data, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, len * sizeof(T), reinterpret_cast<void **>(&data), static_cast<js_value_t **>(result));
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, T *&data, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, sizeof(T), reinterpret_cast<void **>(&data), static_cast<js_value_t **>(result));
}

static inline auto
js_create_arraybuffer(js_env_t *env, size_t len, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, len, nullptr, static_cast<js_value_t **>(result));
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, size_t len, js_arraybuffer_t &result) {
  return js_create_arraybuffer(env, len * sizeof(T), nullptr, static_cast<js_value_t **>(result));
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

  T *view;
  err = js_create_arraybuffer(env, N, view, result);
  if (err < 0) return err;

  std::copy(data, data + N, view);

  return 0;
}

template <typename T, size_t N>
static inline auto
js_create_arraybuffer(js_env_t *env, const std::array<T, N> data, js_arraybuffer_t &result) {
  int err;

  T *view;
  err = js_create_arraybuffer(env, N, view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, const std::span<T> &data, js_arraybuffer_t &result) {
  int err;

  T *view;
  err = js_create_arraybuffer(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <typename T>
static inline auto
js_create_arraybuffer(js_env_t *env, const std::vector<T> &data, js_arraybuffer_t &result) {
  int err;

  T *view;
  err = js_create_arraybuffer(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <typename T>
static inline auto
js_create_external_arraybuffer(js_env_t *env, T *data, size_t len, js_arraybuffer_t &result) {
  return js_create_external_arraybuffer(env, reinterpret_cast<void *>(data), len * sizeof(T), nullptr, nullptr, static_cast<js_value_t **>(result));
}

template <auto finalize, typename T>
static inline auto
js_create_external_arraybuffer(js_env_t *env, T *data, size_t len, js_arraybuffer_t &result) {
  return js_create_external_arraybuffer(env, reinterpret_cast<void *>(data), len * sizeof(T), js_create_finalizer<finalize, T>(), nullptr, static_cast<js_value_t **>(result));
}

template <auto finalize, typename T, typename U>
static inline auto
js_create_external_arraybuffer(js_env_t *env, T *data, size_t len, U *finalize_hint, js_arraybuffer_t &result) {
  return js_create_external_arraybuffer(env, reinterpret_cast<void *>(data), len * sizeof(T), js_create_finalizer<finalize, T, U>(), reinterpret_cast<void *>(finalize_hint), static_cast<js_value_t **>(result));
}

static inline auto
js_detach_arraybuffer(js_env_t *env, const js_arraybuffer_t &arraybuffer) {
  return js_detach_arraybuffer(env, static_cast<js_value_t *>(arraybuffer));
}

template <js_typedarray_element_t T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, const js_arraybuffer_t &arraybuffer, size_t offset, js_typedarray_t<T> &result) {
  return js_create_typedarray(env, js_typedarray_info_t<T>::type, len, static_cast<js_value_t *>(arraybuffer), offset, static_cast<js_value_t **>(result));
}

static inline auto
js_create_typedarray(js_env_t *env, size_t len, const js_arraybuffer_t &arraybuffer, size_t offset, js_typedarray_t<> &result) {
  return js_create_typedarray(env, js_uint8array, len, static_cast<js_value_t *>(arraybuffer), offset, static_cast<js_value_t **>(result));
}

template <js_typedarray_element_t T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, const js_arraybuffer_t &arraybuffer, js_typedarray_t<T> &result) {
  return js_create_typedarray(env, len, arraybuffer, 0, result);
}

static inline auto
js_create_typedarray(js_env_t *env, size_t len, const js_arraybuffer_t &arraybuffer, js_typedarray_t<> &result) {
  return js_create_typedarray(env, len, arraybuffer, 0, result);
}

template <js_typedarray_element_t T>
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
js_create_typedarray(js_env_t *env, size_t len, T *&data, js_typedarray_t<> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer(env, len, data, arraybuffer);
  if (err < 0) return err;

  return js_create_typedarray(env, len * sizeof(T), arraybuffer, result);
}

template <js_typedarray_element_t T>
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
js_create_typedarray(js_env_t *env, size_t len, std::span<T> &view, js_typedarray_t<> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer(env, len, view, arraybuffer);
  if (err < 0) return err;

  return js_create_typedarray(env, len * sizeof(T), arraybuffer, result);
}

template <js_typedarray_element_t T>
static inline auto
js_create_typedarray(js_env_t *env, size_t len, js_typedarray_t<T> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer<T>(env, len, arraybuffer);
  if (err < 0) return err;

  return js_create_typedarray(env, len, arraybuffer, result);
}

static inline auto
js_create_typedarray(js_env_t *env, size_t len, js_typedarray_t<> &result) {
  int err;

  js_arraybuffer_t arraybuffer;
  err = js_create_arraybuffer(env, len, arraybuffer);
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

template <js_typedarray_element_t T, size_t N>
static inline auto
js_create_typedarray(js_env_t *env, const T data[N], js_typedarray_t<T> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, N, view, result);
  if (err < 0) return err;

  std::copy(data, data + N, view);

  return 0;
}

template <typename T, size_t N>
static inline auto
js_create_typedarray(js_env_t *env, const T data[N], js_typedarray_t<> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, N, view, result);
  if (err < 0) return err;

  std::copy(data, data + N, view);

  return 0;
}

template <js_typedarray_element_t T, size_t N>
static inline auto
js_create_typedarray(js_env_t *env, const std::array<T, N> data, js_typedarray_t<T> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, N, view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <typename T, size_t N>
static inline auto
js_create_typedarray(js_env_t *env, const std::array<T, N> data, js_typedarray_t<> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, N, view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <js_typedarray_element_t T>
static inline auto
js_create_typedarray(js_env_t *env, const std::span<T> &data, js_typedarray_t<T> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, const std::span<T> &data, js_typedarray_t<> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <js_typedarray_element_t T>
static inline auto
js_create_typedarray(js_env_t *env, const std::vector<T> &data, js_typedarray_t<T> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <typename T>
static inline auto
js_create_typedarray(js_env_t *env, const std::vector<T> &data, js_typedarray_t<> &result) {
  int err;

  T *view;
  err = js_create_typedarray(env, data.size(), view, result);
  if (err < 0) return err;

  std::copy(data.begin(), data.end(), view);

  return 0;
}

template <typename T>
static inline auto
js_get_arraybuffer_info(js_env_t *env, const js_arraybuffer_t &arraybuffer, T *&data, size_t &len) {
  int err;
  err = js_get_arraybuffer_info(env, static_cast<js_value_t *>(arraybuffer), reinterpret_cast<void **>(&data), &len);
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
  err = js_get_arraybuffer_info(env, static_cast<js_value_t *>(arraybuffer), reinterpret_cast<void **>(&data), &len);
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

template <js_typedarray_element_t T>
static inline auto
js_get_typedarray_info(js_env_t *env, const js_typedarray_t<T> &typedarray, T *&data, size_t &len) {
  return js_get_typedarray_info(env, static_cast<js_value_t *>(typedarray), nullptr, reinterpret_cast<void **>(&data), &len, nullptr, nullptr);
}

template <typename T>
static inline auto
js_get_typedarray_info(js_env_t *env, const js_typedarray_t<> &typedarray, T *&data, size_t &len) {
  int err;

  js_typedarray_type_t type;
  err = js_get_typedarray_info(env, static_cast<js_value_t *>(typedarray), &type, reinterpret_cast<void **>(&data), &len, nullptr, nullptr);
  if (err < 0) return err;

  len = len * js_typedarray_element_size(type) / sizeof(T);

  return 0;
}

template <typename T>
static inline auto
js_get_typedarray_info(js_env_t *env, const js_typedarray_t<uint8_t> &typedarray, T *&data) {
  int err;

  size_t len;
  err = js_get_typedarray_info(env, static_cast<js_value_t *>(typedarray), nullptr, reinterpret_cast<void **>(&data), &len, nullptr, nullptr);
  if (err < 0) return err;

  assert(len == sizeof(T));

  return 0;
}

template <js_typedarray_element_t T>
static inline auto
js_get_typedarray_info(js_env_t *env, const js_typedarray_t<T> &typedarray, std::span<T> &view) {
  int err;

  T *data;
  size_t len;
  err = js_get_typedarray_info(env, typedarray, data, len);
  if (err < 0) return err;

  view = std::span(data, len);

  return 0;
}

template <typename T>
static inline auto
js_get_typedarray_info(js_env_t *env, const js_typedarray_t<> &typedarray, std::span<T> &view) {
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
  return js_get_value_bigint_int64(env, static_cast<js_value_t *>(bigint), &result, nullptr);
}

static inline auto
js_get_value_bigint(js_env_t *env, const js_bigint_t &bigint, int64_t &result, bool &lossless) {
  return js_get_value_bigint_int64(env, static_cast<js_value_t *>(bigint), &result, &lossless);
}

static inline auto
js_get_value_bigint(js_env_t *env, const js_bigint_t &bigint, uint64_t &result) {
  return js_get_value_bigint_uint64(env, static_cast<js_value_t *>(bigint), &result, nullptr);
}

static inline auto
js_get_value_bigint(js_env_t *env, const js_bigint_t &bigint, uint64_t &result, bool &lossless) {
  return js_get_value_bigint_uint64(env, static_cast<js_value_t *>(bigint), &result, &lossless);
}

static inline auto
js_get_value_string(js_env_t *env, const js_string_t &string, std::string &result) {
  int err;

  size_t len;
  err = js_get_value_string_utf8(env, static_cast<js_value_t *>(string), nullptr, 0, &len);
  if (err < 0) return err;

  result.resize(len);

  return js_get_value_string_utf8(env, static_cast<js_value_t *>(string), reinterpret_cast<utf8_t *>(result.data()), result.length(), nullptr);
}

static inline auto
js_get_global(js_env_t *env, js_object_t &result) {
  return js_get_global(env, static_cast<js_value_t **>(result));
}

static inline auto
js_get_global(js_env_t *env, js_receiver_t &result) {
  return js_get_global(env, static_cast<js_value_t **>(result));
}

static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const js_name_t &name, js_handle_t &result) {
  return js_get_property(env, static_cast<js_value_t *>(object), static_cast<js_value_t *>(name), static_cast<js_value_t **>(result));
}

static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const char *name, js_handle_t &result) {
  return js_get_named_property(env, static_cast<js_value_t *>(object), name, static_cast<js_value_t **>(result));
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const js_name_t &name, T &result) {
  int err;

  js_value_t *value;
  err = js_get_property(env, static_cast<js_value_t *>(object), static_cast<js_value_t *>(name), &value);
  if (err < 0) return err;

  return js_type_info_t<T>::template unmarshall<options>(env, value, result);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_get_property(js_env_t *env, js_value_t *object, const js_name_t &name, T &result) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<options>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_get_property<options>(env, unmarshalled, name, result);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const char *name, T &result) {
  int err;

  js_value_t *value;
  err = js_get_named_property(env, static_cast<js_value_t *>(object), name, &value);
  if (err < 0) return err;

  return js_type_info_t<T>::template unmarshall<options>(env, value, result);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_get_property(js_env_t *env, js_value_t *object, const char *name, T &result) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<options>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_get_property<options>(env, unmarshalled, name, result);
}

static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const js_name_t &name, const js_handle_t &value) {
  return js_set_property(env, static_cast<js_value_t *>(object), static_cast<js_value_t *>(name), static_cast<js_value_t *>(value));
}

static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const char *name, const js_handle_t &value) {
  return js_set_named_property(env, static_cast<js_value_t *>(object), name, static_cast<js_value_t *>(value));
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const js_name_t &name, const T &value) {
  int err;

  js_value_t *marshalled;
  err = js_type_info_t<T>::template marshall<options>(env, value, marshalled);
  if (err < 0) return err;

  return js_set_property(env, static_cast<js_value_t *>(object), static_cast<js_value_t *>(name), marshalled);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_set_property(js_env_t *env, js_value_t *object, const js_name_t &name, const T &value) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<options>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_set_property<options>(env, unmarshalled, name, value);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const char *name, const T &value) {
  int err;

  js_value_t *marshalled;
  err = js_type_info_t<T>::template marshall<options>(env, value, marshalled);
  if (err < 0) return err;

  return js_set_named_property(env, static_cast<js_value_t *>(object), name, marshalled);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_set_property(js_env_t *env, js_value_t *object, const char *name, const T &value) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<options>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_set_property<options>(env, unmarshalled, name, value);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const js_name_t &name) {
  int err;

  js_handle_t value;
  err = js_function_info_t<fn>::template marshall<options>(env, nullptr, 0, value);
  if (err < 0) return err;

  return js_set_property(env, object, name, value);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_set_property(js_env_t *env, js_value_t *object, const js_name_t &name) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<static_cast<js_type_options_t>(options)>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_set_property<fn, options>(env, unmarshalled, name);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const char *name) {
  int err;

  js_handle_t value;
  err = js_function_info_t<fn>::template marshall<options>(env, name, -1, value);
  if (err < 0) return err;

  return js_set_named_property(env, static_cast<js_value_t *>(object), name, static_cast<js_value_t *>(value));
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_set_property(js_env_t *env, js_value_t *object, const char *name) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<static_cast<js_type_options_t>(options)>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_set_property<fn, options>(env, unmarshalled, name);
}

static inline auto
js_get_element(js_env_t *env, const js_object_t &object, uint32_t index, js_handle_t &result) {
  return js_get_element(env, static_cast<js_value_t *>(object), index, static_cast<js_value_t **>(result));
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_get_element(js_env_t *env, const js_object_t &object, uint32_t index, T &result) {
  int err;

  js_value_t *value;
  err = js_get_element(env, static_cast<js_value_t *>(object), index, &value);
  if (err < 0) return err;

  return js_type_info_t<T>::template unmarshall<options>(env, value, result);
}

static inline auto
js_set_element(js_env_t *env, const js_object_t &object, uint32_t index, const js_handle_t &value) {
  return js_set_element(env, static_cast<js_value_t *>(object), index, static_cast<js_value_t *>(value));
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_set_element(js_env_t *env, const js_object_t &object, uint32_t index, const T &value) {
  int err;

  js_value_t *marshalled;
  err = js_type_info_t<T>::template marshall<options>(env, value, marshalled);
  if (err < 0) return err;

  return js_set_element(env, static_cast<js_value_t *>(object), index, marshalled);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_set_element(js_env_t *env, js_value_t *object, uint32_t index, const T &value) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<options>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_set_element<options>(env, unmarshalled, index, value);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_set_element(js_env_t *env, const js_object_t &object, uint32_t index) {
  int err;

  js_handle_t value;
  err = js_function_info_t<fn>::template marshall<options>(env, nullptr, 0, value);
  if (err < 0) return err;

  return js_set_element(env, object, index, value);
}

template <auto fn, js_function_options_t options = js_function_options_t{}>
static inline auto
js_set_element(js_env_t *env, js_value_t *object, uint32_t index) {
  int err;

  js_object_t unmarshalled;
  err = js_type_info_t<js_object_t>::template unmarshall<static_cast<js_type_options_t>(options)>(env, object, unmarshalled);
  if (err < 0) return err;

  return js_set_element<fn, options>(env, unmarshalled, index);
}

template <js_type_options_t options = js_type_options_t{}, typename T, size_t N>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, T result[N]) {
  int err;

  js_value_t *values[N];
  uint32_t len;
  err = js_get_array_elements(env, array, values, N, 0, &len);
  if (err < 0) return err;

  assert(len == N);

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template unmarshall<options>(env, values[i], result[i]);
    if (err < 0) return err;
  }

  return 0;
}

template <js_type_options_t options = js_type_options_t{}, typename T, size_t N>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::array<T, N> &result) {
  int err;

  js_value_t *values[N];
  uint32_t len;
  err = js_get_array_elements(env, array, values, N, 0, &len);
  if (err < 0) return err;

  assert(len == N);

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template unmarshall<options>(env, values[i], result[i]);
    if (err < 0) return err;
  }

  return 0;
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::vector<T> &result) {
  int err;

  uint32_t len;
  err = js_get_array_length(env, static_cast<js_value_t *>(array), &len);
  if (err < 0) return err;

  std::vector<js_value_t *> values(len);
  err = js_get_array_elements(env, static_cast<js_value_t *>(array), values.data(), len, 0, &len);
  if (err < 0) return err;

  result.resize(len);

  for (uint32_t i = 0; i < len; i++) {
    err = js_type_info_t<T>::template unmarshall<options>(env, values[i], result[i]);
    if (err < 0) return err;
  }

  return 0;
}

template <js_type_options_t options = js_type_options_t{}, typename... T, size_t... I>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::tuple<T...> &result, std::index_sequence<I...>) {
  int err;

  js_value_t *values[sizeof...(T)];
  uint32_t len;
  err = js_get_array_elements(env, array, values, sizeof...(T), 0, &len);
  if (err < 0) return err;

  assert(len == sizeof...(T));

  try {
    result = {
      js_unmarshall_untyped_value<options, T>(env, values[I])...
    };

    return 0;
  } catch (int err) {
    return err;
  }
}

template <js_type_options_t options = js_type_options_t{}, typename... T>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::tuple<T...> &result) {
  return js_get_array_elements<options>(env, array, result, std::index_sequence_for<T...>());
}

template <js_type_options_t options = js_type_options_t{}, typename T, size_t N>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const T values[N], size_t offset = 0) {
  int err;

  js_value_t *marshalled[N];

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template marshall<options>(env, values[i], marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, static_cast<js_value_t *>(array), const_cast<const js_value_t **>(marshalled), N, offset);
}

template <js_type_options_t options = js_type_options_t{}, typename T, size_t N>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::array<T, N> &values, size_t offset = 0) {
  int err;

  js_value_t *marshalled[N];

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template marshall<options>(env, values[i], marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, static_cast<js_value_t *>(array), const_cast<const js_value_t **>(marshalled), N, offset);
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::vector<T> &values, size_t offset = 0) {
  int err;

  auto len = values.size();

  std::vector<js_value_t *> marshalled(len);

  for (uint32_t i = 0; i < len; i++) {
    err = js_type_info_t<T>::template marshall<options>(env, values[i], marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, static_cast<js_value_t *>(array), const_cast<const js_value_t **>(marshalled.data()), len, offset);
}

template <js_type_options_t options = js_type_options_t{}, typename... T, size_t... I>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::tuple<T...> &values, size_t offset, std::index_sequence<I...>) {
  try {
    js_value_t *values[] = {
      js_marshall_untyped_value<options, T>(env, std::get<I>(values))...
    };

    return js_set_array_elements(env, static_cast<js_value_t *>(array), const_cast<const js_value_t **>(values), sizeof...(T), offset);
  } catch (int err) {
    return err;
  }
}

template <js_type_options_t options = js_type_options_t{}, typename... T>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::tuple<T...> &values, size_t offset = 0) {
  return js_set_array_elements<options>(env, array, values, offset, std::index_sequence_for<T...>());
}

template <js_type_options_t options = js_type_options_t{}, typename T>
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

  const auto &name = property.name();

  err = js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(name.data()), name.length(), &descriptor.name);
  if (err < 0) return err;

  err = js_type_info_t<T>::template marshall<options>(env, property.value(), descriptor.value);
  if (err < 0) return err;

  result = descriptor;

  return 0;
}

template <js_type_options_t options = js_type_options_t{}, typename T>
static inline auto
js_create_property_descriptor(js_env_t *env, const js_property_t<T> &property) {
  int err;

  js_property_descriptor_t descriptor;
  err = js_create_property_descriptor<options>(env, property, descriptor);
  if (err < 0) throw err;

  return descriptor;
}

template <js_type_options_t options = js_type_options_t{}, typename... T>
static inline auto
js_define_properties(js_env_t *env, const js_object_t &object, const js_property_t<T>... properties) {
  try {
    js_property_descriptor_t descriptors[] = {
      js_create_property_descriptor<options>(env, properties)...
    };

    return js_define_properties(env, static_cast<js_value_t *>(object), descriptors, sizeof...(T));
  } catch (int err) {
    return err;
  }
}

static inline auto
js_run_script(js_env_t *env, const char *file, size_t len, int offset, const js_string_t &source, js_handle_t &result) {
  return js_run_script(env, file, len, offset, static_cast<js_value_t *>(source), static_cast<js_value_t **>(result));
}

static inline auto
js_run_script(js_env_t *env, const std::string &file, int offset, const js_string_t &source, js_handle_t &result) {
  return js_run_script(env, file.data(), file.length(), offset, static_cast<js_value_t *>(source), static_cast<js_value_t **>(result));
}

static inline auto
js_run_script(js_env_t *env, const js_string_t &source, js_handle_t &result) {
  return js_run_script(env, nullptr, 0, 0, static_cast<js_value_t *>(source), static_cast<js_value_t **>(result));
}

template <typename T>
static inline auto
js_create_reference(js_env_t *env, const T &value, js_persistent_t<T> &result) {
  int err;

  js_ref_t *ref;
  err = js_create_reference(env, static_cast<js_value_t *>(value), 1, &ref);
  if (err < 0) return err;

  result = js_persistent_t<T>(env, ref);

  return 0;
}

template <typename T>
static inline auto
js_create_weak_reference(js_env_t *env, const T &value, js_persistent_t<T> &result) {
  int err;

  js_ref_t *ref;
  err = js_create_reference(env, static_cast<js_value_t *>(value), 0, &ref);
  if (err < 0) return err;

  result = js_persistent_t<T>(env, ref);

  return 0;
}

template <typename T>
static inline auto
js_get_reference_value(js_env_t *env, const js_persistent_t<T> &reference, T &result) {
  int err;

  js_value_t *value;
  err = js_get_reference_value(env, static_cast<js_ref_t *>(reference), &value);
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
  err = js_get_reference_value(env, static_cast<js_ref_t *>(reference), &value);
  if (err < 0) return err;

  result = value == nullptr ? std::nullopt : std::optional(T(value));

  return 0;
}

template <typename T>
static inline auto
js_wrap(js_env_t *env, const js_object_t &object, T *data) {
  return js_wrap(env, static_cast<js_value_t *>(object), reinterpret_cast<void *>(data), nullptr, nullptr, nullptr);
}

template <auto finalize, typename T>
static inline auto
js_wrap(js_env_t *env, const js_object_t &object, T *data) {
  return js_wrap(env, static_cast<js_value_t *>(object), reinterpret_cast<void *>(data), js_create_finalizer<finalize, T>(), nullptr, nullptr);
}

template <auto finalize, typename T, typename U>
static inline auto
js_wrap(js_env_t *env, const js_object_t &object, T *data, U *finalize_hint) {
  return js_wrap(env, static_cast<js_value_t *>(object), reinterpret_cast<void *>(data), js_create_finalizer<finalize, T, U>(), reinterpret_cast<void *>(finalize_hint), nullptr);
}

template <typename T>
static inline auto
js_unwrap(js_env_t *env, const js_object_t &object, T *&result) {
  return js_unwrap(env, static_cast<js_value_t *>(object), reinterpret_cast<void **>(&result));
}

template <typename T>
static inline auto
js_remove_wrap(js_env_t *env, const js_object_t &object) {
  return js_remove_wrap(env, static_cast<js_value_t *>(object), nullptr);
}

template <typename T>
static inline auto
js_remove_wrap(js_env_t *env, const js_object_t &object, T *&result) {
  return js_remove_wrap(env, static_cast<js_value_t *>(object), reinterpret_cast<void **>(&result));
}

template <typename T>
static inline auto
js_create_external(js_env_t *env, T *data, js_external_t<T> &result) {
  return js_create_external(env, data, nullptr, nullptr, static_cast<js_value_t **>(result));
}

template <auto finalize, typename T>
static inline auto
js_create_external(js_env_t *env, T *data, js_external_t<T> &result) {
  return js_create_external(env, data, js_create_finalizer<finalize, T>(), nullptr, static_cast<js_value_t **>(result));
}

template <auto finalize, typename T, typename U>
static inline auto
js_create_external(js_env_t *env, T *data, U *finalize_hint, js_external_t<T> &result) {
  return js_create_external(env, data, js_create_finalizer<finalize, T, U>(), reinterpret_cast<void *>(finalize_hint), static_cast<js_value_t **>(result));
}

template <typename T>
static inline auto
js_get_value_external(js_env_t *env, const js_external_t<T> &external, T *&result) {
  return js_get_value_external(env, static_cast<js_value_t *>(external), reinterpret_cast<void **>(&result));
}

template <auto finalize>
static inline auto
js_add_finalizer(js_env_t *env, const js_object_t &object) {
  return js_add_finalizer(env, static_cast<js_value_t *>(object), nullptr, js_create_finalizer<finalize>(), nullptr, nullptr);
}

template <auto finalize, typename T>
static inline auto
js_add_finalizer(js_env_t *env, const js_object_t &object, T *data) {
  return js_add_finalizer(env, static_cast<js_value_t *>(object), reinterpret_cast<void *>(data), js_create_finalizer<finalize, T>(), nullptr, nullptr);
}

template <auto finalize, typename T, typename U>
static inline auto
js_add_finalizer(js_env_t *env, const js_object_t &object, T *data, U *finalize_hint) {
  return js_add_finalizer(env, static_cast<js_value_t *>(object), reinterpret_cast<void *>(data), js_create_finalizer<finalize, T, U>(), reinterpret_cast<void *>(finalize_hint), nullptr);
}
