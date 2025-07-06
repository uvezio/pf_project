// All relative paths are relative to the "build/" directory

// This path is the only one relative to "weight_matrix.cpp"
#include "../include/weight_matrix.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <numeric>
#include <stdexcept>

namespace nn {

std::size_t matrix_to_vector_index(std::size_t i, std::size_t j, std::size_t N)
{
  assert(i >= 1 && i <= N);
  assert(j >= 1 && j <= N);

  if (i < j) {
    std::size_t vector_index{(i - 1) * (2 * N - i) / 2 + (j - i - 1)};
    assert(vector_index < N * (N - 1) / 2);
    return vector_index;
  } else if (i > j) {
    return matrix_to_vector_index(j, i, N);
  } else {
    throw std::runtime_error(
        "A vector index relative to the given matrix index does not exist.");
  }
}

void increment_ij(std::size_t& i, std::size_t& j, std::size_t N)
{
  assert(i >= 1 && i <= N - 1);
  assert(j >= i + 1 && j <= N);

  if (j + 1 <= N) {
    ++j;
  } else if (i + 1 <= N) {
    ++i;
    j = i + 1;
  } else {
    throw std::runtime_error("Index i or j out of bounds.");
  }

  assert(i >= 1 && i <= N);
  assert(j >= i + 1);
  if (i != N) {
    assert(j <= N);
  } else {
    assert(j == N + 1);
  }
}

double compute_weight_ij(std::size_t i, std::size_t j, std::size_t N,
                         std::vector<std::vector<int>> const& patterns)
{
  assert(i >= 1 && i <= N - 1);
  assert(j >= i + 1 && j <= N);

  auto sum_ij =
      std::accumulate(patterns.begin(), patterns.end(), 0,
                      [i, j](int sum, std::vector<int> const& pattern) {
                        return sum + pattern[i - 1] * pattern[j - 1];
                      });

  auto weight_ij = static_cast<double>(sum_ij) / static_cast<double>(N);

  assert(weight_ij
             >= -static_cast<double>(patterns.size()) / static_cast<double>(N)
         && weight_ij <= static_cast<double>(patterns.size())
                             / static_cast<double>(N));

  return weight_ij;
}

Weight_Matrix::Weight_Matrix(std::size_t neurons)
    : neurons_{neurons}
{
  assert(neurons_ == neurons);
  assert(weights_.size() == 0);
}

Weight_Matrix::Weight_Matrix()
    : Weight_Matrix::Weight_Matrix(4096)
{}

const std::vector<double>& Weight_Matrix::weights() const
{
  return weights_;
}

std::size_t Weight_Matrix::neurons() const
{
  return neurons_;
}

double Weight_Matrix::at(std::size_t i, std::size_t j) const
{
  assert(weights_.size() == neurons_ * (neurons_ - 1) / 2);

  assert(i >= 1 && i <= neurons_);
  assert(j >= 1 && j <= neurons_);

  if (i != j) {
    return weights_[matrix_to_vector_index(i, j, neurons_)];
  } else {
    return 0.;
  }
}

void Weight_Matrix::fill(std::vector<std::vector<int>> const& patterns,
                         std::size_t neurons)
{
  assert(std::all_of(
      patterns.begin(), patterns.end(),
      [this](std::vector<int> const& pattern) {
        return (pattern.size() == neurons_
                && std::all_of(pattern.begin(), pattern.end(), [](int value) {
                     return value == +1 || value == -1;
                   }));
      }));

  assert(neurons_ == neurons);

  weights_.clear();
  assert(weights_.size() == 0);

  std::size_t i{1};
  std::size_t j{2};
  std::generate_n(std::back_inserter(weights_), (neurons - 1) * neurons / 2,
                  [&i, &j, this, &patterns]() {
                    auto w = compute_weight_ij(i, j, neurons_, patterns);
                    increment_ij(i, j, neurons_);
                    return w;
                  });

  assert(i == neurons_ && j == neurons_ + 1);
  assert(weights_.size() == (neurons_ - 1) * neurons_ / 2);
}

void Weight_Matrix::save_to_file(std::filesystem::path const& matrix_directory,
                                 std::filesystem::path const& name,
                                 std::size_t neurons) const
{
  assert(std::filesystem::is_directory(matrix_directory));

  auto path = matrix_directory;
  path.replace_filename(name);
  assert(path.extension() == ".txt");

  std::ofstream outfile{path};

  if (!outfile) {
    throw std::runtime_error("File \"" + path.string()
                             + "\" not created successfully.");
  }

  assert(std::filesystem::is_regular_file(path));

  assert(neurons_ == neurons);
  assert(weights_.size() == (neurons_ - 1) * neurons_ / 2);

  for (auto weight : weights_) {
    if (!(outfile << weight << ' ')) {
      throw std::runtime_error("File \"" + path.string()
                               + "\" not written successfully.");
    }
  }

  // If outfile is not closed here, file at path could be written after the
  // is_empty() check
  outfile.close();

  if (neurons != 0 && neurons != 1) {
    assert(!std::filesystem::is_empty(path));
  } else {
    assert(std::filesystem::is_empty(path));
  }
}

void Weight_Matrix::load_from_file(
    std::filesystem::path const& matrix_directory,
    std::filesystem::path const& name, std::size_t neurons)
{
  assert(neurons_ == neurons);
  weights_.clear();

  assert(std::filesystem::is_directory(matrix_directory));

  auto path = matrix_directory;
  path.replace_filename(name);

  assert(std::filesystem::is_regular_file(path));
  assert(path.extension() == ".txt");
  if (neurons != 0 && neurons != 1) {
    assert(!std::filesystem::is_empty(path));
  } else {
    assert(std::filesystem::is_empty(path));
  }

  std::ifstream infile{path};

  if (!infile) {
    throw std::runtime_error("File \"" + path.string()
                             + "\" not opened successfully.");
  }

  double weight;
  while (infile >> weight) {
    weights_.push_back(weight);
  }

  if (weights_.size() != (neurons_ - 1) * neurons_ / 2) {
    throw std::runtime_error(
        "Error in file \"" + path.string() + "\".\nNumber of entries must be: "
        + std::to_string((neurons_ - 1) * neurons_ / 2)
        + "\nActual number of entries: " + std::to_string(weights_.size()));
  }

  assert(weights_.size() == (neurons_ - 1) * neurons_ / 2);
}

} // namespace nn