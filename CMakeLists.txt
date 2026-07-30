cmake_minimum_required(VERSION 3.28)
project(hopfield_neural_network VERSION 0.1.0)

include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(APPEND CMAKE_CXX_FLAGS
      " -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion"
      " -Wshadow -Wimplicit-fallthrough -Wextra-semi -Wold-style-cast"
      " -fno-omit-frame-pointer")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  string(APPEND CMAKE_CXX_FLAGS " -D_GLIBCXX_ASSERTIONS")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  string(APPEND CMAKE_CXX_FLAGS " -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE")
endif()

string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address,undefined")
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  string(APPEND CMAKE_CXX_FLAGS " -D_GLIBCXX_SANITIZE_STD_ALLOCATOR")
endif()
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -fsanitize=address,undefined")

find_package(SFML 2.6 COMPONENTS graphics REQUIRED)

add_executable(acquisition main/main_acquisition.cpp src/acquisition.cpp src/pattern.cpp)
target_link_libraries(acquisition PRIVATE sfml-graphics)

add_executable(training main/main_training.cpp src/training.cpp src/weight_matrix.cpp src/pattern.cpp)
target_link_libraries(training PRIVATE sfml-graphics)

add_executable(recall main/main_recall.cpp src/recall.cpp src/weight_matrix.cpp src/pattern.cpp)
target_link_libraries(recall PRIVATE sfml-graphics)

if (BUILD_TESTING)

  add_executable(pattern.t tests/src/pattern.test.cpp src/pattern.cpp)
  target_link_libraries(pattern.t PRIVATE sfml-graphics)
  add_test(NAME pattern.t COMMAND pattern.t)

  add_executable(acquisition.t tests/src/acquisition.test.cpp src/acquisition.cpp src/pattern.cpp)
  target_link_libraries(acquisition.t PRIVATE sfml-graphics)
  add_test(NAME acquisition.t COMMAND acquisition.t)

  add_executable(weight_matrix.t tests/src/weight_matrix.test.cpp src/weight_matrix.cpp)
  add_test(NAME weight_matrix.t COMMAND weight_matrix.t)

  add_executable(training.t tests/src/training.test.cpp src/training.cpp src/weight_matrix.cpp src/pattern.cpp)
  target_link_libraries(training.t PRIVATE sfml-graphics)
  add_test(NAME training.t COMMAND training.t)

  add_executable(recall.t tests/src/recall.test.cpp src/recall.cpp src/weight_matrix.cpp src/pattern.cpp)
  target_link_libraries(recall.t PRIVATE sfml-graphics)
  add_test(NAME recall.t COMMAND recall.t)

endif()