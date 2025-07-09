// All relative paths are relative to the build/ directory

#ifndef NN_PATTERN_HPP
#define NN_PATTERN_HPP

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <vector>

namespace nn {

sf::Color compute_color(int value);

sf::Image create_image(unsigned int width, unsigned int height,
                       std::vector<int> const& pattern);

class Pattern
{
 private:
  std::vector<int> pattern_;

 public:
  Pattern(std::vector<int> pattern);
  
  Pattern();

  const std::vector<int>& pattern() const;

  std::size_t size() const;

  void add(int value);

  void save_to_file(std::filesystem::path const& patterns_directory,
                    std::filesystem::path const& name, std::size_t size) const;

  void load_from_file(std::filesystem::path const& patterns_directory,
                      std::filesystem::path const& name, std::size_t size);

  void save_image(std::filesystem::path const& binarized_directory,
                  std::filesystem::path const& pattern_name, unsigned int width,
                  unsigned int height) const;

  void add_noise(double probability, std::size_t size);

  // new_value must be +1 (white fill) or -1 (black fill)
  void cut(int new_value, unsigned int from_row, unsigned int to_row,
           unsigned int from_column, unsigned int to_column, unsigned int width,
           unsigned int height);
};

} // namespace nn

#endif