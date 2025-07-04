// All relative paths are relative to the build/ directory

#ifndef NN_PATTERN_HPP
#define NN_PATTERN_HPP

#include <filesystem>
#include <vector>

namespace nn {

class Pattern
{
 private:
  std::vector<int> pattern_;

 public:
  Pattern();

  const std::vector<int>& pattern() const;

  std::size_t size() const;

  void add(int value);

  void save_to_file(std::filesystem::path const& patterns_directory,
                    std::filesystem::path const& name, std::size_t size) const;

  void load_from_file(std::filesystem::path const& patterns_directory,
                      std::filesystem::path const& name, std::size_t size);

  void create_image(std::filesystem::path const& binarized_directory,
                    std::filesystem::path const& pattern_name,
                    unsigned int width, unsigned int height) const;

  void add_noise(double probability, std::size_t size);

  void cut(unsigned int from_row, unsigned int number_of_rows,
           unsigned int from_column, unsigned int number_of_columns,
           unsigned int width, unsigned int height);
};

} // namespace nn

#endif