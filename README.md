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

#### `js_external_t<T>`

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

##### `template <js_type_options_t options> int js_type_info_t<T>::marshall(const T &, type &)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is not allowed to call into the JavaScript engine and must be omitted if the value cannot be converted without calling into the JavaScript engine.

##### `template <js_type_options_t options> int js_type_info_t<T>::marshall(js_env_t *, const T &, type &)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is allowed to call into the JavaScript engine. For cases where `js_type_info_t<t>::type` is `js_value_t *` the method will be the same as the method below.

##### `template <js_type_options_t options> int js_type_info_t<T>::marshall(js_env_t *, const T &, js_value_t *&)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is allowed to call into the JavaScript engine.

```cpp
template <>
struct js_type_info_t<struct data> {
  // ...

  template <js_type_options_t options>
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

##### `template <js_type_options_t options> int js_type_info_t<T>::unmarshall(const type &, T &)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is not allowed to call into the JavaScript engine and must be omitted if the value cannot be converted without calling into the JavaScript engine.

##### `template <js_type_options_t options> int js_type_info_t<T>::unmarshall(js_env_t *, const type &, T &)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is allowed to call into the JavaScript engine. For cases where `js_type_info_t<t>::type` is `js_value_t *` the method will be the same as the method below.

##### `template <js_type_options_t options> int js_type_info_t<T>::unmarshall(js_env_t *, const type &, js_value_t *&)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is allowed to call into the JavaScript engine.

```cpp
template <>
struct js_type_info_t<struct data> {
  // ...

  template <js_type_options_t options>
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

`libjstl` comes with a number of builtin `js_type_info_t<T>` implementations to cover the most common JavaScript, C, and C++ types. To add support for your own types, see the [Type marshalling](#type-marshalling) section above.

#### `void`

The `undefined` value in JavaScript. It is only valid as the return type of native functions and any other use of this type is undefined.

#### `bool`

Either `true` or `false` in JavaScript.

#### `int32_t`

A `number` in JavaScript with a valid range of -2,147,483,648 to 2,147,483,647.

#### `uint32_t`

A `number` in JavaScript with a valid range of 0 to 4,294,967,295.

#### `int64_t`

A `number` in JavaScript with a valid range of `Number.MIN_SAFE_INTEGER` (-9,007,199,254,740,991) to `Number.MAX_SAFE_INTEGER` (9,007,199,254,740,991).

#### `double`

A `number` in JavaScript with a valid range of `Number.MIN_VALUE` (5e-324) to `Number.MAX_VALUE` (1.7976931348623157e+308) and a precision of `Number.EPSILON`.

#### `js_bigin64_t`

A `bigint` in JavaScript with a valid range of -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807.

#### `js_biguin64_t`

A `bigint` in JavaScript with a valid range of 0 to 18,446,744,073,709,551,615.

#### `js_arraybuffer_span_t`

The elements of an `ArrayBuffer` in JavaScript. Supports indexing operations such as `y = arrabuffer[x]`, `arraybuffer[x] = y`, and `p = &arraybuffer[x]`.

#### `js_arraybuffer_span_of_t<T>`

The elements of an `ArrayBuffer` in JavaScript interpreted as an array of elements of type `T`. Supports indexing operations such as `y = arrabuffer[x]`, `arraybuffer[x] = y`, and `p = &arraybuffer[x]`.

#### `js_arraybuffer_span_of_t<T, 1>`

The elements of an `ArrayBuffer` in JavaScript interpreted as a single element of type `T`. Supports dereferencing operations such as `value->field` and `*value`.

#### `js_typedarraybuffer_span_t<T>`

The elements of a `TypedArray` in JavaScript that is a view of elements of type `T`. Supports indexing operations such as `y = arrabuffer[x]`, `arraybuffer[x] = y`, and `p = &arraybuffer[x]`.

#### `js_typedarraybuffer_span_t<>`

The elements of a `TypedArray` in JavaScript that is a view of elements of any type.

#### `T *`

An `external` value in JavaScript that is a pointer to an element of type `T`.

#### `utf8_t[N]`

A `string` in JavaScript represented as a UTF-8 `NULL`-terminated C string of `N` characters.

#### `const utf8_t[N]`

A `string` in JavaScript represented as an immutable UTF-8 `NULL`-terminated C string of `N` characters.

#### `utf8_t *`

A `string` in JavaScript represented as a UTF-8 `NULL`-terminated C string.

> [!IMPORTANT]
> When unmarshalling JavaScript strings to `utf8_t *` the caller is responsible for calling `delete[]` when the C string is no longer required.

#### `const utf8_t *`

A `string` in JavaScript represented as an immutable UTF-8 `NULL`-terminated C string.

#### `utf16_t[N]`

A `string` in JavaScript represented as a UTF-16LE `NULL`-terminated C string of `N` characters.

#### `const utf16_t[N]`

A `string` in JavaScript represented as an immutable UTF-16LE `NULL`-terminated C string of `N` characters.

#### `utf16_t *`

A `string` in JavaScript represented as a UTF-16LE `NULL`-terminated C string.

> [!IMPORTANT]
> When unmarshalling JavaScript strings to `utf16_t *` the caller is responsible for calling `delete[]` when the C string is no longer required.

#### `const utf16_t *`

A `string` in JavaScript represented as an immutable UTF-16LE `NULL`-terminated C string.

#### `char[N]`

A `string` in JavaScript represented as a `NULL`-terminated C string of `N` characters.

#### `const char[N]`

A `string` in JavaScript represented as an immutable `NULL`-terminated C string of `N` characters.

#### `char *`

A `string` in JavaScript represented as a `NULL`-terminated C string.

> [!IMPORTANT]
> When unmarshalling JavaScript strings to `char *` the caller is responsible for calling `delete[]` when the C string is no longer required.

#### `const char *`

A `string` in JavaScript represented as an immutable `NULL`-terminated C string.

#### `std::string`

A `string` in JavaScript represented as a C++ string.

#### `T[N]`

An `Array` in JavaScript represented as a C array with `N` elements of type `T`.

#### `const T[N]`

An `Array` in JavaScript represented as a C array with `N` immutable elements of type `T`.

#### `std::array<T, N>`

An `Array` in JavaScript represented as a C++ array with `N` elements.

#### `std::vector<T>`

An `Array` in JavaScript represented as a C++ vector.

#### `std::tuple<T...>`

An `Array` in JavaScript represented as a C++ tuple.

#### `std::span<T>`

The elements of a `TypedArray` in JavaScript that is a view of elements of type `T`, represented as a C++ span.

#### `std::optional<T>`

Either `undefined` or a value of type `T`.

## License

Apache-2.0
