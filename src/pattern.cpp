// All relative paths are relative to the "build/" directory

// This path is the only one relative to "pattern.cpp"
#include "../include/pattern.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>

namespace nn {

sf::Color compute_color(int value)
{
  assert(value == +1 || value == -1);
  return value == +1 ? sf::Color::White : sf::Color::Black;
}

sf::Image create_image(unsigned int width, unsigned int height,
                       std::vector<int> const& pattern)
{
  assert(pattern.size() == width * height);

  sf::Image image;
  image.create(width, height);

  for (unsigned int y{0}; y < height; ++y) {
    for (unsigned int x{0}; x < width; ++x) {
      auto color = compute_color(pattern[y * width + x]);
      image.setPixel(x, y, color);
    }
  }

  assert(image.getSize().x == width);
  assert(image.getSize().y == height);

  return image;
}

Pattern::Pattern(std::vector<int> pattern)
    : pattern_{pattern}
{
  assert(pattern_.size() == pattern.size());
}

Pattern::Pattern()
    : Pattern::Pattern(std::vector<int>{})
{}

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

void Pattern::save_to_file(std::filesystem::path const& patterns_directory,
                           std::filesystem::path const& name,
                           std::size_t size) const
{
  assert(std::filesystem::is_directory(patterns_directory));

  auto path = patterns_directory;
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

  // If outfile is not closed here, file at path could be written after the
  // is_empty() check
  outfile.close();

  if (size != 0) {
    assert(!std::filesystem::is_empty(path));
  } else {
    assert(std::filesystem::is_empty(path));
  }
}

void Pattern::load_from_file(std::filesystem::path const& patterns_directory,
                             std::filesystem::path const& name,
                             std::size_t size)
{
  pattern_.clear();

  assert(std::filesystem::is_directory(patterns_directory));

  auto path = patterns_directory;
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

void Pattern::save_image(std::filesystem::path const& binarized_directory,
                         std::filesystem::path const& pattern_name,
                         unsigned int width, unsigned int height) const
{
  assert(std::filesystem::is_directory(binarized_directory));

  auto path = binarized_directory;
  path.replace_filename(pattern_name);
  assert(path.extension() == ".txt");
  path.replace_extension(".png");

  assert(pattern_.size() == width * height);

  auto image = create_image(width, height, pattern_);

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

void Pattern::add_noise(double probability, std::size_t size)
{
  assert(pattern_.size() == size);
  assert(std::all_of(pattern_.begin(), pattern_.end(),
                     [](int value) { return value == +1 || value == -1; }));

  assert(probability >= 0 && probability <= 1);

  std::random_device r;
  std::default_random_engine eng{r()};
  std::bernoulli_distribution dist{probability};

  std::transform(pattern_.begin(), pattern_.end(), pattern_.begin(),
                 [&](int value) { return (dist(eng)) ? -1 * value : value; });

  assert(pattern_.size() == size);
  assert(std::all_of(pattern_.begin(), pattern_.end(),
                     [](int value) { return value == +1 || value == -1; }));
  (void)size; // Prevent unused parameter warning
}

void Pattern::cut(int new_value, unsigned int from_row, unsigned int to_row,
                  unsigned int from_column, unsigned int to_column,
                  unsigned int width, unsigned int height)
{
  assert(pattern_.size() == width * height);
  assert(std::all_of(pattern_.begin(), pattern_.end(),
                     [](int value) { return value == +1 || value == -1; }));

  assert(new_value == +1 || new_value == -1);
  assert(from_row <= to_row && to_row <= height);
  assert(from_column <= to_column && to_column <= width);

  for (unsigned int y{from_row - 1}; y != to_row; ++y) {
    for (unsigned int x{from_column - 1}; x != to_column; ++x) {
      pattern_[y * width + x] = new_value;
    }
  }

  assert(pattern_.size() == width * height);
  assert(std::all_of(pattern_.begin(), pattern_.end(),
                     [](int value) { return value == +1 || value == -1; }));
  (void)height; // Prevent unused parameter warning
}

} // namespace nn