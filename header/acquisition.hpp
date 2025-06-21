#ifndef NN_ACQUISITION_HPP
#define NN_ACQUISITION_HPP

#include "pattern.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace nn {

sf::Image load_image(std::string const& path);

sf::Uint8 linear_interpolation(sf::Uint8 a, sf::Uint8 b, double t);

sf::Color color_interpolation(sf::Color const& c1, sf::Color const& c2,
                              double t);

sf::Image resize_image(sf::Image const& image);

Pattern binarize_image(sf::Image const& image, std::string const& name);

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
  bool all_loaded_{false};
  bool all_resized_{false};
  bool all_binarized_{false};

  std::vector<Image> initialize_images(std::vector<std::string> const& names);

 public:
  Acquisition(std::vector<std::string> const& names);

  std::vector<Image> images() const;

  void load_images();

  void resize_images();

  void binarize_images();

  void save_binarized_images() const;
};

} // namespace nn

#endif