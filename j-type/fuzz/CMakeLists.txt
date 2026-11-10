# LibFuzzer harnesses, enabled by the "fuzz" preset (needs Clang).
# Each harness is its own executable: libFuzzer brings the main loop and
# feeds arbitrary bytes to LLVMFuzzerTestOneInput, which must never crash,
# hang or leak no matter the input.

add_executable(fuzz_messages messages.cpp)
target_link_libraries(fuzz_messages PRIVATE JType::Common)

# The fuzzer runtime (its main loop) is linked here only: at global scope it
# would also hijack the main of CMake's own compiler checks.
target_link_options(fuzz_messages PRIVATE -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)

set_target_properties(fuzz_messages PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(fuzz_messages PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
jtype_set_warnings(fuzz_messages)
