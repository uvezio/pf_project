#ifndef NN_ACQUISITION_HPP
#define NN_ACQUISITION_HPP

#include "pattern.hpp"

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace nn {

sf::Image load_image(std::string const& path, unsigned int min_width,
                     unsigned int min_height);

sf::Uint8 linear_interpolation(sf::Uint8 a, sf::Uint8 b, double t);

sf::Color color_interpolation(sf::Color const& c1, sf::Color const& c2,
                              double t);

sf::Image resize_image(sf::Image const& image, unsigned int width,
                       unsigned int height);

Pattern binarize_image(sf::Image const& image, unsigned int width,
                       unsigned int height, sf::Uint8 threshold);

class Acquisition
{
 private:
  std::vector<Pattern> patterns_;
  const std::filesystem::path source_directory_;
  const std::filesystem::path binarized_directory_;
  const std::filesystem::path patterns_directory_;

  void validate_source_directory_() const;
  void validate_binarized_directory_() const;
  void validate_patterns_directory_() const;

 public:
  Acquisition(std::filesystem::path const& dir);

  Acquisition();

  const std::vector<Pattern>& patterns() const;

  void acquire_and_save_patterns();

  void save_binarized_images() const;
};

} // namespace nn

#endif