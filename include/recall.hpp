// All relative paths are relative to the build/ directory

#ifndef NN_RECALL_HPP
#define NN_RECALL_HPP

// This path is the only one relative to "recall.hpp"
#include "pattern.hpp"
#include "weight_matrix.hpp"

#include <filesystem>

namespace nn {

int hopfield_rule(std::size_t index, std::vector<int> const& pattern,
                  std::vector<double> const& weights);

class Recall
{
 private:
  Weight_Matrix weight_matrix_;
  Pattern original_pattern_;
  Pattern noisy_pattern_;
  Pattern cut_pattern_;
  const std::filesystem::path weight_matrix_directory_;
  const std::filesystem::path patterns_directory_;
  const std::filesystem::path corrupted_directory_;

  void validate_weight_matrix_directory_() const;
  void validate_patterns_directory_() const;
  void configure_corrupted_directory_() const;

 public:
  /*
   * Given the current structure of the project root, base_directory can only be
   * "" or "tests/" to differentiate ordinary code execution from test
   * execution. Alternatively the program throws an error since the
   * patterns_directory_ and the weight_matrix_directory_ do not exist.
   */
  Recall(std::filesystem::path const& base_directory);

  Recall();

  const Weight_Matrix& weight_matrix() const;

  const Pattern& original_pattern() const;

  const Pattern& noisy_pattern() const;

  const Pattern& cut_pattern() const;

  // Acquires and corrupt a pattern from "../base_directory/patterns/" and saves
  // the corrupted pattern and image in "../base_directory/corrupted_files/"
  void corrupt_pattern(std::filesystem::path const& name);

  void network_update_dynamics();
};

} // namespace nn

#endif