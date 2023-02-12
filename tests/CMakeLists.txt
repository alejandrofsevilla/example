add_executable(example_tests
  TestHelper.hpp
  TcpTest.cpp)

target_link_libraries(example_tests PRIVATE example)
find_package(GTest 1.11.0 REQUIRED)
if (GTest_FOUND)
  target_link_libraries(example_tests PRIVATE GTest::gtest GTest::gtest_main)
endif()
