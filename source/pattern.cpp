#include "../header/pattern.hpp"

#include <fstream>
#include <stdexcept>

namespace nn {

Pattern::Pattern(std::string const& name)
    : name_{name}
{}

Pattern::Pattern()
    : name_{}
{}

std::size_t Pattern::size() const
{
  return pattern_.size();
}

/*std::string Pattern::name() const
{
  return name_;
}*/

void Pattern::add(int value)
{
  pattern_.push_back(value);
}

void Pattern::save_to_file() const
{

  std::string path{"../pattern/" + name_.substr(0, 1) + ".txt"};
  std::ofstream outfile{path};

  if (!outfile) {
    throw std::runtime_error("File \"../patterns/" + name_.substr(0, 1)
                             + ".txt\" not opened successfully.");
  }

  for (auto value : pattern_) {
    if (!(outfile << value << ' ')) {
      throw std::runtime_error("");
    }
  }
}

void Pattern::save_image(unsigned int width, unsigned int height) const
{
  sf::Image image;
  image.create(width, height);

  for (unsigned int y = 0; y < height; ++y) {
    for (unsigned int x = 0; x < width; ++x) {
      std::size_t i = y * width + x;
      auto value    = pattern_[i];
      sf::Color color;
      if (value == +1) {
        color = sf::Color::White;
      } else {
        color = sf::Color::Black;
      }
      image.setPixel(x, y, color);
    }
  }

  image.saveToFile("../binarized_images/" + name_);
}

} // namespace nn