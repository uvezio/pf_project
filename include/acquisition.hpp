// All relative paths are relative to the build/ directory

#ifndef NN_ACQUISITION_HPP
#define NN_ACQUISITION_HPP

// This path is the only one relative to "acquisition.hpp"
#include "pattern.hpp"

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <vector>

namespace nn {

// Free functions used in the Acquisition class

sf::Image load_image(std::filesystem::path const& path, unsigned int min_width,
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
  std::vector<Pattern> patterns_; // Not necessary but useful in testing
  const std::filesystem::path source_directory_;
  const std::filesystem::path binarized_directory_;
  const std::filesystem::path patterns_directory_;
  const std::vector<std::filesystem::path> extensions_allowed_;

  void validate_source_directory_() const;
  void configure_output_directories_() const;

 public:
  /*
   * Given the current structure of the project root, base_directory can only be
   * "" or "tests/" to differentiate ordinary code execution from test
   * execution. Alternatively the program throws an error since the
   * source_directory_ does not exist.
   */
  Acquisition(std::filesystem::path const& base_directory);

  Acquisition();

  const std::vector<Pattern>& patterns() const;

  // Acquires images from "../base_directory/images/source_images/" and saves
  // patterns in a one-line .txt file in "../base_directory/patterns/"
  void acquire_and_save_patterns();

  // Saves binarized images in "../base_directory/images/binarized_images/"
  // binarized images are constructed by loading patterns stored in
  // "../base_directory/patterns/"
  void save_binarized_images() const;
};

} // namespace nn

#endif