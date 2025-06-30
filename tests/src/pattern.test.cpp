#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../../include/pattern.hpp"

#include "../doctest.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

TEST_CASE("Testing the add method")
{
  nn::Pattern pattern{};

  nn::Pattern::set_directory("../tests/patterns/");

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
    pattern.save_to_file("empty.txt", 0);
    CHECK(std::filesystem::is_regular_file("../tests/patterns/empty.txt"));
    pattern.load_from_file("empty.txt", 0);
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

    pattern.save_to_file("test.txt", 10);
    CHECK(std::filesystem::is_regular_file("../tests/patterns/test.txt"));

    pattern.load_from_file("test.txt", 10);
    CHECK(pattern.size() == 10);
  }

  nn::Pattern pattern;
  REQUIRE(pattern.size() == 0);

  std::vector<int> values{+1, -1, +1, +1, 0, -1, +1, -1, +1};
  std::ofstream incorrect{"../tests/patterns/incorrect.txt"};
  for (auto v : values) {
    incorrect << v << ' ';
  }

  SUBCASE("Loading an incorrect pattern")
  {
    CHECK_THROWS(pattern.load_from_file("incorrect.txt", 9));
  }

  SUBCASE("Loading over-sized and under-sized patterns")
  {
    CHECK_THROWS(pattern.load_from_file("test.txt", 8));
    CHECK_THROWS(pattern.load_from_file("test.txt", 12));
    CHECK_THROWS(pattern.load_from_file("empty.txt", 1));
  }

  SUBCASE("Loading a pattern")
  {
    pattern.load_from_file("test.txt", 10);
    CHECK(pattern.size() == 10);
  }

  SUBCASE("Overwriting an existing pattern with a loaded one")
  {
    pattern.add(1);
    REQUIRE(pattern.size() == 1);
    pattern.load_from_file("test.txt", 10);
    CHECK(pattern.size() == 10);
  }
}

TEST_CASE("Testing creation of images")
{
  nn::Pattern pattern;
  pattern.load_from_file("test.txt", 10);

  SUBCASE("Creating image in an existing directory")
  {
    sf::Image image;

    pattern.create_image("../tests/images/binarized_images/", "test.txt", 5, 2);
    image.loadFromFile("../tests/images/binarized_images/test.jpg");
    CHECK(image.getSize().x == 5);
    CHECK(image.getSize().y == 2);

    pattern.create_image("../tests/images/binarized_images/", "test_.txt", 2, 5);
    image.loadFromFile("../tests/images/binarized_images/test_.jpg");
    CHECK(image.getSize().x == 2);
    CHECK(image.getSize().y == 5);
  }
}