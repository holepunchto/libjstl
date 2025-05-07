# libjstl

C++ template library for <https://github.com/holepunchto/libjs>. It aims to make interacting with the low-level API safer and more efficient without making callers feel too much like they're writing C++.

## API

### Handle types

To improve type safety and ergonomics `libjstl` wraps the `js_value_t *` ABI handle type in a type hierarchy that mirrors the JavaScript type hierarchy. All handle types support conversion to plain `js_value_t *` ABI handles by means of explicit casts:

```cpp
js_boolean_t boolean;

js_value_t *handle = static_cast<js_value_t *>(boolean);
```

For cases where an ABI handle output variable is expected, cast to `js_value_t **` instead:

```cpp
js_boolean_t boolean;

js_value_t **handle = static_cast<js_value_t **>(boolean);
```

#### `js_handle_t`

#### `js_primitive_t`

#### `js_boolean_t`

#### `js_numeric_t`

#### `js_number_t`

#### `js_integer_t`

#### `js_bigint_t`

#### `js_name_t`

#### `js_string_t`

#### `js_symbol_t`

#### `js_object_t`

#### `js_array_t`

#### `js_arraybuffer_t`

#### `js_typedarray_t<T>`

#### `js_typedarray_t<>`

#### `js_function_t<R, A...>`

#### `js_external_t`

### Type marshalling

At the core of `libjstl` is a type marshalling system responsible for converting between ABI types, such as `js_value_t *` handles, and native C/C++ types. Type marshalling is accessed through `js_type_info_t<T>` which describes the type `T` and implements the marshalling and unmarshalling methods responsible for converting `T` to and from its associated ABI type.

#### `js_type_info_t<T>`

```cpp
struct data {
  int32_t foo;
  bool bar;
};

template <>
struct js_type_info_t<struct data> {
  // ...
};
```

##### `js_type_info_t<T>::type`

The ABI type used to represent `T`. Must be `js_value_t *` unless otherwise stated.

```cpp
template <>
struct js_type_info_t<struct data> {
  using type = js_value_t *;

  // ...
};
```

##### `js_type_info_t<T>::signature`

The typed function signature used to represent `T` as a `constexpr` value. Must be a member of `js_value_type_t` unless otherwise stated.

```cpp
template <>
struct js_type_info_t<struct data> {
  // ...

  static constexpr auto signature = js_object;

  // ...
};
```

##### `template <bool checked> int js_type_info_t<T>::marshall(const T &, type &)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is not allowed to call into the JavaScript engine and must be omitted if the value cannot be converted without calling into the JavaScript engine.

##### `template <bool checked> int js_type_info_t<T>::marshall(js_env_t *, const T &, type &)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is allowed to call into the JavaScript engine. For cases where `js_type_info_t<t>::type` is `js_value_t *` the method will be the same as the method below.

##### `template <bool checked> int js_type_info_t<T>::marshall(js_env_t *, const T &, js_value_t *&)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is allowed to call into the JavaScript engine.

```cpp
template <>
struct js_type_info_t<struct data> {
  // ...

  template <bool checked>
  static auto
  marshall(js_env_t *env, struct data value, js_value_t *&result) {
    int err;

    err = js_create_object(env, &result);
    if (err < 0) return err;

    err = js_set_property(env, result, "foo", value.foo);
    if (err < 0) return err;

    err = js_set_property(env, result, "bar", value.bar);
    if (err < 0) return err;

    return 0;
  }

  // ...
};
```

##### `template <bool checked> int js_type_info_t<T>::unmarshall(const type &, T &)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is not allowed to call into the JavaScript engine and must be omitted if the value cannot be converted without calling into the JavaScript engine.

##### `template <bool checked> int js_type_info_t<T>::unmarshall(js_env_t *, const type &, T &)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is allowed to call into the JavaScript engine. For cases where `js_type_info_t<t>::type` is `js_value_t *` the method will be the same as the method below.

##### `template <bool checked> int js_type_info_t<T>::unmarshall(js_env_t *, const type &, js_value_t *&)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is allowed to call into the JavaScript engine.

```cpp
template <>
struct js_type_info_t<struct data> {
  // ...

  template <bool checked>
  static auto
  unmarshall(js_env_t *env, js_value_t *value, struct data &result) {
    int err;

    err = js_get_property(env, value, "foo", result.foo);
    if (err < 0) return err;

    err = js_get_property(env, value, "bar", result.bar);
    if (err < 0) return err;

    return 0;
  }
};
```

### Native functions

#### `js_receiver_t`

### Persistent references

#### `js_persistent_t<T>`

##### `js_persistent_t<T>.reset()`

##### `bool js_persistent_t<T>.empty()`

### Builtin types

#### `void`

#### `bool`

#### `int32_t`

#### `uint32_t`

#### `int64_t`

#### `double`

#### `js_bigin64_t`

#### `js_biguin64_t`

#### `js_arraybuffer_span_t`

#### `js_arraybuffer_span_of_t<T>`

#### `js_arraybuffer_span_of_t<T, 1>`

#### `js_typedarraybuffer_span_t<T>`

#### `js_typedarraybuffer_span_t<>`

#### `T *`

#### `char[N]`

#### `const char[N]`

#### `char *`

#### `const char *`

#### `std::string`

#### `T[N]`

#### `std::array<T, N>`

#### `std::vector<T>`

#### `std::tuple<T...>`

#### `std::span<T>`

#### `std::optional<T>`

## License

Apache-2.0
