#ifndef NN_PATTERN_HPP
#define NN_PATTERN_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace nn {

class Pattern
{
 private:
  std::vector<int> pattern_;
  std::string name_;
  static const std::string directory_;

 public:
  Pattern(std::string const& path);

  Pattern();

  std::size_t size() const;

  const std::string& name() const;

  void add(int value);

  void save_to_file() const;

  void save_image(unsigned int width, unsigned int height, std::string const& directory) const;
};

} // namespace nn

#endif