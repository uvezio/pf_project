#include "../include/pattern.hpp"

#include <SFML/Graphics.hpp>
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
                             + "\" not created successfully.");
  }

  for (auto value : pattern_) {
    assert(value == +1 || value == -1);
    if (!(outfile << value << ' ')) {
      throw std::runtime_error("File \"" + directory_ + name_
                               + "\" not written successfully.");
    }
  }
}

void Pattern::load_from_file(std::string const& name, std::size_t size)
{
  pattern_.clear();
  name_ = name;

  std::ifstream infile{directory_ + name};

  if (!infile) {
    throw std::runtime_error("File \"" + directory_ + name_
                             + "\" not opened successfully.");
  }

  int value;
  while (infile >> value) {
    if (value != +1 && value != -1) {
      throw std::runtime_error("Error in file \"" + directory_ + name_
                               + "\".\nEntries must be +1 or -1.");
    }
    pattern_.push_back(value);
  }

  if (pattern_.size() != size) {
    throw std::runtime_error(
        "Error in file \"" + directory_ + name_
        + "\".\nNumber of entries must be: " + std::to_string(size)
        + "\nActual number of entries: " + std::to_string(pattern_.size()));
  }

  assert(name_ == name);
  assert(pattern_.size() == size);
}

void Pattern::create_image(unsigned int width, unsigned int height,
                           std::string const& directory) const
{
  assert(pattern_.size() == width * height);

  sf::Image image;
  image.create(width, height);

  for (unsigned int y{0}; y < height; ++y) {
    for (unsigned int x{0}; x < width; ++x) {
      auto index = y * width + x;
      auto value = pattern_[index];
      auto color = value == +1 ? sf::Color::White : sf::Color::Black;
      image.setPixel(x, y, color);
    }
  }

  auto name = name_.substr(0, name_.rfind('.'));
  name += ".jpg";

  if (!image.saveToFile(directory + name)) {
    throw std::runtime_error("Image \"" + directory + name
                             + "\" not created successfully.");
  }
}

} // namespace nn