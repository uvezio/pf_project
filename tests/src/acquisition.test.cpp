#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../../include/acquisition.hpp"

#include "../doctest.h"

#include <filesystem>
#include <fstream>

TEST_CASE("Testing load, resize and binarize functions on single images")
{
  sf::Image image;

  SUBCASE("Loading non existing images")
  {
    image.create(0, 0);
    CHECK_THROWS(image =
                     nn::load_image("../tests/images/source_images/", 64, 64));
    CHECK_THROWS(image = nn::load_image(
                     "../tests/images/source_images/non_existing.jpg", 64, 64));
    CHECK(image.getSize().x == 0);
    CHECK(image.getSize().y == 0);
  }

  SUBCASE("Loading an image with an invalid extension")
  {
    std::ofstream inv{"../tests/images/source_images/invalid_extension.pdf"};
    CHECK_THROWS(
        image = nn::load_image(
            "../tests/images/source_images/invalid_extension.pdf", 64, 64));
  }

  SUBCASE("Loading an existing image")
  {
    image = nn::load_image("../tests/images/source_images/1.jpg", 64, 64);
    REQUIRE((image.getSize().x >= 64 && image.getSize().y >= 64));
    CHECK(image.getSize().x == 720);
    CHECK(image.getSize().y == 720);
    CHECK_THROWS(image = nn::load_image("../tests/images/source_images/1.jpg",
                                        721, 720));
  }

  SUBCASE("Loading an image with size less than 64 pixels")
  {
    image.create(10, 10, sf::Color::White);
    image.saveToFile("../tests/images/source_images/under_sized.jpg");
    CHECK_THROWS(image = nn::load_image(
                     "../tests/images/source_images/under_sized.jpg", 64, 64));
  }

  image = nn::load_image("../tests/images/source_images/1.jpg", 64, 64);
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
    CHECK(nn::linear_interpolation(v3, v4, 0.5)
          == nn::linear_interpolation(v4, v3, 0.5));
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
    auto resized = nn::resize_image(image, 64, 64);
    REQUIRE((resized.getSize().x == 64 && resized.getSize().y == 64));
  }

  auto resized = nn::resize_image(image, 64, 64);
  REQUIRE((resized.getSize().x == 64 && resized.getSize().y == 64));

  SUBCASE("Binarizing a resized image")
  {
    auto pattern = nn::binarize_image(resized, "1.txt", 64, 64, 127);
    REQUIRE(pattern.size() == 64 * 64);
    REQUIRE(pattern.name() == "1.txt");
  }
}

TEST_CASE("Testing the Acquisition class on invalid directories")
{
  SUBCASE("Non existing source directory "
          "(\"../non_existing/images/source_images/\")")
  {
    REQUIRE(!std::filesystem::exists("../non_existing/images/source_images/"));
    CHECK_THROWS(nn::Acquisition("non_existing/"));
    REQUIRE(
        !std::filesystem::exists("../non_existing/images/binarized_images/"));
  }

  SUBCASE("Source directory with a non regular file "
          "(\"../tests/images/source_images/dir/\")")
  {
    std::filesystem::create_directory("../tests/images/source_images/dir/");
    CHECK_THROWS(nn::Acquisition("tests/"));
    std::filesystem::remove("../tests/images/source_images/dir/");
  }

  SUBCASE("Source directory containing files with invalid extensions "
          "(\"../tests/images/source_images/0.pdf\")")
  {
    CHECK_THROWS(nn::Acquisition("tests/"));
    std::filesystem::remove(
        "../tests/images/source_images/invalid_extension.pdf");
  }

  SUBCASE("Non existing binarized directory")
  {
    REQUIRE(std::filesystem::exists("../tests/images/binarized_images/"));
    for (auto const& file : std::filesystem::directory_iterator(
             "../tests/images/binarized_images/")) {
      std::filesystem::remove_all(file);
    }
    std::filesystem::remove("../tests/images/binarized_images/");
    REQUIRE(!std::filesystem::exists("../tests/images/binarized_images/"));
    nn::Acquisition acq{"tests/"};
    CHECK(std::filesystem::exists("../tests/images/binarized_images/"));
  }

  SUBCASE("Not empty binarized directory")
  {
    std::ofstream img{"../tests/images/binarized_images/0.jpg"};
    REQUIRE(!std::filesystem::is_empty("../tests/images/binarized_images/"));
    nn::Acquisition acq{"tests/"};
    CHECK(std::filesystem::is_empty("../tests/images/binarized_images/"));
  }
}

TEST_CASE("Testing acquire_images()")
{
  nn::Acquisition acq{"tests/"};

  SUBCASE("Acquiring an under-sized image \"(under_sized.jpg)\"")
  {
    CHECK_THROWS(acq.acquire_images());
    std::filesystem::remove("../tests/images/source_images/under_sized.jpg");
    REQUIRE(!std::filesystem::exists(
        "../tests/images/source_images/under_sized.jpg"));
  }

  SUBCASE("Acquiring all the images in the directory")
  {
    acq.acquire_images();
    CHECK(acq.images().size() == 4);
    for (auto const& image : acq.images()) {
      CHECK(image.image.getSize().x >= 64);
      CHECK(image.image.getSize().y >= 64);
      CHECK(image.pattern.size() == 64 * 64);
      CHECK(image.pattern.name()
            == image.name.substr(0, image.name.rfind('.')) + ".txt");
    }
  }

  SUBCASE("Saving multiple binarized images")
  {
    acq.acquire_images();
    acq.save_binarized_images();

    for (auto const& image : acq.images()) {
      auto name =
          image.pattern.name().substr(0, image.pattern.name().rfind('.'))
          + ".jpg";
      sf::Image binary_image;
      CHECK(binary_image.loadFromFile("../tests/images/binarized_images/"
                                      + name));
      CHECK(binary_image.getSize().x == 64);
      CHECK(binary_image.getSize().y == 64);

      nn::Pattern pattern;
      pattern.load_from_file(image.pattern.name(), 64 * 64);
      CHECK(pattern.size() == 64 * 64);
      CHECK(pattern.name() == image.pattern.name());
    }
  }
}