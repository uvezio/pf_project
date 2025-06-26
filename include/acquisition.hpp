#ifndef NN_ACQUISITION_HPP
#define NN_ACQUISITION_HPP

#include "pattern.hpp"

#include <SFML/Graphics.hpp>
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

Pattern binarize_image(sf::Image const& image, std::string const& name,
                       unsigned int width, unsigned int height,
                       sf::Uint8 threshold);

struct Image
{
  std::string name;
  sf::Image image;
  sf::Image resized;
  Pattern pattern;
};

class Acquisition
{
 private:
  std::vector<Image> images_;
  const std::string source_directory_;
  const std::string binarized_directory_;

  std::vector<Image> initialize_images(std::vector<std::string> const& names);

 public:
  Acquisition(std::vector<std::string> const& names);

  const std::vector<Image>& images() const;

  void load_images();

  void resize_images();

  void binarize_images();

  void save_binarized_images() const;
};

} // namespace nn

#endif