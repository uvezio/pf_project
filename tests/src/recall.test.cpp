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

#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>

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

  SUBCASE("Invalid output directories")
  {
    std::vector<std::filesystem::path> output_directories{
        "corrupted_patterns/noisy_patterns/",
        "corrupted_patterns/incomplete_patterns/",
        "images/corrupted_images/noisy_images/",
        "images/corrupted_images/noisy_images/"};

    for (auto const& dir : output_directories) {
      SUBCASE("Non existing directory")
      {
        REQUIRE(std::filesystem::exists("../tests/" + dir.string()));
        for (auto const& file :
             std::filesystem::directory_iterator("../tests/" + dir.string())) {
          std::filesystem::remove_all(file);
        }
        std::filesystem::remove("../tests/" + dir.string());
        REQUIRE(!std::filesystem::exists("../tests/" + dir.string()));
        nn::Recall rec{"tests/"};
        CHECK(std::filesystem::exists("../tests/" + dir.string()));
      }

      SUBCASE("Not empty directory")
      {
        std::ofstream img{"../tests/" + dir.string() + "0.txt"};
        REQUIRE(!std::filesystem::is_empty("../tests/" + dir.string()));
        nn::Recall rec{"tests/"};
        CHECK(std::filesystem::is_empty("../tests/" + dir.string()));
      }
    }
  }
}

TEST_CASE("Testing corrupt_patterns()")
{
  nn::Recall recall{"tests/"};

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

    CHECK_THROWS(recall.corrupt_patterns());
    std::filesystem::remove("../tests/patterns/under_sized.txt");
    REQUIRE(!std::filesystem::exists("../tests/patterns/under_sized.txt"));
  }

  SUBCASE(
      "Acquiring all the patterns in the directory and saving them")
  {
    recall.corrupt_patterns();

    for (int i{1}; i != 5; ++i) {
      std::filesystem::path name{std::to_string(i) + ".txt"};
      nn::Pattern pattern;

      CHECK(std::filesystem::is_regular_file(
          "../tests/corrupted_patterns/noisy_patterns/" + name.string()));
      pattern.load_from_file("../tests/corrupted_patterns/noisy_patterns/",
                             name, 4096);
      CHECK(pattern.size() == 4096);

      CHECK(std::filesystem::is_regular_file(
          "../tests/corrupted_patterns/incomplete_patterns/" + name.string()));
      pattern.load_from_file("../tests/corrupted_patterns/incomplete_patterns/",
                             name, 4096);
      CHECK(pattern.size() == 4096);

      name.replace_extension(".png");
      sf::Image corrupted_image;

      CHECK(corrupted_image.loadFromFile(
          "../tests/images/corrupted_images/noisy_images/" + name.string()));
      CHECK(corrupted_image.getSize().x == 64);
      CHECK(corrupted_image.getSize().y == 64);

      CHECK(corrupted_image.loadFromFile(
          "../tests/images/corrupted_images/incomplete_images/"
          + name.string()));
      CHECK(corrupted_image.getSize().x == 64);
      CHECK(corrupted_image.getSize().y == 64);
    }
  }
}