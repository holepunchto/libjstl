#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <jstl.h>

template <typename T>
struct js_type_info_t<std::shared_ptr<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::shared_ptr<T> &value, js_value_t *&result) {
    int err;

    auto handle = new std::shared_ptr<T>(value);

    auto finalize = +[](js_env_t *, void *data, void *) {
      delete reinterpret_cast<std::shared_ptr<T> *>(data);
    };

    err = js_create_external(env, reinterpret_cast<void *>(handle), finalize, nullptr, &result);
    if (err < 0) delete handle;

    return err;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::shared_ptr<T> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_external>(env, value, "external");
      if (err < 0) return err;
    }

    std::shared_ptr<T> *handle;
    err = js_get_value_external(env, value, reinterpret_cast<void **>(&handle));
    if (err < 0) return err;

    result = *handle;

    return 0;
  }
};

template <typename T>
struct js_type_info_t<std::unique_ptr<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::unique_ptr<T> &value, js_value_t *&result) {
    int err;

    auto handle = new std::unique_ptr<T>(std::move(value));

    auto finalize = +[](js_env_t *, void *data, void *) {
      delete reinterpret_cast<std::unique_ptr<T> *>(data);
    };

    err = js_create_external(env, reinterpret_cast<void *>(handle), finalize, nullptr, &result);
    if (err < 0) delete handle;

    return err;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::unique_ptr<T> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_external>(env, value, "external");
      if (err < 0) return err;
    }

    std::unique_ptr<T> *handle;
    err = js_get_value_external(env, value, reinterpret_cast<void **>(&handle));
    if (err < 0) return err;

    result = std::move(*handle);

    return 0;
  }
};

template <>
struct js_type_info_t<std::string> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::string &value, js_value_t *&result) {
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

    result.reserve(len + 1 /* NULL */);
    result.resize(len);

    return js_get_value_string_utf8(env, value, reinterpret_cast<utf8_t *>(result.data()), result.capacity(), nullptr);
  }
};

template <>
struct js_type_info_t<std::string_view> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::string_view value, js_value_t *&result) {
    return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value.data()), value.size(), &result);
  }
};

template <>
struct js_type_info_t<std::u16string> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::u16string &value, js_value_t *&result) {
    return js_create_string_utf16le(env, reinterpret_cast<const utf16_t *>(value.data()), value.size(), &result);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::u16string &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_string>(env, value, "string");
      if (err < 0) return err;
    }

    size_t len;
    err = js_get_value_string_utf16le(env, value, nullptr, 0, &len);
    if (err < 0) return err;

    result.resize(len);

    return js_get_value_string_utf16le(env, value, reinterpret_cast<utf16_t *>(result.data()), len, nullptr);
  }
};

template <>
struct js_type_info_t<std::u16string_view> {
  using type = js_value_t *;

  static constexpr auto signature = js_string;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::u16string_view value, js_value_t *&result) {
    return js_create_string_utf16le(env, reinterpret_cast<const utf16_t *>(value.data()), value.size(), &result);
  }
};

template <typename T, size_t N>
struct js_type_info_t<std::array<T, N>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::array<T, N> &array, js_value_t *&result) {
    int err;

    err = js_create_array_with_length(env, N, &result);
    if (err < 0) return err;

    js_value_t *values[N];

    for (uint32_t i = 0; i < N; i++) {
      err = js_type_info_t<T>::template marshall<options>(env, array[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, const_cast<js_value_t *const *>(values), N, 0);
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
  marshall(js_env_t *env, std::vector<T> &vector, js_value_t *&result) {
    int err;

    auto len = vector.size();

    err = js_create_array_with_length(env, len, &result);
    if (err < 0) return err;

    std::vector<js_value_t *> values(len);

    for (uint32_t i = 0; i < len; i++) {
      err = js_type_info_t<T>::template marshall<options>(env, vector[i], values[i]);
      if (err < 0) return err;
    }

    return js_set_array_elements(env, result, const_cast<js_value_t *const *>(values.data()), len, 0);
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
  marshall(js_env_t *env, std::tuple<T...> &tuple, js_value_t *&result, std::index_sequence<I...>) {
    int err;
    err = js_create_array_with_length(env, sizeof...(T), &result);
    if (err < 0) return err;

    js_value_t *values[sizeof...(T)];
    err = js__marshall_untyped_values<options, T...>(env, values, std::get<I>(tuple)...);
    if (err < 0) return err;

    return js_set_array_elements(env, result, values, sizeof...(T), 0);
  }

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::tuple<T...> &tuple, js_value_t *&result) {
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

    if ((... || ((err = js_type_info_t<T>::template unmarshall<options>(env, values[I], std::get<I>(result))) < 0))) return err;

    return 0;
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::tuple<T...> &result) {
    return unmarshall<options>(env, value, result, std::index_sequence_for<T...>());
  }
};

template <typename A, typename B>
struct js_type_info_t<std::pair<A, B>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::pair<A, B> &pair, js_value_t *&result) {
    int err;
    err = js_create_array_with_length(env, 2, &result);
    if (err < 0) return err;

    js_value_t *values[2];
    err = js__marshall_untyped_values<options, A, B>(env, values, pair.first, pair.second);
    if (err < 0) return err;

    return js_set_array_elements(env, result, values, 2, 0);
  }

  template <js_type_options_t options>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, std::pair<A, B> &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_array>(env, value, "array");
      if (err < 0) return err;
    }

    js_value_t *values[2];
    uint32_t len;
    err = js_get_array_elements(env, value, values, 2, 0, &len);
    if (err < 0) return err;

    assert(len == 2);

    err = js_type_info_t<A>::template unmarshall<options>(env, values[0], result.first);
    if (err < 0) return err;

    return js_type_info_t<B>::template unmarshall<options>(env, values[1], result.second);
  }
};

template <typename T>
struct js_type_info_t<std::span<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::span<T> &view, js_value_t *&result) {
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
struct js_type_info_t<std::span<const T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, const std::span<const T> &view, js_value_t *&result) {
    int err;

    js_value_t *arraybuffer;

    T *data;
    err = js_create_arraybuffer(env, view.size_bytes(), reinterpret_cast<void **>(&data), &arraybuffer);
    if (err < 0) return err;

    std::copy(view.begin(), view.end(), data);

    return js_create_typedarray(env, js_typedarray_info_t<T>::type, view.size(), arraybuffer, 0, &result);
  }
};

template <typename T>
struct js_type_info_t<std::optional<T>> {
  using type = js_value_t *;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, std::optional<T> &value, js_value_t *&result) {
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

template <typename D>
struct js_type_info_t<std::chrono::time_point<std::chrono::system_clock, D>> {
  using type = js_value_t *;

  using time_point_t = std::chrono::time_point<std::chrono::system_clock, D>;

  using milliseconds_t = std::chrono::duration<double, std::milli>;

  static constexpr auto signature = js_object;

  template <js_type_options_t options>
  static auto
  marshall(js_env_t *env, time_point_t value, js_value_t *&result) {
    return js_create_date(env, milliseconds_t(value.time_since_epoch()).count(), &result);
  }

  template <js_type_options_t options>
  static int
  unmarshall(js_env_t *env, js_value_t *value, time_point_t &result) {
    int err;

    if constexpr (options.checked) {
      err = js_check_value<js_is_date>(env, value, "date");
      if (err < 0) return err;
    }

    double time;
    err = js_get_value_date(env, value, &time);
    if (err < 0) return err;

    // Converting a time that is NaN or out of range for D is undefined
    // behavior, so it's rejected even when checks are off.
    if (!(time >= milliseconds_t((D::min)()).count() && time <= milliseconds_t((D::max)()).count())) {
      err = js_throw_range_error(env, nullptr, "Date is invalid or out of range");
      assert(err == 0);

      return js_pending_exception;
    }

    result = time_point_t(std::chrono::duration_cast<D>(milliseconds_t(time)));

    return 0;
  }
};

template <auto fn, js_function_options_t options = js_function_options_t()>
static inline auto
js_create_function(js_env_t *env, const std::string &name, typename js_function_info_t<fn>::type &result) {
  return js_function_info_t<fn>::template marshall<options>(env, name.data(), name.size(), result);
}

template <auto fn, js_function_options_t options = js_function_options_t()>
static inline auto
js_create_function(js_env_t *env, std::string name, js_handle_t &result) {
  return js_function_info_t<fn>::template marshall<options>(env, name.data(), name.length(), result);
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
js_create_string(js_env_t *env, const std::string &value, js_string_t &result) {
  return js_create_string_utf8(env, reinterpret_cast<const utf8_t *>(value.data()), value.length(), static_cast<js_value_t **>(result));
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

template <js_typedarray_element T>
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

template <js_typedarray_element T, size_t N>
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

template <js_typedarray_element T>
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

template <js_typedarray_element T>
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
js_get_arraybuffer_info(js_env_t *env, const js_arraybuffer_t &arraybuffer, std::span<T> &view) {
  int err;

  T *data;
  size_t len;
  err = js_get_arraybuffer_info(env, arraybuffer, data, len);
  if (err < 0) return err;

  view = std::span(data, len);

  return 0;
}

template <js_typedarray_element T>
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
js_get_value(js_env_t *env, const js_string_t &string, std::string &result) {
  int err;

  size_t len;
  err = js_get_value_string_utf8(env, static_cast<js_value_t *>(string), nullptr, 0, &len);
  if (err < 0) return err;

  result.reserve(len + 1 /* NULL */);
  result.resize(len);

  return js_get_value_string_utf8(env, static_cast<js_value_t *>(string), reinterpret_cast<utf8_t *>(result.data()), result.capacity(), nullptr);
}

static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const std::string &name, js_handle_t &result) {
  return js_get_named_property(env, static_cast<js_value_t *>(object), name.c_str(), static_cast<js_value_t **>(result));
}

template <js_type_options_t options = js_type_options_t(), typename T>
static inline auto
js_get_property(js_env_t *env, const js_object_t &object, const std::string &name, T &result) {
  return js_get_property<options>(env, object, name.c_str(), result);
}

template <js_type_options_t options = js_type_options_t(), typename T>
static inline auto
js_get_property(js_env_t *env, js_value_t *object, const std::string &name, T &result) {
  return js_get_property<options>(env, object, name.c_str(), result);
}

static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const std::string &name, const js_handle_t &value) {
  return js_set_named_property(env, static_cast<js_value_t *>(object), name.c_str(), static_cast<js_value_t *>(value));
}

template <js_type_options_t options = js_type_options_t(), typename T>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const std::string &name, T value) {
  return js_set_property<options>(env, object, name.c_str(), value);
}

template <js_type_options_t options = js_type_options_t(), typename T>
static inline auto
js_set_property(js_env_t *env, js_value_t *object, const std::string &name, T value) {
  return js_set_property<options>(env, object, name.c_str(), value);
}

template <auto fn, js_function_options_t options = js_function_options_t()>
static inline auto
js_set_property(js_env_t *env, const js_object_t &object, const std::string &name) {
  return js_set_property<fn, options>(env, object, name.c_str());
}

template <auto fn, js_function_options_t options = js_function_options_t()>
static inline auto
js_set_property(js_env_t *env, js_value_t *object, const std::string &name) {
  return js_set_property<fn, options>(env, object, name.c_str());
}

template <js_type_options_t options = js_type_options_t(), typename T, size_t N>
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

template <js_type_options_t options = js_type_options_t(), typename T>
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

template <js_type_options_t options = js_type_options_t(), typename... T, size_t... I>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::tuple<T...> &result, std::index_sequence<I...>) {
  int err;

  js_value_t *values[sizeof...(T)];
  uint32_t len;
  err = js_get_array_elements(env, array, values, sizeof...(T), 0, &len);
  if (err < 0) return err;

  assert(len == sizeof...(T));

  if ((... || ((err = js_type_info_t<T>::template unmarshall<options>(env, values[I], std::get<I>(result))) < 0))) return err;

  return 0;
}

template <js_type_options_t options = js_type_options_t(), typename... T>
static inline auto
js_get_array_elements(js_env_t *env, const js_array_t &array, std::tuple<T...> &result) {
  return js_get_array_elements<options>(env, array, result, std::index_sequence_for<T...>());
}

template <js_type_options_t options = js_type_options_t(), typename T, size_t N>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::array<T, N> &values, size_t offset = 0) {
  int err;

  js_value_t *marshalled[N];

  for (uint32_t i = 0; i < N; i++) {
    err = js_type_info_t<T>::template marshall<options>(env, values[i], marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, static_cast<js_value_t *>(array), const_cast<js_value_t *const *>(marshalled), N, offset);
}

template <js_type_options_t options = js_type_options_t(), typename T>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::vector<T> &values, size_t offset = 0) {
  int err;

  auto len = values.size();

  std::vector<js_value_t *> marshalled(len);

  for (uint32_t i = 0; i < len; i++) {
    auto value = values[i];

    err = js_type_info_t<T>::template marshall<options>(env, value, marshalled[i]);
    if (err < 0) return err;
  }

  return js_set_array_elements(env, static_cast<js_value_t *>(array), const_cast<js_value_t *const *>(marshalled.data()), len, offset);
}

template <js_type_options_t options = js_type_options_t(), typename... T, size_t... I>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::tuple<T...> &values, size_t offset, std::index_sequence<I...>) {
  int err;

  js_value_t *marshalled[sizeof...(T)];
  err = js__marshall_untyped_values<options, T...>(env, marshalled, std::get<I>(values)...);
  if (err < 0) return err;

  return js_set_array_elements(env, static_cast<js_value_t *>(array), marshalled, sizeof...(T), offset);
}

template <js_type_options_t options = js_type_options_t(), typename... T>
static inline auto
js_set_array_elements(js_env_t *env, const js_array_t &array, const std::tuple<T...> &values, size_t offset = 0) {
  return js_set_array_elements<options>(env, array, values, offset, std::index_sequence_for<T...>());
}

static inline auto
js_run_script(js_env_t *env, const std::string &file, int offset, const js_string_t &source, js_handle_t &result) {
  return js_run_script(env, file.data(), file.length(), offset, static_cast<js_value_t *>(source), static_cast<js_value_t **>(result));
}

static inline auto
js_create_error(js_env_t *env, const std::string &message, js_object_t &result) {
  int err;

  js_string_t string;
  err = js_create_string(env, message, string);
  if (err < 0) return err;

  return js_create_error(env, string, result);
}

static inline auto
js_create_error(js_env_t *env, const js_handle_t &code, const std::string &message, js_object_t &result) {
  int err;

  js_string_t string;
  err = js_create_string(env, message, string);
  if (err < 0) return err;

  return js_create_error(env, code, string, result);
}

template <js_type_options_t options = js_type_options_t(), typename T>
static inline auto
js_create_error(js_env_t *env, T code, const std::string &message, js_object_t &result) {
  int err;

  js_string_t string;
  err = js_create_string(env, message, string);
  if (err < 0) return err;

  return js_create_error<options>(env, code, string, result);
}
