// All relative paths are relative to the "build/" directory

/*
 * This test takes as input the patterns "1.txt", "2.txt", "3.txt", "4.txt" in
 * "../tests/patterns/" and generates the weight matrix "weight_matrix.txt" in
 * "../tests/weight_matrix/".
 *
 * This test writes temporary files to perform the necessary checks.
 *
 * This test uses implicitly output files of the test in
 * "weight_matrix.test.cpp" controlling their actual removal during the
 * construction of the nn::Training object in TEST_CASE "Testing the Training
 * class on invalid directories".
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// These two paths are the only ones relative to "training.test.cpp"
#include "../../include/training.hpp"
#include "../doctest.h"

#include <fstream>

TEST_CASE("Testing the Training class on invalid directories")
{
  SUBCASE("Non existing patterns directory "
          "(\"../non_existing/patterns/\")")
  {
    REQUIRE(!std::filesystem::exists("../non_existing/patterns/"));
    CHECK_THROWS(nn::Training("non_existing/"));
    REQUIRE(!std::filesystem::exists("../non_existing/weight_matrix/"));
  }

  SUBCASE("Patterns directory with a non regular file "
          "(\"../tests/patterns/dir/\")")
  {
    std::filesystem::create_directory("../tests/patterns/dir/");
    CHECK_THROWS(nn::Training("tests/"));
    std::filesystem::remove("../tests/patterns/dir/");
  }

  SUBCASE("Patterns directory containing files with invalid extensions "
          "(\"../tests/patterns/invalid_extension.pdf\")")
  {
    std::ofstream inv{"../tests/patterns/invalid_extension.pdf"};
    CHECK_THROWS(nn::Training("tests/"));
    std::filesystem::remove("../tests/patterns/invalid_extension.pdf");
  }

  SUBCASE("Not empty weight matrix directory")
  {
    REQUIRE(!std::filesystem::is_empty("../tests/weight_matrix/"));
    nn::Training training{"tests/"};
    CHECK(std::filesystem::is_empty("../tests/weight_matrix/"));
  }

  SUBCASE("Non existing weight matrix directory")
  {
    REQUIRE(std::filesystem::exists("../tests/weight_matrix/"));
    REQUIRE(std::filesystem::is_empty("../tests/weight_matrix/"));
    std::filesystem::remove("../tests/weight_matrix/");
    REQUIRE(!std::filesystem::exists("../tests/weight_matrix/"));
    nn::Training training{"tests/"};
    CHECK(std::filesystem::exists("../tests/weight_matrix/"));
  }
}

TEST_CASE("Testing acquire_and_save_weight_matrix()")
{
  nn::Training training{"tests/"};
  REQUIRE(training.weight_matrix().neurons() == 4096);
  REQUIRE(training.weight_matrix().weights().size() == 0);

  SUBCASE("Acquiring an under-sized pattern \"(under_sized.txt)\"")
  {
    std::ofstream undersized{"../tests/patterns/under_sized.txt"};
    for (int i{0}; i != 4095; ++i) {
      undersized << ((i % 3 == 0) ? +1 : -1);
    }
    undersized.close();
    // If incorrect is not closed here, file at
    // "../tests/patterns/under_sized.txt" could be written after the is_empty()
    // check in load_from_file()

    CHECK_THROWS(training.acquire_and_save_weight_matrix());
    std::filesystem::remove("../tests/patterns/under_sized.txt");
    REQUIRE(!std::filesystem::exists("../tests/patterns/under_sized.txt"));
  }

  SUBCASE("Acquiring all the patterns in the directory and filling the weight "
          "matrix")
  {
    training.acquire_and_save_weight_matrix();
    CHECK(training.weight_matrix().weights().size() == 4096 * 4095 / 2);

    CHECK(std::filesystem::is_regular_file(
        "../tests/weight_matrix/weight_matrix.txt"));

    nn::Weight_Matrix weight_matrix;
    weight_matrix.load_from_file("../tests/weight_matrix/", "weight_matrix.txt",
                                 4096);
    CHECK(weight_matrix.weights().size() == 4096 * 4095 / 2);

    CHECK(weight_matrix.weights()[weight_matrix.matrix_to_vector_index(1, 12)]
          == doctest::Approx(0.000488281).epsilon(0.000000001));
    CHECK(weight_matrix.weights()[weight_matrix.matrix_to_vector_index(2, 5)]
          == doctest::Approx(0.000976562).epsilon(0.000000001));
    CHECK(weight_matrix
              .weights()[weight_matrix.matrix_to_vector_index(4094, 4095)]
          == doctest::Approx(0.000976562).epsilon(0.000000001));
    CHECK(weight_matrix
              .weights()[weight_matrix.matrix_to_vector_index(4095, 4096)]
          == doctest::Approx(0.000976562).epsilon(0.000000001));
  }
}