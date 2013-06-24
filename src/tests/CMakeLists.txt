add_definitions(-DUNIT_TESTS)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Use create_test() framework...
add_executable(slebtest slebtest.cpp)
