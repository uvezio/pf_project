#include "../include/pattern.hpp"

#include <SFML/Graphics.hpp>
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <string>

namespace nn {

std::filesystem::path Pattern::patterns_directory_{"../"};
bool Pattern::initialized_{false};

Pattern::Pattern()
    : pattern_{}
{
  assert(std::filesystem::is_directory(patterns_directory_));
  assert(pattern_.size() == 0);
}

void Pattern::set_directory(std::filesystem::path const& patterns_directory)
{
  assert(std::filesystem::is_directory(patterns_directory));
  if (!initialized_) {
    patterns_directory_ = patterns_directory;
    initialized_        = true;
  }
}

const std::vector<int>& Pattern::pattern() const
{
  return pattern_;
}

std::size_t Pattern::size() const
{
  return pattern_.size();
}

void Pattern::add(int value)
{
  assert(value == +1 || value == -1);
  pattern_.push_back(value);
}

void Pattern::save_to_file(std::filesystem::path const& name,
                           std::size_t size) const
{
  assert(std::filesystem::is_directory(patterns_directory_));

  auto path = patterns_directory_;
  path.replace_filename(name);
  assert(path.extension() == ".txt");

  std::ofstream outfile{path};

  if (!outfile) {
    throw std::runtime_error("File \"" + path.string()
                             + "\" not created successfully.");
  }

  assert(std::filesystem::is_regular_file(path));

  assert(pattern_.size() == size);

  for (auto value : pattern_) {
    assert(value == +1 || value == -1);
    if (!(outfile << value << ' ')) {
      throw std::runtime_error("File \"" + path.string()
                               + "\" not written successfully.");
    }
  }

  outfile.close();
  // if not closed here it could be written after the is_empty() check

  if (size != 0) {
    assert(!std::filesystem::is_empty(path));
  } else {
    assert(std::filesystem::is_empty(path));
  }
}

void Pattern::load_from_file(std::filesystem::path const& name,
                             std::size_t size)
{
  pattern_.clear();

  assert(std::filesystem::is_directory(patterns_directory_));

  auto path = patterns_directory_;
  path.replace_filename(name);

  assert(std::filesystem::is_regular_file(path));
  assert(path.extension() == ".txt");
  if (size != 0) {
    assert(!std::filesystem::is_empty(path));
  } else {
    assert(std::filesystem::is_empty(path));
  }

  std::ifstream infile{path};

  if (!infile) {
    throw std::runtime_error("File \"" + path.string()
                             + "\" not opened successfully.");
  }

  int value;
  while (infile >> value) {
    if (value != +1 && value != -1) {
      throw std::runtime_error("Error in file \"" + path.string()
                               + "\".\nEntries must be +1 or -1.");
    }
    pattern_.push_back(value);
  }

  if (pattern_.size() != size) {
    throw std::runtime_error(
        "Error in file \"" + path.string()
        + "\".\nNumber of entries must be: " + std::to_string(size)
        + "\nActual number of entries: " + std::to_string(pattern_.size()));
  }

  assert(pattern_.size() == size);
}

void Pattern::create_image(std::filesystem::path const& binarized_directory,
                           std::filesystem::path const& pattern_name,
                           unsigned int width, unsigned int height) const
{
  assert(std::filesystem::is_directory(binarized_directory));

  auto path = binarized_directory;
  path.replace_filename(pattern_name);
  assert(path.extension() == ".txt");
  path.replace_extension(".jpg");

  assert(pattern_.size() == width * height);

  sf::Image image;
  image.create(width, height);

  for (unsigned int y{0}; y < height; ++y) {
    for (unsigned int x{0}; x < width; ++x) {
      auto index = y * width + x;
      auto value = pattern_[index];
      assert(value == +1 || value == -1);
      auto color = value == +1 ? sf::Color::White : sf::Color::Black;
      image.setPixel(x, y, color);
    }
  }

  if (!image.saveToFile(path)) {
    throw std::runtime_error("Image \"" + path.string()
                             + "\" not created successfully.");
  }

  assert(std::filesystem::is_regular_file(path));
  if (width * height != 0) {
    assert(!std::filesystem::is_empty(path));
  } else {
    assert(std::filesystem::is_empty(path));
  }
}

} // namespace nn