#include "../header/acquisition.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace nn {

sf::Image load_image(std::string const& path)
{
  sf::Image image;
  if (!image.loadFromFile(path)) {
    throw std::runtime_error("Image \"" + path
                             + "\" not loaded successfully.\n");
  }
  if (image.getSize().x < 64 || image.getSize().y < 64) {
    throw std::runtime_error("Image \"" + path
                             + "\" size out of bounds.\nSize: "
                             + std::to_string(image.getSize().x) + "x"
                             + std::to_string(image.getSize().y));
  }
  assert(image.getSize().x >= 64 && image.getSize().y >= 64);

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
  assert(r >= 0 && r <= 255);
  assert(g >= 0 && g <= 255);
  assert(b >= 0 && b <= 255);

  return sf::Color(r, g, b);
}

sf::Image resize_image(sf::Image const& image)
{
  assert(image.getSize().x >= 64 && image.getSize().y >= 64);

  sf::Image resized;
  resized.create(64, 64);

  auto image_width  = image.getSize().x;
  auto image_height = image.getSize().y;

  for (unsigned int y{0}; y != 64; ++y) {
    for (unsigned int x{0}; x != 64; ++x) {
      auto image_x = static_cast<double>(x) * image_width / 64;
      auto image_y = static_cast<double>(y) * image_height / 64;

      auto x1 = static_cast<unsigned int>(image_x);
      auto y1 = static_cast<unsigned int>(image_y);

      auto x2 = std::min(x1 + 1, static_cast<unsigned int>(image_width - 1));
      auto y2 = std::min(y1 + 1, static_cast<unsigned int>(image_width - 1));

      assert(x2 >= x1 && x2 <= image_width - 1);
      assert(y2 >= y1 && y2 <= image_height - 1);

      auto dx = (image_x - x1);
      auto dy = (image_y - y1);

      if (x1 == x2) {
        dx = 0;
      }

      if (y1 == x2) {
        dy = 0;
      }

      assert(dx >= 0. && dx <= 1.);
      assert(dy >= 0. && dy <= 1.);

      auto c11 = image.getPixel(x1, y1);
      auto c12 = image.getPixel(x1, y2);
      auto c21 = image.getPixel(x2, y1);
      auto c22 = image.getPixel(x2, y2);

      sf::Color color =
          color_interpolation(color_interpolation(c11, c12, dy),
                              color_interpolation(c21, c22, dy), dx);

      resized.setPixel(x, y, color);
    }
  }

  assert(resized.getSize().x == 64 && resized.getSize().y == 64);

  return resized;
}

Pattern binarize_image(sf::Image const& resized, std::string const& name)
{
  assert(resized.getSize().x == 64 && resized.getSize().y == 64);

  Pattern pattern{name};
  for (unsigned int y{0}; y != 64; ++y) {
    for (unsigned int x{0}; x != 64; ++x) {
      auto color   = resized.getPixel(x, y);
      auto average = static_cast<int>((color.r + color.g + color.b) / 3);
      if (average > 127) {
        pattern.add(+1);
      } else {
        pattern.add(-1);
      }
    }
  }
  assert(pattern.size() == 64 * 64);

  return pattern;
}

std::vector<Image> Acquisition::initialize_images(std::vector<std::string> const& names)
{
  std::vector<Image> initialized;
  for (auto const& name : names) {
    initialized.push_back(Image{name, {}, {}, {}});
  }
  return initialized;
}

Acquisition::Acquisition(std::vector<std::string> const& names)
    : images_{initialize_images(names)}
{
  assert(images_.size() == names.size());

  for (auto const& image : images_) {
    if (all_loaded_) {
      assert(image.image.getSize().x >= 64 && image.image.getSize().y >= 64);
    }
    if (all_resized_) {
      assert(image.image.getSize().x == 64 && image.image.getSize().y == 64);
    }
    if (all_binarized_) {
      assert(image.pattern.size() == 64 * 64);
    }
  }
}

std::vector<Image> Acquisition::images() const
{
  return images_;
}

void Acquisition::load_images()
{
  for (auto& image : images_) {
    std::string path{"../images/source_images/" + image.name};
    image.image = load_image(path);
  }
  all_loaded_ = true;
}

void Acquisition::resize_images()
{
  for (auto& image : images_) {
    image.resized = resize_image(image.image);
  }
  all_resized_ = true;
}

void Acquisition::binarize_images()
{
  for (auto& image : images_) {
    image.pattern = binarize_image(image.resized, image.name);
  }
  all_binarized_ = true;
}

void Acquisition::save_binarized_images() const
{
  for (auto const& image : images_) {
    image.pattern.save_to_file();
    image.pattern.save_image(64, 64);
  }
}

} // namespace nn
