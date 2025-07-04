// All relative paths are relative to the "build/" directory

/*
 * This test takes as input the patterns "1.txt", "2.txt", "3.txt", "4.txt" in
 * "../tests/patterns/" the weight matrix "weight_matrix.txt" in
 * "../tests/weight_matrix/" and generates the images "1.png", "2.png",
 * "3.png", "4.png" in both "../tests/images/corrupted_images/noisy_images/" and
 * "../tests/images/corrupted_images/incomplete_images/".
 *
 * This test writes temporary files to perform the necessary checks.
 *
 * This test uses implicitly output files of the test in
 * "pattern.test.cpp" controlling their actual removal during the
 * construction of the nn::Recall object in TEST_CASE "Testing the Training
 * class on invalid directories".
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// These two paths are the only ones relative to "recall.test.cpp"
#include "../../include/recall.hpp"
#include "../doctest.h"

#include <fstream>

TEST_CASE("Testing the Recall class on invalid directories")
{
  SUBCASE("Non existing patterns and weight matrix directory "
          "(\"../non_existing/patterns/\", \"../non_existing/weight_matrix/\")")
  {
    REQUIRE(!std::filesystem::exists("../non_existing/weight_matrix/"));
    REQUIRE(!std::filesystem::exists("../non_existing/patterns/"));
    CHECK_THROWS(nn::Recall("non_existing/"));
    REQUIRE(!std::filesystem::exists("../non_existing/weight_matrix/"));
    REQUIRE(!std::filesystem::exists("../non_existing/patterns/"));
  }

  SUBCASE("Weight matrix directory with a non regular file "
          "(\"../tests/weight_matrix/dir/\")")
  {
    std::filesystem::create_directory("../tests/weight_matrix/dir/");
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/weight_matrix/dir/");
  }

  SUBCASE("Weight matrix directory with a file different from "
          "\"weight_matrix.txt\"")
  {
    std::ofstream other{"../tests/weight_matrix/other.txt"};
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/weight_matrix/other.txt");
  }

  SUBCASE("Patterns directory with a non regular file "
          "(\"../tests/patterns/dir/\")")
  {
    std::filesystem::create_directory("../tests/patterns/dir/");
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/patterns/dir/");
  }

  SUBCASE("Patterns containing files with invalid extensions "
          "(\"../tests/patterns/invalid_extension.pdf\")")
  {
    std::ofstream inv{"../tests/patterns/invalid_extension.pdf"};
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/patterns/invalid_extension.pdf");
  }

  SUBCASE("Non existing noisy directory")
  {
    REQUIRE(std::filesystem::exists(
        "../tests/images/corrupted_images/noisy_images/"));
    for (auto const& file : std::filesystem::directory_iterator(
             "../tests/images/corrupted_images/noisy_images/")) {
      std::filesystem::remove_all(file);
    }
    std::filesystem::remove("../tests/images/corrupted_images/noisy_images/");
    REQUIRE(!std::filesystem::exists(
        "../tests/images/corrupted_images/noisy_images/"));
    nn::Recall rec{"tests/"};
    CHECK(std::filesystem::exists(
        "../tests/images/corrupted_images/noisy_images/"));
  }

  SUBCASE("Not empty noisy directory")
  {
    std::ofstream img{"../tests/images/corrupted_images/noisy_images/0.png"};
    REQUIRE(!std::filesystem::is_empty(
        "../tests/images/corrupted_images/noisy_images/"));
    nn::Recall rec{"tests/"};
    CHECK(std::filesystem::is_empty(
        "../tests/images/corrupted_images/noisy_images/"));
  }

  SUBCASE("Non existing incomplete directory")
  {
    REQUIRE(std::filesystem::exists(
        "../tests/images/corrupted_images/incomplete_images/"));
    for (auto const& file : std::filesystem::directory_iterator(
             "../tests/images/corrupted_images/incomplete_images/")) {
      std::filesystem::remove_all(file);
    }
    std::filesystem::remove(
        "../tests/images/corrupted_images/incomplete_images/");
    REQUIRE(!std::filesystem::exists(
        "../tests/images/corrupted_images/incomplete_images/"));
    nn::Recall rec{"tests/"};
    CHECK(std::filesystem::exists(
        "../tests/images/corrupted_images/incomplete_images/"));
  }

  SUBCASE("Not empty patterns directory")
  {
    std::ofstream img{
        "../tests/images/corrupted_images/incomplete_images/0.png"};
    REQUIRE(!std::filesystem::is_empty(
        "../tests/images/corrupted_images/incomplete_images/"));
    nn::Recall rec{"tests/"};
    CHECK(std::filesystem::is_empty(
        "../tests/images/corrupted_images/incomplete_images/"));
  }
}