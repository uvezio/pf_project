// All relative paths are relative to the build/ directory

#ifndef NN_TRAINING_HPP
#define NN_TRAINING_HPP

// This path is the only one relative to "training.hpp"
#include "weight_matrix.hpp"

#include <filesystem>

namespace nn {

class Training
{
 private:
  Weight_Matrix weight_matrix_; // Non necessary but useful in testing
  const std::filesystem::path patterns_directory_;
  const std::filesystem::path weight_matrix_directory_;

  void validate_patterns_directory_() const;
  void configure_output_directory_() const;

 public:
  /*
   * Given the current structure of the project root, base_directory can only be
   * "" or "tests/" to differentiate ordinary code execution from test
   * execution. Alternatively the program throws an error since the
   * patterns_directory_ does not exists.
   */
  Training(std::filesystem::path const& base_directory);

  Training();

  const Weight_Matrix& weight_matrix() const;

  // Acquires patterns from "../base_directory/patterns/" and saves
  // the wheight_matrix in a one-line .txt file in
  // "../base_directory/weight_matrix/"
  void acquire_and_save_weight_matrix();
};

} // namespace nn

#endif