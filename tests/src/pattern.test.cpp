#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../../include/pattern.hpp"

#include "../doctest.h"

#include <SFML/Graphics.hpp>

TEST_CASE("Testing the add method")
{
  nn::Pattern pattern{};

  REQUIRE(pattern.size() == 0);
  REQUIRE(pattern.name() == "");

  SUBCASE("Adding three admissible values")
  {
    pattern.add(-1);
    pattern.add(-1);
    pattern.add(+1);
    CHECK(pattern.size() == 3);
  }

  auto pattern1 = nn::Pattern("file.txt");
  REQUIRE(pattern1.size() == 0);
  REQUIRE(pattern1.name() == "file.txt");

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
    CHECK(pattern.name() == "file.txt");
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
    nn::Pattern pattern{"empty.txt"};
    REQUIRE(pattern.size() == 0);
    REQUIRE(pattern.name() == "empty.txt");
    pattern.save_to_file();
    pattern.load_from_file("empty.txt", 0);
    CHECK(pattern.size() == 0);
  }

  SUBCASE("Saving pattern to file and loading the same pattern")
  {
    nn::Pattern pattern{"test.txt"};
    std::vector<int> values{+1, -1, +1, +1, +1, -1, +1, -1, +1, -1};
    for (auto v : values) {
      pattern.add(v);
    }
    REQUIRE(pattern.size() == 10);
    REQUIRE(pattern.name() == "test.txt");

    pattern.save_to_file();

    pattern.load_from_file("test.txt", 10);
    CHECK(pattern.size() == 10);
    CHECK(pattern.name() == "test.txt");
  }

  nn::Pattern pattern;
  REQUIRE(pattern.size() == 0);
  REQUIRE(pattern.name() == "");

  SUBCASE("Saving an incorrect pattern")
  {
    CHECK_THROWS(pattern.save_to_file());
  }

  SUBCASE("Loading a non existing pattern")
  {
    CHECK_THROWS(pattern.load_from_file("../patterns/", 64 * 64));
    CHECK_THROWS(pattern.load_from_file("../patterns/nonex.txt", 64 * 64));
  }

  SUBCASE("Loading an incorrect pattern")
  {
    CHECK_THROWS(pattern.load_from_file("../patterns/inc.txt", 6));
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
    CHECK(pattern.name() == "test.txt");
  }

  SUBCASE("Overwriting an existing pattern with a loaded one")
  {
    pattern.add(1);
    REQUIRE(pattern.size() == 1);
    pattern.load_from_file("test.txt", 10);
    CHECK(pattern.size() == 10);
    CHECK(pattern.name() == "test.txt");
  }
}

TEST_CASE("Testing creation of images")
{
  nn::Pattern pattern;
  pattern.load_from_file("test.txt", 10);

  SUBCASE("Creating image in an existing directory")
  {
    sf::Image image;

    pattern.create_image(5, 2, "../images/binarized_images/");
    image.loadFromFile("../images/binarized_images/test.jpg");
    CHECK(image.getSize().x == 5);
    CHECK(image.getSize().y == 2);

    pattern.create_image(2, 5, "../images/binarized_images/");
    image.loadFromFile("../images/binarized_images/test.jpg");
    CHECK(image.getSize().x == 2);
    CHECK(image.getSize().y == 5);
  }

  SUBCASE("Creating image in a non existing directory")
  {
    CHECK_THROWS(pattern.create_image(5, 2, "../images/binarized/"));
  }
}