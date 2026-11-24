fetch_package("github:holepunchto/libjs")

set(tests
  add-deferred-teardown-callback.cc
  add-deferred-teardown-callback-finish.cc
  add-teardown-callback.cc
  add-teardown-callback-remove.cc
  add-teardown-callback-remove-with-data.cc
  add-teardown-callback-with-data.cc
  create-array-vector-object.cc
  create-array-vector-tuple.cc
  create-external-arraybuffer-with-finalizer.cc
  create-external-arraybuffer-with-finalizer-detach.cc
  create-function-noexcept.cc
  create-function-pointer.cc
  create-function-receiver.cc
  create-function-receiver-no-env.cc
  create-function-throw.cc
  create-function-throw-hot.cc
  create-function-throw-round-trip.cc
  create-function-unscoped.cc
  create-function-return-array-int32.cc
  create-function-return-arraybuffer.cc
  create-function-return-arraybuffer-span.cc
  create-function-return-bigint64.cc
  create-function-return-biguint64.cc
  create-function-return-bool.cc
  create-function-return-date-arg-date.cc
  create-function-return-double.cc
  create-function-return-external-hot.cc
  create-function-return-int32.cc
  create-function-return-int32-arg-optional-uint32-hot.cc
  create-function-return-int64.cc
  create-function-return-pair-arg-pair.cc
  create-function-return-pointer.cc
  create-function-return-shared-ptr.cc
  create-function-return-string-view.cc
  create-function-return-string.cc
  create-function-return-string-literal.cc
  create-function-return-u16string-arg-u16string.cc
  create-function-return-uint8array.cc
  create-function-return-uint8array-any.cc
  create-function-return-uint8array-span.cc
  create-function-return-uint8array-typedarray-span.cc
  create-function-return-uint8array-typedarray-span-any.cc
  create-function-return-uint16array.cc
  create-function-return-uint16array-any.cc
  create-function-return-uint16array-span.cc
  create-function-return-uint16array-typedarray-span.cc
  create-function-return-uint16array-typedarray-span-any.cc
  create-function-return-uint32.cc
  create-function-return-uint64.cc
  create-function-return-unique-ptr.cc
  create-function-return-vector-int32.cc
  create-function-return-vector-int32-pending-exception.cc
  create-function-return-void.cc
  create-function-return-void-arg-array-int32.cc
  create-function-return-void-arg-arraybuffer.cc
  create-function-return-void-arg-arraybuffer-span.cc
  create-function-return-void-arg-arraybuffer-span-of-struct.cc
  create-function-return-void-arg-arraybuffer-span-of-struct-dynamic.cc
  create-function-return-void-arg-bigint64.cc
  create-function-return-void-arg-biguint64.cc
  create-function-return-void-arg-bool.cc
  create-function-return-void-arg-double.cc
  create-function-return-void-arg-int32.cc
  create-function-return-void-arg-int64.cc
  create-function-return-void-arg-multiple.cc
  create-function-return-void-arg-optional-string.cc
  create-function-return-void-arg-pointer.cc
  create-function-return-void-arg-shared-ptr.cc
  create-function-return-void-arg-string.cc
  create-function-return-void-arg-string-literal.cc
  create-function-return-void-arg-tuple-int32.cc
  create-function-return-void-arg-tuple-int32-string.cc
  create-function-return-void-arg-uint8array.cc
  create-function-return-void-arg-uint8array-any.cc
  create-function-return-void-arg-uint8array-span.cc
  create-function-return-void-arg-uint8array-typedarray-span.cc
  create-function-return-void-arg-uint8array-typedarray-span-any.cc
  create-function-return-void-arg-uint8array-typedarray-span-of-struct.cc
  create-function-return-void-arg-uint8array-typedarray-span-of-struct-dynamic.cc
  create-function-return-void-arg-uint16array.cc
  create-function-return-void-arg-uint16array-any.cc
  create-function-return-void-arg-uint16array-span.cc
  create-function-return-void-arg-uint16array-typedarray-span.cc
  create-function-return-void-arg-uint16array-typedarray-span-any.cc
  create-function-return-void-arg-uint32.cc
  create-function-return-void-arg-uint64.cc
  create-function-return-void-arg-unique-ptr.cc
  create-function-return-void-arg-vector-int32.cc
  create-function-with-statistics.cc
  create-object-with-properties.cc
  create-object-with-properties-handles.cc
  create-reference-get-value.cc
  create-reference-overwrite-previous.cc
  create-reference-move-assign.cc
  create-threadsafe-function.cc
  create-threadsafe-function-no-callback.cc
  create-threadsafe-function-with-finalizer.cc
  get-null.cc
  get-value-double.cc
  create-typedarray-data-cast.cc
  create-typedarray-get-info.cc
  create-typedarray-get-info-any.cc
  create-typedarray-get-info-copy.cc
  create-typedarray-get-info-data-cast.cc
  create-typedarray-get-info-move-assign.cc
  set-get-property-literal-char-array.cc
  set-get-property-literal-char-pointer.cc
  set-get-property-literal-function-pointer.cc
  set-get-property-literal-int32.cc
  set-get-property-literal-struct.cc
  set-get-property-literal-uint32.cc
)

add_test_executables(
  ${tests}
  LINK
    jstl::headers
    js::<TYPE>
)

# Tests that don't include <jstl/std.h> are built a second time without
# exceptions, RTTI, or the C++ standard library, which fails to link if the
# core header pulls in anything from the C++ runtime. Linking the static libjs
# would pull it in regardless, so this needs the shared library.
include(CheckLinkerFlag)

check_linker_flag(CXX -nostdlib++ supports_nostdlibxx)

if(supports_nostdlibxx AND TARGET js::shared)
  set(freestanding_tests)

  foreach(test IN LISTS tests)
    file(READ "${test}" source)

    if(NOT source MATCHES "#include <jstl/std.h>")
      list(APPEND freestanding_tests "${test}")
    endif()
  endforeach()

  add_test_executables(
    ${freestanding_tests}
    PREFIX freestanding-
    LINK
      jstl::headers
      js::<TYPE>
  )

  foreach(test IN LISTS freestanding_tests)
    cmake_path(REMOVE_EXTENSION test LAST_ONLY OUTPUT_VARIABLE target)

    target_compile_options(freestanding-${target} PRIVATE -fno-exceptions -fno-rtti)

    target_link_options(freestanding-${target} PRIVATE -nostdlib++)
  endforeach()
endif()
