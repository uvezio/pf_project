#ifndef NN_PATTERN_HPP
#define NN_PATTERN_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace nn {

class Pattern
{
 private:
  std::vector<int> pattern_;

  static std::filesystem::path directory_;
  static bool initialized_;

 public:
  Pattern();

  static void set_directory(std::filesystem::path const& directory);

  const std::vector<int>& pattern() const;

  std::size_t size() const;

  void add(int value);

  void save_to_file(std::filesystem::path const& name, std::size_t size) const;

  void load_from_file(std::filesystem::path const& name, std::size_t size);

  void create_image(std::filesystem::path const& directory,
                    std::filesystem::path const& pattern_name,
                    unsigned int width, unsigned int height) const;
};

} // namespace nn

#endif