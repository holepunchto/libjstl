fetch_package("github:holepunchto/libjs")

list(APPEND tests
  add-deferred-teardown-callback
  add-deferred-teardown-callback-finish
  add-teardown-callback
  add-teardown-callback-remove
  add-teardown-callback-remove-with-data
  add-teardown-callback-with-data
  create-external-arraybuffer-with-finalizer
  create-external-arraybuffer-with-finalizer-detach
  create-function-pointer
  create-function-receiver
  create-function-receiver-no-env
  create-function-return-array-int32
  create-function-return-arraybuffer
  create-function-return-arraybuffer-span
  create-function-return-bigint64
  create-function-return-biguint64
  create-function-return-bool
  create-function-return-double
  create-function-return-int32
  create-function-return-int64
  create-function-return-pointer
  create-function-return-string
  create-function-return-string-literal
  create-function-return-uint8array
  create-function-return-uint8array-any
  create-function-return-uint8array-span
  create-function-return-uint8array-typedarray-span
  create-function-return-uint8array-typedarray-span-any
  create-function-return-uint16array
  create-function-return-uint16array-any
  create-function-return-uint16array-span
  create-function-return-uint16array-typedarray-span
  create-function-return-uint16array-typedarray-span-any
  create-function-return-uint32
  create-function-return-uint64
  create-function-return-vector-int32
  create-function-return-void
  create-function-return-void-arg-array-int32
  create-function-return-void-arg-arraybuffer
  create-function-return-void-arg-arraybuffer-span
  create-function-return-void-arg-arraybuffer-span-of-struct
  create-function-return-void-arg-arraybuffer-span-of-struct-dynamic
  create-function-return-void-arg-bigint64
  create-function-return-void-arg-biguint64
  create-function-return-void-arg-bool
  create-function-return-void-arg-double
  create-function-return-void-arg-int32
  create-function-return-void-arg-int64
  create-function-return-void-arg-multiple
  create-function-return-void-arg-optional-string
  create-function-return-void-arg-pointer
  create-function-return-void-arg-string
  create-function-return-void-arg-string-literal
  create-function-return-void-arg-tuple-int32
  create-function-return-void-arg-tuple-int32-string
  create-function-return-void-arg-uint8array
  create-function-return-void-arg-uint8array-any
  create-function-return-void-arg-uint8array-span
  create-function-return-void-arg-uint8array-typedarray-span
  create-function-return-void-arg-uint8array-typedarray-span-any
  create-function-return-void-arg-uint8array-typedarray-span-of-struct
  create-function-return-void-arg-uint8array-typedarray-span-of-struct-dynamic
  create-function-return-void-arg-uint16array
  create-function-return-void-arg-uint16array-any
  create-function-return-void-arg-uint16array-span
  create-function-return-void-arg-uint16array-typedarray-span
  create-function-return-void-arg-uint16array-typedarray-span-any
  create-function-return-void-arg-uint32
  create-function-return-void-arg-uint64
  create-function-return-void-arg-vector-int32
  create-function-with-statistics
  create-object-with-properties
  create-reference-get-value
  create-reference-overwrite-previous
  create-reference-move-assign
  create-typedarray-data-cast
  create-typedarray-get-info
  create-typedarray-get-info-any
  create-typedarray-get-info-copy
  create-typedarray-get-info-data-cast
  create-typedarray-get-info-move-assign
  set-get-property-literal-char-array
  set-get-property-literal-char-pointer
  set-get-property-literal-function-pointer
  set-get-property-literal-int32
  set-get-property-literal-struct
  set-get-property-literal-uint32
)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.cc)

  harden(${test} CXX)

  set_target_properties(
    ${test}
    PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 20
    CXX_SCAN_FOR_MODULES OFF
  )

  target_link_libraries(
    ${test}
    PRIVATE
      js_shared
      jstl
  )

  add_test(
    NAME ${test}
    COMMAND ${test}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )

  set_tests_properties(
    ${test}
    PROPERTIES
    TIMEOUT 30
  )
endforeach()
