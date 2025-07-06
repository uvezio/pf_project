// All relative paths are relative to the build/ directory

#ifndef NN_WEIGHT_MATRIX_HPP
#define NN_WEIGHT_MATRIX_HPP

#include <filesystem>
#include <vector>

namespace nn {

// Free functions useful to manage symmetric N * N matrices with null diagonal
// stored in vectors sized N * (N - 1) / 2

std::size_t matrix_to_vector_index(std::size_t i, std::size_t j, std::size_t N);

void increment_ij(std::size_t& i, std::size_t& j, std::size_t N);

double compute_weight_ij(std::size_t i, std::size_t j, std::size_t N,
                         std::vector<std::vector<int>> const& patterns);

class Weight_Matrix
{
 private:
  const std::size_t neurons_;

  // Since weight matrix is symmetric neurons_ * neurons_ with null diagonal
  // weights_.size() == neurons_ * (neurons_ - 1) / 2 after the call to fill()
  std::vector<double> weights_;

 public:
  // Not necessary but useful in testing
  Weight_Matrix(std::size_t neurons);

  Weight_Matrix();

  const std::vector<double>& weights() const;

  std::size_t neurons() const;

  double at(std::size_t i, std::size_t j) const;

  void fill(std::vector<std::vector<int>> const& patterns, std::size_t neurons);

  void save_to_file(std::filesystem::path const& matrix_directory,
                    std::filesystem::path const& name,
                    std::size_t neurons) const;

  void load_from_file(std::filesystem::path const& matrix_directory,
                      std::filesystem::path const& name, std::size_t neurons);
};

} // namespace nn

#endif