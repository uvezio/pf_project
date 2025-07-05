// All relative paths used at runtime are relative to the "build/" directory

/*
 * This test generates the files "empty.txt", "test.txt", "incorrect.txt" in
 * "../tests/patterns/" and the images "test.png" and "test_.png" in
 * "../tests/images/binarized_images/".
 * These files are implicitly used in "acquisition.test.cpp".
 *
 * This test generates images "test.png" in
 * "../tests/images/corrupted_images/incomplete_images/", "test.png" and
 * "test_.png" in "../tests/images/corrupted_images/noisy_images/".
 * These files are implicitly used in "recall.test.cpp".
 *
 * This test does not use the images in "../tests/images/source_images/".
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// These two paths are the only ones relative to "pattern.test.cpp"
#include "../../include/pattern.hpp"
#include "../doctest.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <fstream>

TEST_CASE("Testing the add method")
{
  nn::Pattern pattern{};

  // nn::Pattern::set_directory("../tests/patterns/");

  REQUIRE(pattern.size() == 0);

  SUBCASE("Adding three admissible values")
  {
    pattern.add(-1);
    pattern.add(-1);
    pattern.add(+1);
    CHECK(pattern.size() == 3);
  }

  auto pattern1 = nn::Pattern();
  REQUIRE(pattern1.size() == 0);

  std::vector<int> values = {-1, +1, +1, -1, +1, +1, +1};

  SUBCASE("Adding a set of values")
  {
    for (auto v : values) {
      pattern1.add(v);
    }
    CHECK(pattern1.size() == 7);
  }

  for (auto v : values) {
    pattern1.add(v);
  }

  SUBCASE("Substituting an existing pattern")
  {
    pattern = pattern1;
    CHECK(pattern.size() == 7);
    CHECK(std::count(pattern.pattern().begin(), pattern.pattern().end(), +1)
          == 5);
    CHECK(std::count(pattern.pattern().begin(), pattern.pattern().end(), -1)
          == 2);
  }
}

TEST_CASE("Testing input and output")
{
  SUBCASE("Saving an empty pattern")
  {
    nn::Pattern pattern{};
    REQUIRE(pattern.size() == 0);
    pattern.save_to_file("../tests/patterns/", "empty.txt", 0);
    CHECK(std::filesystem::is_regular_file("../tests/patterns/empty.txt"));
    CHECK(std::filesystem::is_empty("../tests/patterns/empty.txt"));
    pattern.load_from_file("../tests/patterns/", "empty.txt", 0);
    CHECK(pattern.size() == 0);
  }

  SUBCASE("Saving pattern to file and loading the same pattern")
  {
    nn::Pattern pattern;
    std::vector<int> values{+1, -1, +1, +1, +1, -1, +1, -1, +1, -1};
    for (auto v : values) {
      pattern.add(v);
    }
    REQUIRE(pattern.size() == 10);

    pattern.save_to_file("../tests/patterns/", "test.txt", 10);
    CHECK(std::filesystem::is_regular_file("../tests/patterns/test.txt"));
    CHECK(!std::filesystem::is_empty("../tests/patterns/test.txt"));
    pattern.load_from_file("../tests/patterns/", "test.txt", 10);
    CHECK(pattern.size() == 10);
  }

  nn::Pattern pattern;
  REQUIRE(pattern.size() == 0);

  std::vector<int> values{+1, -1, +1, +1, 0, -1, +1, -1, +1};
  std::ofstream incorrect{"../tests/patterns/incorrect.txt"};
  for (auto v : values) {
    incorrect << v << ' ';
  }
  incorrect.close();
  // If incorrect is not closed here, file at "../tests/patterns/incorrect.txt"
  // could be written after the is_empty() check in load_from_file()

  SUBCASE("Loading an incorrect pattern")
  {
    CHECK_THROWS(
        pattern.load_from_file("../tests/patterns/", "incorrect.txt", 9));
  }

  SUBCASE("Loading over-sized and under-sized patterns")
  {
    CHECK_THROWS(pattern.load_from_file("../tests/patterns/", "test.txt", 8));
    CHECK_THROWS(pattern.load_from_file("../tests/patterns/", "test.txt", 12));
  }

  SUBCASE("Loading a pattern")
  {
    pattern.load_from_file("../tests/patterns/", "test.txt", 10);
    CHECK(pattern.size() == 10);
  }

  SUBCASE("Overwriting an existing pattern with a loaded one")
  {
    pattern.add(1);
    REQUIRE(pattern.size() == 1);
    pattern.load_from_file("../tests/patterns/", "test.txt", 10);
    CHECK(pattern.size() == 10);
  }
}

TEST_CASE("Testing creation of images")
{
  nn::Pattern pattern;
  pattern.load_from_file("../tests/patterns/", "test.txt", 10);

  SUBCASE("Creating image in an existing directory")
  {
    sf::Image image;

    pattern.create_image("../tests/images/binarized_images/", "test.txt", 5, 2);
    image.loadFromFile("../tests/images/binarized_images/test.png");
    CHECK(image.getSize().x == 5);
    CHECK(image.getSize().y == 2);

    pattern.create_image("../tests/images/binarized_images/", "test_.txt", 2,
                         5);
    image.loadFromFile("../tests/images/binarized_images/test_.png");
    CHECK(image.getSize().x == 2);
    CHECK(image.getSize().y == 5);
  }
}

TEST_CASE("Testing pattern corruption methods")
{
  nn::Pattern pattern;
  pattern.load_from_file("../tests/patterns/", "test.txt", 10);

  SUBCASE("Adding noise")
  {
    pattern.add_noise(0.3, 10);
    CHECK(pattern.size() == 10);
    pattern.create_image("../tests/corrupted_files/",
                         "test.txt", 5, 2);

    auto p = pattern.pattern();

    pattern.add_noise(1., 10);
    CHECK(pattern.size() == 10);
    pattern.create_image("../tests/corrupted_files/",
                         "test_.txt", 5, 2);

    std::size_t i{0};
    std::all_of(p.begin(), p.end(), [&](int v) {
      ++i;
      return (v == +1 || v == -1) && v == -1 * pattern.pattern()[i - 1];
    });
  }

  SUBCASE("Cutting pattern")
  {
    pattern.cut(-1, 1, 3, 1, 1, 2, 5);
    pattern.create_image("../tests/corrupted_files/",
                         "test.txt", 2, 5);

    for (unsigned int y{0}; y != 2; ++y) {
      CHECK(pattern.pattern()[y * 2] == -1);
    }
  }
}