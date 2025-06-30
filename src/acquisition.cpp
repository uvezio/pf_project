#include "../include/acquisition.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace nn {

sf::Image load_image(std::string const& path, unsigned int min_width,
                     unsigned int min_height)
{
  sf::Image image;
  if (!image.loadFromFile(path)) {
    throw std::runtime_error("Image \"" + path + "\" not loaded successfully.");
  }
  if (image.getSize().x < min_width || image.getSize().y < min_height) {
    throw std::runtime_error(
        "Image \"" + path + "\" size out of bounds.\nMinimum size: "
        + std::to_string(min_width) + "x" + std::to_string(min_height)
        + "\nActual size: " + std::to_string(image.getSize().x) + "x"
        + std::to_string(image.getSize().y));
  }
  assert(image.getSize().x >= min_width && image.getSize().y >= min_height);

  return image;
}

sf::Uint8 linear_interpolation(sf::Uint8 a, sf::Uint8 b, double t)
{
  assert(t >= 0. && t <= 1.);
  auto interpolation = static_cast<sf::Uint8>(a + t * (b - a));
  assert(interpolation >= std::min(a, b) && interpolation <= std::max(a, b));

  return interpolation;
}

sf::Color color_interpolation(sf::Color const& c1, sf::Color const& c2,
                              double t)
{
  assert(t >= 0. && t <= 1.);
  auto r = linear_interpolation(c1.r, c2.r, t);
  auto g = linear_interpolation(c1.g, c2.g, t);
  auto b = linear_interpolation(c1.b, c2.b, t);

  return sf::Color(r, g, b);
}

sf::Image resize_image(sf::Image const& image, unsigned int width,
                       unsigned int height)
{
  assert(image.getSize().x >= width && image.getSize().y >= height);

  sf::Image resized;
  resized.create(width, height);

  auto image_width  = image.getSize().x;
  auto image_height = image.getSize().y;

  for (unsigned int y{0}; y != height; ++y) {
    auto image_y = static_cast<double>(y) * image_height / height;
    auto y1      = static_cast<unsigned int>(image_y);
    auto y2      = std::min(y1 + 1, static_cast<unsigned int>(image_width - 1));
    assert(y2 >= y1 && y2 <= image_height - 1);
    auto dy = (image_y - y1);
    assert(dy >= 0. && dy <= 1.);

    for (unsigned int x{0}; x != width; ++x) {
      auto image_x = static_cast<double>(x) * image_width / width;
      auto x1      = static_cast<unsigned int>(image_x);
      auto x2 = std::min(x1 + 1, static_cast<unsigned int>(image_width - 1));
      assert(x2 >= x1 && x2 <= image_width - 1);
      auto dx = (image_x - x1);
      assert(dx >= 0. && dx <= 1.);

      auto c11 = image.getPixel(x1, y1);
      auto c12 = image.getPixel(x1, y2);
      auto c21 = image.getPixel(x2, y1);
      auto c22 = image.getPixel(x2, y2);

      auto color = color_interpolation(color_interpolation(c11, c12, dy),
                                       color_interpolation(c21, c22, dy), dx);

      resized.setPixel(x, y, color);
    }
  }

  assert(resized.getSize().x == width && resized.getSize().y == height);

  return resized;
}

Pattern binarize_image(sf::Image const& resized, unsigned int width,
                       unsigned int height, sf::Uint8 threshold)
{
  assert(resized.getSize().x == width && resized.getSize().y == height);

  Pattern pattern;

  for (unsigned int y{0}; y != height; ++y) {
    for (unsigned int x{0}; x != width; ++x) {
      auto color   = resized.getPixel(x, y);
      auto average = (color.r + color.g + color.b) / 3;
      pattern.add(average > threshold ? +1 : -1);
    }
  }
  assert(pattern.size() == width * height);

  return pattern;
}

void Acquisition::validate_source_directory_() const
{
  if (!std::filesystem::exists(source_directory_)) {
    throw std::runtime_error("Directory " + source_directory_.string()
                             + " not found.");
  }
  if (!std::filesystem::is_directory(source_directory_)) {
    throw std::runtime_error("Path " + source_directory_.string()
                             + " is not a directory.");
  }
  if (std::filesystem::is_empty(source_directory_)) {
    throw std::runtime_error("Directory " + source_directory_.string()
                             + " is empty.");
  }

  const std::vector<std::string> valid_extensions{".png", ".jpg", ".jpeg"};

  for (auto const& file :
       std::filesystem::directory_iterator(source_directory_)) {
    if (!file.is_regular_file()) {
      throw std::runtime_error("File " + source_directory_.string()
                               + file.path().filename().string()
                               + " is not regular file.");
    }

    if (valid_extensions.end()
        == std::find(valid_extensions.begin(), valid_extensions.end(),
                     file.path().extension())) {
      throw std::runtime_error("File " + source_directory_.string()
                               + file.path().filename().string()
                               + " has not valid extension.");
    }
  }
}

void Acquisition::validate_binarized_directory_() const
{
  if (!std::filesystem::exists(binarized_directory_)) {
    std::filesystem::create_directory(binarized_directory_);
  }
  if (!std::filesystem::is_directory(binarized_directory_)) {
    throw std::runtime_error("Path " + binarized_directory_.string()
                             + " is not a directory.");
  }
  if (!std::filesystem::is_empty(binarized_directory_)) {
    for (auto const& file :
         std::filesystem::directory_iterator(binarized_directory_)) {
      std::filesystem::remove_all(file.path());
    }
  }
}

void Acquisition::validate_patterns_directory_() const
{
  if (!std::filesystem::exists(patterns_directory_)) {
    std::filesystem::create_directory(patterns_directory_);
  }
  if (!std::filesystem::is_directory(patterns_directory_)) {
    throw std::runtime_error("Path " + patterns_directory_.string()
                             + " is not a directory.");
  }
  if (!std::filesystem::is_empty(patterns_directory_)) {
    for (auto const& file :
         std::filesystem::directory_iterator(patterns_directory_)) {
      std::filesystem::remove_all(file.path());
    }
  }

  nn::Pattern::set_directory(patterns_directory_);
}

Acquisition::Acquisition(std::filesystem::path const& dir)
    : source_directory_{"../" + dir.string() + "images/source_images/"}
    , binarized_directory_{"../" + dir.string() + "images/binarized_images/"}
    , patterns_directory_{"../" + dir.string() + "patterns/"}
{
  validate_source_directory_();
  validate_binarized_directory_();
  validate_patterns_directory_();
}

Acquisition::Acquisition()
    : Acquisition::Acquisition("")
{}

const std::vector<Pattern>& Acquisition::patterns() const
{
  return patterns_;
}

void Acquisition::acquire_and_save_patterns()
{
  for (auto const& file :
       std::filesystem::directory_iterator(source_directory_)) {
    auto image = load_image(file.path().string(), 64, 64);
    assert(image.getSize().x >= 64 && image.getSize().y >= 64);

    image = resize_image(image, 64, 64);
    assert(image.getSize().x == 64 && image.getSize().y == 64);

    auto name = file.path().filename().replace_extension(".txt");
    assert(name.extension() == ".txt");

    auto pattern = binarize_image(image, 64, 64, 127);
    assert(pattern.size() == 64 * 64);

    pattern.save_to_file(name, 64 * 64);
    patterns_.push_back(pattern);
  }
}

void Acquisition::save_binarized_images() const
{
  for (auto const& file :
       std::filesystem::directory_iterator(patterns_directory_)) {
        Pattern pattern;
        auto name = file.path().filename();
        pattern.load_from_file(name, 64 * 64);
        pattern.create_image(binarized_directory_, name, 64, 64);
  }
}

/*Acquisition::Acquisition(std::string const& dir)
    : source_directory_{"../" + dir + "images/source_images/"}
    , binarized_directory_{"../" + dir + "images/binarized_images/"}
{
  assert(images_.size() == 0);
  valid_source_directory_();
  valid_binarized_directory_();
}

Acquisition::Acquisition()
    : Acquisition::Acquisition("")
{}

const std::vector<Image>& Acquisition::images() const
{
  return images_;
}

void Acquisition::acquire_images()
{
  for (auto const& file :
       std::filesystem::directory_iterator(source_directory_)) {
    auto name = file.path().filename().string();

    auto image = load_image(file.path(), 64, 64);
    assert(image.getSize().x >= 64 && image.getSize().y >= 64);

    auto resized = resize_image(image, 64, 64);
    assert(resized.getSize().x == 64 && resized.getSize().y == 64);

    auto pattern_name = name.substr(0, name.rfind('.')) + ".txt";
    auto pattern      = binarize_image(resized, pattern_name, 64, 64, 127);
    assert(pattern.size() == 64 * 64);

    images_.push_back({name, image, pattern});
  }
}*/

} // namespace nn
