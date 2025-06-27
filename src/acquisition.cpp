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

Pattern binarize_image(sf::Image const& resized, std::string const& name,
                       unsigned int width, unsigned int height,
                       sf::Uint8 threshold)
{
  assert(resized.getSize().x == width && resized.getSize().y == height);

  Pattern pattern{name};
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

std::vector<Image>
Acquisition::initialize_images(std::vector<std::string> const& names)
{
  std::vector<Image> initialized;
  for (auto const& name : names) {
    initialized.push_back(Image{name, {}, {}, {}});
  }
  return initialized;
}

Acquisition::Acquisition(std::vector<std::string> const& names)
    : images_{initialize_images(names)}
    , source_directory_{"../images/source_images/"}
    , binarized_directory_{"../images/binarized_images/"}
{
  assert(images_.size() == names.size());

  for (auto const& image : images_) {
    assert(image.image.getSize().x == 0 && image.image.getSize().y == 0);
    assert(image.image.getSize().x == 0 && image.image.getSize().y == 0);
    assert(image.pattern.size() == 0);
    (void)image;
  }
}

const std::vector<Image>& Acquisition::images() const
{
  return images_;
}

void Acquisition::load_images()
{
  for (auto& image : images_) {
    auto path   = source_directory_ + image.name;
    image.image = load_image(path, 64, 64);
  }
}

void Acquisition::resize_images()
{
  for (auto& image : images_) {
    image.resized = resize_image(image.image, 64, 64);
  }
}

void Acquisition::binarize_images()
{
  for (auto& image : images_) {
    auto name = image.name.substr(0, image.name.rfind('.'));
    name += ".txt";
    image.pattern = binarize_image(image.resized, name, 64, 64, 127);
  }
}

void Acquisition::save_binarized_images() const
{
  for (auto const& image : images_) {
    image.pattern.save_to_file();
    Pattern pattern;
    pattern.load_from_file(image.pattern.name(), 64 * 64);
    pattern.create_image(64, 64, binarized_directory_);
  }
}

} // namespace nn
