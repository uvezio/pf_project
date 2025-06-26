#include "../include/pattern.hpp"

#include <cassert>
#include <fstream>
#include <stdexcept>

namespace nn {

const std::string Pattern::directory_{"../patterns/"};

Pattern::Pattern(std::string const& name)
    : pattern_{}
    , name_{name}
{}

Pattern::Pattern()
    : Pattern::Pattern("")
{}

std::size_t Pattern::size() const
{
  return pattern_.size();
}

const std::string& Pattern::name() const
{
  return name_;
}

void Pattern::add(int value)
{
  pattern_.push_back(value);
}

void Pattern::save_to_file() const
{
  std::ofstream outfile{directory_ + name_};

  if (!outfile) {
    throw std::runtime_error("File \"" + directory_ + name_
                             + "\" not opened successfully.");
  }

  for (auto value : pattern_) {
    assert(value == +1 || value == -1);
    if (!(outfile << value << ' ')) {
      throw std::runtime_error("File \"" + directory_ + name_
                               + "\" not written successfully.");
    }
  }
}

void Pattern::save_image(unsigned int width, unsigned int height,
                         std::string const& directory) const
{
  sf::Image image;
  image.create(width, height);

  for (unsigned int y = 0; y < height; ++y) {
    for (unsigned int x = 0; x < width; ++x) {
      std::size_t i   = y * width + x;
      auto value      = pattern_[i];
      sf::Color color = value == +1 ? sf::Color::White : sf::Color::Black;
      image.setPixel(x, y, color);
    }
  }

  auto name = name_.substr(0, name_.rfind('.'));
  name += ".jpg";
  image.saveToFile(directory + name);
}

} // namespace nn