#ifndef NN_RECALL_HPP
#define NN_RECALL_HPP

#include "pattern.hpp"
#include "weight_matrix.hpp"

#include <filesystem>

namespace nn {

class Recall
{
 private:
  const std::filesystem::path weight_matrix_directory_;
  const std::filesystem::path patterns_directory_;
  const std::filesystem::path corrupted_directory_;

  void validate_weight_matrix_directory_() const;
  void validate_patterns_directory_() const;

 public:
  /*
   * Given the current structure of the project root, base_directory can only be
   * "" or "tests/" to differentiate ordinary code execution from test
   * execution. Alternatively the program throws an error since the
   * patterns_directory_ and the weight_matrix_directory_ do not exist.
   */
  Recall(std::filesystem::path const& base_directory);

  Recall();
};

} // namespace nn

#endif