#ifndef NN_PATTERN_HPP
#define NN_PATTERN_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace nn {

class Pattern
{
 private:
  std::vector<int> pattern_{};
  std::string name_;

 public:
  Pattern(std::string const& path);

  Pattern();

  std::size_t size() const;

  //std::string name() const;

  void add(int value);

  void save_to_file() const;

  void save_image(unsigned int width, unsigned int height) const;
};

} // namespace nn

#endif