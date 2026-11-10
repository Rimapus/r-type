# LibFuzzer harnesses, enabled by the "fuzz" preset (needs Clang).
# Each harness is its own executable: libFuzzer brings the main loop and
# feeds arbitrary bytes to LLVMFuzzerTestOneInput, which must never crash,
# hang or leak no matter the input.

add_executable(fuzz_packet packet.cpp)
target_link_libraries(fuzz_packet PRIVATE younity_network)

# The fuzzer runtime (its main loop) is linked here only: at global scope it
# would also hijack the main of CMake's own compiler checks.
target_link_options(fuzz_packet PRIVATE -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)

set_target_properties(fuzz_packet PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(fuzz_packet PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
younity_set_warnings(fuzz_packet)
