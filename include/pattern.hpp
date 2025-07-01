#ifndef NN_PATTERN_HPP
#define NN_PATTERN_HPP

#include <filesystem>
#include <vector>

namespace nn {

class Pattern
{
 private:
  std::vector<int> pattern_;

  // Not necessary but it is an additional guarantee that the patterns are all
  // in the same directory
  static std::filesystem::path patterns_directory_;
  // Not necessary but useful so that patterns_directory_ is not modified after
  // the first call to set_directory()
  static bool initialized_;

 public:
  Pattern();

  static void set_directory(std::filesystem::path const& patterns_directory);

  const std::vector<int>& pattern() const;

  std::size_t size() const;

  void add(int value);

  void save_to_file(std::filesystem::path const& name, std::size_t size) const;

  void load_from_file(std::filesystem::path const& name, std::size_t size);

  void create_image(std::filesystem::path const& binarized_directory,
                    std::filesystem::path const& pattern_name,
                    unsigned int width, unsigned int height) const;
};

} // namespace nn

#endif