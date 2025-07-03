// All relative paths are relative to the build/ directory

#ifndef NN_WEIGHT_MATRIX_HPP
#define NN_WEIGHT_MATRIX_HPP

#include <filesystem>
#include <vector>

namespace nn {

class Weight_Matrix
{
 private:
  const std::size_t neurons_;
  std::vector<double> weights_;

 public:
  // Not necessary but useful in testing
  Weight_Matrix(std::size_t neurons);

  Weight_Matrix();

  const std::vector<double>& weights() const;

  std::size_t neurons() const;

  // Not necessary but useful in testing
  std::size_t matrix_to_vector_index(std::size_t i, std::size_t j) const;

  void increment_ij(std::size_t& i, std::size_t& j) const;

  double compute_weight_ij(std::size_t i, std::size_t j,
                           std::vector<std::vector<int>> const& patterns) const;

  void fill(std::vector<std::vector<int>> const& patterns, std::size_t neurons);

  void save_to_file(std::filesystem::path const& matrix_directory,
                    std::filesystem::path const& name,
                    std::size_t neurons) const;

  void load_from_file(std::filesystem::path const& matrix_directory,
                      std::filesystem::path const& name, std::size_t neurons);
};

} // namespace nn

#endif