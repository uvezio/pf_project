// All relative paths are relative to the "build/" directory

// This path is the only one relative to "acquisition.cpp"
#include "../include/acquisition.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <string>

namespace nn {

sf::Image load_image(std::filesystem::path const& path, unsigned int min_width,
                     unsigned int min_height)
{
  assert(std::filesystem::is_regular_file(path));
  auto ext = path.extension();
  // By assumption the only extensions allowed are .jpg, .jpeg, .png
  assert(ext == ".jpg" || ext == ".jpeg" || ext == ".png");

  sf::Image image;
  if (!image.loadFromFile(path)) {
    throw std::runtime_error("Image \"" + path.string()
                             + "\" not loaded successfully.");
  }
  if (image.getSize().x < min_width || image.getSize().y < min_height) {
    throw std::runtime_error(
        "Image \"" + path.string() + "\" size out of bounds.\nMinimum size: "
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
    // image_y corresponds to where y would be in the original image
    auto image_y = static_cast<double>(y) * image_height / height;
    auto y1      = static_cast<unsigned int>(image_y);
    // y2 must be within the image boundaries
    auto y2 = std::min(y1 + 1, static_cast<unsigned int>(image_width - 1));
    assert(y2 >= y1 && y2 <= image_height - 1);
    // dy is the weight to pass to the interpolation function
    auto dy = (image_y - y1);
    assert(dy >= 0. && dy <= 1.);

    for (unsigned int x{0}; x != width; ++x) {
      // Analogous to y
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

      // color_interpolation() is not commutative with respect to colors
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
    throw std::runtime_error("Directory \"" + source_directory_.string()
                             + "\" not found.");
  }
  if (!std::filesystem::is_directory(source_directory_)) {
    throw std::runtime_error("Path \"" + source_directory_.string()
                             + "\" is not a directory.");
  }
  if (std::filesystem::is_empty(source_directory_)) {
    throw std::runtime_error("Directory \"" + source_directory_.string()
                             + "\" is empty.");
  }

  for (auto const& file :
       std::filesystem::directory_iterator(source_directory_)) {
    if (!file.is_regular_file()) {
      throw std::runtime_error("File \"" + source_directory_.string()
                               + file.path().filename().string()
                               + "\" is not a regular file.");
    }
    if (extensions_allowed_.end()
        == std::find(extensions_allowed_.begin(), extensions_allowed_.end(),
                     file.path().extension())) {
      throw std::runtime_error("File \"" + source_directory_.string()
                               + file.path().filename().string()
                               + "\" has an invalid extension.");
    }
  }
}

void Acquisition::configure_output_directories_() const
{
  for (auto const& output_dir : {binarized_directory_, patterns_directory_}) {
    if (!std::filesystem::exists(output_dir)) {
      std::filesystem::create_directory(output_dir);
    }
    if (!std::filesystem::is_directory(output_dir)) {
      throw std::runtime_error("Path \"" + output_dir.string()
                               + "\" is not a directory.");
    }
    if (!std::filesystem::is_empty(output_dir)) {
      for (auto const& file : std::filesystem::directory_iterator(output_dir)) {
        std::filesystem::remove_all(file.path());
      }
    }
  }
}

// base_directory can only be "" or "tests/"
Acquisition::Acquisition(std::filesystem::path const& base_directory)
    : source_directory_{"../" + base_directory.string()
                        + "images/source_images/"}
    , binarized_directory_{"../" + base_directory.string()
                           + "images/binarized_images/"}
    , patterns_directory_{"../" + base_directory.string() + "patterns/"}
    , extensions_allowed_{".jpg", ".jpeg", ".png"} // By assumption
{
  assert(extensions_allowed_.size() != 0);

  validate_source_directory_();
  configure_output_directories_();

  assert(std::filesystem::is_directory(source_directory_)
         && !std::filesystem::is_empty(source_directory_));
  assert(std::filesystem::is_directory(binarized_directory_)
         && std::filesystem::is_empty(binarized_directory_));
  assert(std::filesystem::is_directory(patterns_directory_)
         && std::filesystem::is_empty(patterns_directory_));
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
    assert(file.is_regular_file());
    assert(extensions_allowed_.end()
           != std::find(extensions_allowed_.begin(), extensions_allowed_.end(),
                        file.path().extension()));

    auto image = load_image(file.path().string(), 64, 64);
    assert(image.getSize().x >= 64 && image.getSize().y >= 64);

    image = resize_image(image, 64, 64);
    assert(image.getSize().x == 64 && image.getSize().y == 64);

    auto name = file.path().filename().replace_extension(".txt");
    assert(name.extension() == ".txt");

    auto pattern = binarize_image(image, 64, 64, 127);
    assert(pattern.size() == 64 * 64);

    pattern.save_to_file(patterns_directory_, name, 64 * 64);
    patterns_.push_back(pattern);
  }
}

void Acquisition::save_binarized_images() const
{
  for (auto const& file :
       std::filesystem::directory_iterator(patterns_directory_)) {
    assert(file.is_regular_file());
    assert(file.path().extension() == ".txt");

    Pattern pattern;
    pattern.load_from_file(patterns_directory_, file.path().filename(),
                           64 * 64);
    pattern.create_image(binarized_directory_, file.path().filename(), 64, 64);
  }
}

} // namespace nn
