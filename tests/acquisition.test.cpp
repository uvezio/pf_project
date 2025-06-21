#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../header/acquisition.hpp"

#include "doctest.h"

TEST_CASE("Testing load, resize and binarize functions on single images")
{
  SUBCASE("Loading a non existing image")
  {
    sf::Image image;
    image.create(0, 0);
    CHECK_THROWS(image = nn::load_image("../images/source_images/0.jpg"));
    CHECK(image.getSize().x == 0);
    CHECK(image.getSize().y == 0);
  }

  SUBCASE("Loading an existing image")
  {
    sf::Image image;
    image = nn::load_image("../images/source_images/1.jpg");
    REQUIRE((image.getSize().x >= 64 && image.getSize().y >= 64));
    CHECK(image.getSize().x == 720);
    CHECK(image.getSize().y == 720);
  }

  SUBCASE("Loading an image with size less than 64 pixels")
  {
    sf::Image image;
    CHECK_THROWS(image = nn::load_image("../images/source_images/-1.jpg"));
  }

  auto image = nn::load_image("../images/source_images/1.jpg");
  REQUIRE((image.getSize().x >= 64 && image.getSize().y >= 64));

  SUBCASE("Interpolating two sf::Uint8 values")
  {
    auto t = 0.5;
    sf::Uint8 v1{255};
    CHECK(nn::linear_interpolation(v1, v1, t) == 255);

    sf::Uint8 v2{0};
    CHECK(nn::linear_interpolation(v1, v2, t) == 127);
    t = 0.;
    CHECK(nn::linear_interpolation(v1, v2, t) == 255);
    t = 0.78;
    CHECK(nn::linear_interpolation(v1, v2, t) == 56);

    sf::Uint8 v3{201};
    sf::Uint8 v4{144};
    CHECK(nn::linear_interpolation(v4, v3, t) == 188);
    CHECK(nn::linear_interpolation(v3, v4, t) == 156);
    CHECK(nn::linear_interpolation(v3, v4, 0.5) == nn::linear_interpolation(v4, v3, 0.5));
  }

  SUBCASE("Interpolating two sf::Color values")
  {
    auto c1 = sf::Color::Black;
    auto c2 = sf::Color::White;
    auto t  = 0.;
    CHECK(nn::color_interpolation(c1, c2, t) == sf::Color(0, 0, 0));
    t = 0.5;
    CHECK(nn::color_interpolation(c1, c2, t) == sf::Color(127, 127, 127));

    auto c3 = sf::Color(104, 201, 45);
    auto c4 = sf::Color(3, 99, 34);
    CHECK(nn::color_interpolation(c3, c4, t) == sf::Color(53, 150, 39));
    t = 0.43;
    CHECK(nn::color_interpolation(c3, c4, t) == sf::Color(60, 157, 40));
  }

  SUBCASE("Resizing an image")
  {
    auto resized = nn::resize_image(image);
    REQUIRE((resized.getSize().x == 64 && resized.getSize().y == 64));
  }

  auto resized = nn::resize_image(image);
  REQUIRE((resized.getSize().x == 64 && resized.getSize().y == 64));

  SUBCASE("Binarizing a resized image")
  {
    auto pattern = nn::binarize_image(resized, "1.jpg");
    REQUIRE(pattern.size() == 64 * 64);
  }
}

TEST_CASE("Testing the class on multiple images")
{
  SUBCASE("Loading three different images")
  {
    std::vector<std::string> names{"1.jpg", "2.jpeg", "3.jpg"};
    nn::Acquisition acq{names};
    REQUIRE(acq.images().size() == 3);
    acq.load_images();
    for (auto const& image : acq.images()) {
      CHECK(image.image.getSize().x >= 64);
      CHECK(image.image.getSize().y >= 64);
    }
  }

  SUBCASE("Loading a non existing image and two existing ones")
  {
    std::vector<std::string> names{"0.jpg", "2.jpeg", "3.jpg"};
    nn::Acquisition acq{names};
    CHECK_THROWS(acq.load_images());
  }

  SUBCASE("Loading an image with size less than 64p and two correct ones")
  {
    std::vector<std::string> names{"-1.jpg", "2.jpeg", "3.jpg"};
    nn::Acquisition acq{names};
    CHECK_THROWS(acq.load_images());
  }

  std::vector<std::string> names = {"1.jpg", "2.jpeg", "3.jpg", "4.jpg"};
  nn::Acquisition acq{names};
  REQUIRE(acq.images().size() == 4);
  acq.load_images();

  SUBCASE("Resizing multiple images")
  {
    acq.resize_images();
    for (auto const& image : acq.images()) {
      CHECK(image.resized.getSize().x == 64);
      CHECK(image.resized.getSize().y == 64);
    }
  }

  SUBCASE("Binarizing multiple images")
  {
    acq.resize_images();
    acq.binarize_images();
    for (auto const& image : acq.images()) {
      CHECK(image.pattern.size() == 64 * 64);
    }
  }

  SUBCASE("Saving multiple binarized images")
  {
    acq.resize_images();
    acq.binarize_images();
    //acq.save_binarized_images();

    /*for (auto const& image : acq.images()) {
      auto path = "../patterns/" + image.name;
      std::ifstream infile{path};
      CHECK(infile);

      nn::Pattern pattern;
      int x;
      while (infile >> x) {
        pattern.add(x);
      }
      CHECK(pattern.size() == 64 * 64);

      path = "../images/binarized_images/" + image.name;
      sf::Image binary_image;
      CHECK(binary_image.loadFromFile(path));
      CHECK(binary_image.getSize().x == 64);
      CHECK(binary_image.getSize().y == 64);
    }*/
  }
}