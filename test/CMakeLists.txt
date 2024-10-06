# Dummy test
add_executable(dummy_test DummyTest.cpp)
add_dependencies(dummy_test image_ipc)
target_link_libraries(dummy_test image_ipc)
target_include_directories(dummy_test
PRIVATE ../example)
configure_test(dummy_test)
