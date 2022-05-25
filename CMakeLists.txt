cmake_minimum_required(VERSION 3.22)
project(coltest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# enable_testing()

add_subdirectory(abseil-cpp)


add_executable(coltest)
target_sources(coltest PRIVATE
	main.cpp
)

target_link_libraries(coltest PRIVATE
	absl::random_random
	absl::btree
	absl::hash
	absl::raw_hash_set
	absl::strings
)
