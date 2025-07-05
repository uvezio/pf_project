cmake_minimum_required(VERSION 3.28)
project(progettopf VERSION 0.1.0)

# abilita il supporto per i test, tra cui l'opzione BUILD_TESTING usata sotto
include(CTest)

# esporta il compilation database, utile per altri strumenti, a partire dall'editor
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# richiedi l'uso di C++20 (volendo anche C++23), senza estensioni non-standard offerte dal compilatore usato
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# abilita warning
string(APPEND CMAKE_CXX_FLAGS
      " -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion"
      " -Wshadow -Wimplicit-fallthrough -Wextra-semi -Wold-style-cast"
      " -fno-omit-frame-pointer")

# abilita asserzioni della standard library
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  string(APPEND CMAKE_CXX_FLAGS " -D_GLIBCXX_ASSERTIONS")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  string(APPEND CMAKE_CXX_FLAGS " -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE")
endif()

# abilita address sanitizer e undefined-behaviour sanitizer in Debug mode
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address,undefined")
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  string(APPEND CMAKE_CXX_FLAGS " -D_GLIBCXX_SANITIZE_STD_ALLOCATOR")
endif()
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -fsanitize=address,undefined")

# se usato, richiedi il componente graphics della libreria SFML (versione 2.6 in Ubuntu 24.04)
find_package(SFML 2.6 COMPONENTS graphics REQUIRED)

# dichiara un eseguibile chiamato "progetto", prodotto a partire dai file sorgente indicati
# sostituire "progetto" con il nome del proprio eseguibile e i file sorgente con i propri (con nomi sensati!)
add_executable(acquisition main/main_acquisition.cpp src/acquisition.cpp src/pattern.cpp)
# nel caso si usi SFML. analogamente per eventuali altre librerie
target_link_libraries(acquisition PRIVATE sfml-graphics)

# aggiungere eventuali altri eseguibili
add_executable(training main/main_training.cpp src/training.cpp src/weight_matrix.cpp src/pattern.cpp)
target_link_libraries(training PRIVATE sfml-graphics)

# il testing e' abilitato di default
# per disabilitarlo, passare -DBUILD_TESTING=OFF a cmake durante la fase di configurazione
if (BUILD_TESTING)

  # aggiungi l'eseguibile progetto.t
  add_executable(pattern.t tests/src/pattern.test.cpp src/pattern.cpp)
  target_link_libraries(pattern.t PRIVATE sfml-graphics)
  # aggiungi l'eseguibile progetto.t alla lista dei test
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
