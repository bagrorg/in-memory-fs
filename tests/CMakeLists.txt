find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(tests test.cpp)
target_link_libraries(tests ${GTEST_LIBRARIES} pthread storage)