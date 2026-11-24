# libjstl

C++ template library for <https://github.com/holepunchto/libjs>. It aims to make interacting with the low-level API safer and more efficient without making callers feel too much like they're writing C++.

## Usage

`libjstl` ships two headers:

- `<jstl.h>` is the core of the library. It doesn't use the C++ standard library or need the C++ runtime, so it can be used in code that is compiled with `-fno-exceptions -fno-rtti` and linked without a C++ standard library, such as on FreeRTOS. When exceptions are enabled, native functions may throw exceptions, which are rethrown as JavaScript errors.
- `<jstl/std.h>` includes `<jstl.h>` and adds support for types from the C++ standard library, such as `std::string`, `std::vector`, and `std::span`.

When exceptions are disabled, native functions report failures the same way as the C API, by making an exception pending and returning.

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

Handle types are thin wrappers around a `js_value_t *` and are as cheap to copy as the pointer itself. Like the ABI handles they wrap, they are only valid within the handle scope they were created in. To keep a value alive beyond that, create a reference with `js_persistent_t<T>`.

A handle type converts implicitly to any of its base types, so a `js_string_t` can be passed where a `js_name_t` or a `js_handle_t` is expected. Going the other way requires an explicit conversion, which does not check the type of the value:

```cpp
js_string_t string;

js_handle_t handle = string;

js_string_t same = js_string_t(static_cast<js_value_t *>(handle));
```

Handle types that support type marshalling can be used as the arguments and results of native functions and as property and element values. When unmarshalling such a handle from JavaScript, the type of the value is checked if the `checked` type option is set, which is the default in debug builds. A failed check throws a `TypeError` in JavaScript.

#### `js_handle_t`

The base of all handle types, which holds a value of any type. A default constructed handle is empty, which `empty()` and the `bool` conversion report. Unmarshalling a `js_handle_t` never checks the type of the value.

#### `js_primitive_t`

A primitive value in JavaScript, such as a `boolean`, `number`, `bigint`, `string`, or `symbol`. Does not support type marshalling.

#### `js_boolean_t`

A `boolean` in JavaScript. Does not support type marshalling, so use `bool` for native function arguments and results. Read its value with `js_get_value()`.

#### `js_numeric_t`

A `number` or `bigint` in JavaScript. Does not support type marshalling.

#### `js_number_t`

A `number` in JavaScript. Does not support type marshalling, so use `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, or `double` for native function arguments and results. Read its value with `js_get_value()`.

#### `js_integer_t`

A `number` in JavaScript that holds an integer. Does not support type marshalling.

#### `js_bigint_t`

A `bigint` in JavaScript. Create one with `js_create_bigint()` and read its value with `js_get_value()`, which can also report whether the conversion was lossless. Checked to be a `bigint` when unmarshalled.

#### `js_name_t`

A value in JavaScript that can name a property, that is a `string` or a `symbol`. Accepted as the property name by `js_get_property()` and `js_set_property()`. Does not support type marshalling.

#### `js_string_t`

A `string` in JavaScript. Create one with `js_create_string()`. Checked to be a `string` when unmarshalled.

#### `js_symbol_t`

A `symbol` in JavaScript. Checked to be a `symbol` when unmarshalled.

#### `js_object_t`

An `object` in JavaScript. Create one with `js_create_object()`, optionally with a list of properties given as `js_property_t` values. Most object operations take a `js_object_t`, such as `js_get_property()`, `js_set_property()`, `js_wrap()`, and `js_add_finalizer()`. Checked to be an `object` when unmarshalled.

> [!NOTE]
> As in JavaScript, where `typeof null` is `"object"`, `js_get_null()` returns `null` as a `js_object_t`.

#### `js_array_t`

An `Array` in JavaScript. Create one with `js_create_array()` and access its elements with `js_get_array_elements()` and `js_set_array_elements()`. Checked to be an `Array` when unmarshalled.

#### `js_arraybuffer_t`

An `ArrayBuffer` in JavaScript. Create one with `js_create_arraybuffer()` or `js_create_external_arraybuffer()`, and access its memory with `js_get_arraybuffer_info()`. Checked to be an `ArrayBuffer` when unmarshalled.

#### `js_typedarray_t<T>`

A `TypedArray` in JavaScript with elements of type `T`, which must be one of `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `float`, or `double`. Aliases are provided for each, such as `js_uint8array_t` for `js_typedarray_t<uint8_t>` and `js_bigint64array_t` for `js_typedarray_t<int64_t>`. Create one with `js_create_typedarray()` and access its elements with `js_get_typedarray_info()`. Checked to be a `TypedArray` of the matching kind when unmarshalled.

#### `js_typedarray_t<>`

A `TypedArray` in JavaScript of any kind. The elements can be accessed as any type with `js_get_typedarray_info()`, in which case the length is given in elements of that type. Checked to be a `TypedArray` when unmarshalled.

#### `js_function_t<R, A...>`

A `function` in JavaScript that takes arguments of types `A...` and returns a result of type `R`. Create one from a native function with `js_create_function()`, which derives `R` and `A...` from the signature of the native function, and call one with `js_call_function()`, which marshalls the arguments and unmarshalls the result. Only checked to be a `function` when unmarshalled, as the parameter types of a JavaScript function cannot be checked.

#### `js_external_t<T>`

An `external` value in JavaScript that holds a pointer to a value of type `T`. Create one with `js_create_external()`, optionally with a finalizer, and read the pointer with `js_get_value()`. Checked to be an `external` when unmarshalled, but the type of the pointer is not checked.

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

##### `template <js_type_options_t options> int js_type_info_t<T>::marshall(T &, type &)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is not allowed to call into the JavaScript engine and must be omitted if the value cannot be converted without calling into the JavaScript engine.

##### `template <js_type_options_t options> int js_type_info_t<T>::marshall(js_env_t *, T &, type &)`

Convert a value of type `T` to the ABI type indiciated by `js_type_info_t<t>::type`. The method is allowed to call into the JavaScript engine. For cases where `js_type_info_t<t>::type` is `js_value_t *` the method will be the same as the method below.

##### `template <js_type_options_t options> int js_type_info_t<T>::marshall(js_env_t *, T &, js_value_t *&)`

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

##### `template <js_type_options_t options> int js_type_info_t<T>::unmarshall(type &, T &)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is not allowed to call into the JavaScript engine and must be omitted if the value cannot be converted without calling into the JavaScript engine.

##### `template <js_type_options_t options> int js_type_info_t<T>::unmarshall(js_env_t *, type &, T &)`

Convert a value of the ABI type indiciated by `js_type_info_t<t>::type` to the type `T`. The method is allowed to call into the JavaScript engine. For cases where `js_type_info_t<t>::type` is `js_value_t *` the method will be the same as the method below.

##### `template <js_type_options_t options> int js_type_info_t<T>::unmarshall(js_env_t *, type &, js_value_t *&)`

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

A C or C++ function can be exposed to JavaScript as long as its result and each of its arguments support type marshalling. The result may also be `void`. The function may take a `js_env_t *` as its first argument, which lets it call into JavaScript, and is otherwise only given its marshalled arguments:

```cpp
static int32_t
add(int32_t a, int32_t b) noexcept {
  return a + b;
}

static js_object_t
point(js_env_t *env, double x, double y) {
  js_object_t result;
  js_create_object(env, result, js_property_t("x", x), js_property_t("y", y));

  return result;
}
```

The function is passed as a template argument, so the glue that converts between JavaScript and native values is generated at compile time and needs no allocation or state at runtime.

Each native function has two entry points. By default, calls go through an untyped entry point that receives the arguments as `js_value_t *` handles and unmarshalls them. Once the engine has optimized a call site, it may instead call a typed entry point that receives the arguments as their ABI types, such as `int32_t`, which skips most of the conversion. Both entry points behave the same, and `js_function_statistics_t` reports which one was used.

> [!NOTE]
> A native function must be called with the number of arguments it declares, which is asserted in debug builds. To leave out an argument of type `std::optional<T>`, pass `undefined` explicitly.

#### Errors

A native function reports a failure the same way as the C API: it makes a JavaScript exception pending, such as with `js_throw_error()`, and returns. The result it returns is then ignored. Failures in marshalling the arguments or the result are reported in the same way, in which case the native function is not called, or its result is dropped.

When exceptions are enabled, a native function that is not declared `noexcept` may also throw a C++ exception, which is rethrown in JavaScript:

| C++ exception           | JavaScript error                                    |
| ----------------------- | --------------------------------------------------- |
| `std::invalid_argument` | `TypeError` with the message of `what()`            |
| `std::out_of_range`     | `RangeError` with the message of `what()`           |
| `std::exception`        | `Error` with the message of `what()`                |
| Any other exception     | `Error` with the message "Unknown native exception" |

If a JavaScript exception is already pending when the C++ exception is thrown, the pending exception is kept. Declaring a function `noexcept` skips the exception handling. When exceptions are disabled, there is no exception handling, and functions don't need to be declared `noexcept`.

#### `js_create_function<fn, options>(js_env_t *, [const char *name, [size_t len,]] result)`

Create a JavaScript function from the native function `fn`. The `result` is either a `js_handle_t` or a `js_function_t<R, A...>` whose types match the signature of `fn`, leaving out the `js_env_t *`. The `name` is optional and defaults to an anonymous function, and the `len` defaults to a `NULL`-terminated `name`.

```cpp
js_function_t<int32_t, int32_t, int32_t> fn;
err = js_create_function<add>(env, "add", fn);
```

To create a function and assign it to a property in one step, use `js_set_property<fn, options>(js_env_t *, object, name)`:

```cpp
err = js_set_property<add>(env, exports, "add");
```

#### `js_function_options_t`

The options used when creating a native function, passed as the second template argument of `js_create_function()`:

- `checked`: Whether values are checked when marshalled and unmarshalled, such as the types of arguments and the range of `int64_t` results. Defaults to `true` in debug builds and `false` otherwise.
- `scoped`: Whether the typed entry point of a function that takes a `js_env_t *` opens a handle scope around each call. Defaults to `true`. Turn it off only for functions that create no handles, or that manage their own handle scopes. The untyped entry point always runs in a handle scope that the engine provides.
- `statistics`: A pointer to a `js_function_statistics_t` that counts the calls of the function. Defaults to `nullptr`.

```cpp
static js_function_statistics_t add_statistics;

err = js_create_function<add, js_function_options_t(false, &add_statistics)>(env, "add", fn);
```

#### `js_function_statistics_t`

Counts the calls of a native function. As it is passed as a template argument, it must have static storage duration.

##### `uint64_t js_function_statistics_t.calls()`

The total number of calls.

##### `uint64_t js_function_statistics_t.calls(js_function_call_t::type_t type)`

The number of calls through the typed or untyped entry point, given as `js_function_call_t::typed` or `js_function_call_t::untyped`.

##### `bool js_function_statistics_t.optimized()`

Whether the function has been called through its typed entry point at least once.

#### `js_receiver_t`

The `this` value of a call. When the first argument of a native function is of type `js_receiver_t`, it receives the `this` value instead of the first JavaScript argument:

```cpp
static void
greet(js_env_t *env, js_receiver_t self, js_string_t name) {
  // ...
}
```

A `js_receiver_t` can be created from any handle type, and can also be the first argument of a `js_function_t` that is called from native code.

#### `js_call_function<options>(js_env_t *, const js_function_t<R, A...> &, A... args, [R &result])`

Call a JavaScript function from native code. The arguments are marshalled to JavaScript and, unless `R` is `void`, the result is unmarshalled to `result`. If the first argument is of type `js_receiver_t`, it is used as the `this` value of the call. Otherwise, the global object is.

If the function throws, the call fails with `js_pending_exception` and the exception is left pending, to be handled by the caller.

#### `js_call_function_with_checkpoint<options>(js_env_t *, const js_function_t<R, A...> &, A... args, [R &result])`

Like `js_call_function()`, but always performs a microtask checkpoint after the call. It must only be used when no JavaScript is already executing on the stack, such as when calling into JavaScript from the event loop. If in doubt, use `js_call_function()`, which performs microtask checkpoints as needed.

### Persistent references

#### `js_persistent_t<T>`

A reference to a value of handle type `T` that keeps the value alive outside of the handle scope it was created in. Create one with `js_create_reference()`, and get the value back with `js_get_reference_value()`, which, like any function that returns a handle, must be called within a handle scope:

```cpp
js_persistent_t<js_object_t> reference;
err = js_create_reference(env, object, reference);

// Later, within a handle scope.

js_object_t object;
err = js_get_reference_value(env, reference, object);
```

Copying a `js_persistent_t<T>` shares the underlying reference between the copies, and the value is kept alive until every copy has been reset or destroyed. Moving it transfers the reference and leaves the source empty. Assigning to a `js_persistent_t<T>` first resets it.

A `js_persistent_t<T>` must only be used on the thread of its environment, and must be reset or destroyed before the environment is destroyed.

##### `js_persistent_t<T>.reset()`

Release the reference, leaving the `js_persistent_t<T>` empty. The value can be garbage collected once every copy has been released.

##### `bool js_persistent_t<T>.empty()`

Whether the `js_persistent_t<T>` holds no reference, as is the case when default constructed, reset, or moved from. The `bool` conversion reports the opposite.

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

#### `uint64_t`

A `number` in JavaScript with a valid range of 0 to `Number.MAX_SAFE_INTEGER` (9,007,199,254,740,991).

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

#### `js_typedarray_span_of_t<T>`

The elements of a `TypedArray` in JavaScript interpreted as an array of elements of type `T`. Supports indexing operations such as `y = typedarray[x]`, `typedarray[x] = y`, and `p = &typedarray[x]`.

#### `js_typedarray_span_of_t<T, 1>`

The elements of a `TypedArray` in JavaScript interpreted as a single element of type `T`. Supports dereferencing operations such as `value->field` and `*value`.

#### `T *`

An `external` value in JavaScript that is a pointer to an element of type `T`.

#### `utf8_t[N]`

A `string` in JavaScript represented as a UTF-8 `NULL`-terminated C string of `N` characters.

#### `const utf8_t[N]`

A `string` in JavaScript represented as an immutable UTF-8 `NULL`-terminated C string of `N` characters.

#### `utf8_t *`

A `string` in JavaScript represented as a UTF-8 `NULL`-terminated C string.

> [!NOTE]
> Values of type `utf8_t *` can only be marshalled, as unmarshalling would need to allocate. Unmarshall to `utf8_t[N]` instead.

#### `const utf8_t *`

A `string` in JavaScript represented as an immutable UTF-8 `NULL`-terminated C string.

#### `utf16_t[N]`

A `string` in JavaScript represented as a UTF-16LE `NULL`-terminated C string of `N` characters.

#### `const utf16_t[N]`

A `string` in JavaScript represented as an immutable UTF-16LE `NULL`-terminated C string of `N` characters.

#### `utf16_t *`

A `string` in JavaScript represented as a UTF-16LE `NULL`-terminated C string.

> [!NOTE]
> Values of type `utf16_t *` can only be marshalled, as unmarshalling would need to allocate. Unmarshall to `utf16_t[N]` instead.

#### `const utf16_t *`

A `string` in JavaScript represented as an immutable UTF-16LE `NULL`-terminated C string.

#### `char[N]`

A `string` in JavaScript represented as a `NULL`-terminated C string of `N` characters.

#### `const char[N]`

A `string` in JavaScript represented as an immutable `NULL`-terminated C string of `N` characters.

#### `char *`

A `string` in JavaScript represented as a `NULL`-terminated C string.

> [!NOTE]
> Values of type `char *` can only be marshalled, as unmarshalling would need to allocate. Unmarshall to `char[N]` instead.

#### `const char *`

A `string` in JavaScript represented as an immutable `NULL`-terminated C string.

#### `std::string`

A `string` in JavaScript represented as a C++ string. Requires `<jstl/std.h>`.

#### `std::string_view`

A `string` in JavaScript represented as a view of a C++ string. Requires `<jstl/std.h>`.

> [!NOTE]
> Values of type `std::string_view` can only be marshalled, as the view doesn't own the memory it points to. Unmarshall to `std::string` instead.

#### `std::u16string`

A `string` in JavaScript represented as a UTF-16 C++ string. Requires `<jstl/std.h>`.

#### `std::u16string_view`

A `string` in JavaScript represented as a view of a UTF-16 C++ string. Requires `<jstl/std.h>`.

> [!NOTE]
> Values of type `std::u16string_view` can only be marshalled, as the view doesn't own the memory it points to. Unmarshall to `std::u16string` instead.

#### `T[N]`

An `Array` in JavaScript represented as a C array with `N` elements of type `T`.

#### `const T[N]`

An `Array` in JavaScript represented as a C array with `N` immutable elements of type `T`.

#### `std::array<T, N>`

An `Array` in JavaScript represented as a C++ array with `N` elements. Requires `<jstl/std.h>`.

#### `std::vector<T>`

An `Array` in JavaScript represented as a C++ vector. Requires `<jstl/std.h>`.

#### `std::tuple<T...>`

An `Array` in JavaScript represented as a C++ tuple. Requires `<jstl/std.h>`.

#### `std::pair<A, B>`

An `Array` in JavaScript with two elements, represented as a C++ pair. Requires `<jstl/std.h>`.

#### `std::span<T>`

The elements of a `TypedArray` in JavaScript that is a view of elements of type `T`, represented as a C++ span. Requires `<jstl/std.h>`.

#### `std::span<const T>`

The elements of a `TypedArray` in JavaScript that is an immutable view of elements of type `T`, represented as a C++ span. Requires `<jstl/std.h>`.

#### `std::optional<T>`

Either `undefined` or a value of type `T`. Requires `<jstl/std.h>`.

#### `std::chrono::time_point<std::chrono::system_clock, D>`

A `Date` in JavaScript represented as a point in time of the system clock, such as `std::chrono::system_clock::time_point`. Requires `<jstl/std.h>`.

> [!NOTE]
> Unmarshalling a `Date` that is invalid or that cannot be represented with the duration `D` throws a `RangeError` in JavaScript.

#### `std::shared_ptr<T>`

An `external` value in JavaScript that is a shared pointer to an element of type `T`. Requires `<jstl/std.h>`.

#### `std::unique_ptr<T>`

An `external` value in JavaScript that is a unique pointer to an element of type `T`. Requires `<jstl/std.h>`.

## License

Apache-2.0
