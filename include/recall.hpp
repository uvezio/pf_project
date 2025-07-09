// All relative paths are relative to the build/ directory

#ifndef NN_RECALL_HPP
#define NN_RECALL_HPP

// These two paths are the only ones relative to "recall.hpp"
#include "pattern.hpp"
#include "weight_matrix.hpp"

#include <filesystem>
#include <vector>

namespace nn {

int sign(double value);

double hopfield_local_field(std::size_t index,
                            std::vector<int> const& current_state,
                            Weight_Matrix const& weight_matrix);

std::vector<double> hopfield_local_fields(std::vector<int> const& current_state,
                                          Weight_Matrix const& weight_matrix);

double hopfield_energy(std::vector<int> const& current_state,
                       Weight_Matrix const& weight_matrix);

class Recall
{
 private:
  Weight_Matrix weight_matrix_;
  Pattern original_pattern_;
  Pattern noisy_pattern_;
  Pattern cut_pattern_;
  std::vector<int> current_state_;
  std::size_t current_iteration_;

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

  const std::vector<int>& current_state() const;

  std::size_t current_iteration() const;

  void clear_state();

  // Acquires and corrupt a pattern from "../base_directory/patterns/" and saves
  // the corrupted pattern and image in "../base_directory/corrupted_files/";
  // sets the class state in order to call network_update_dynamics().
  void corrupt_pattern(std::filesystem::path const& name);

  // Applies Hopefield rule to update the current state
  bool single_network_update();

  // Updates the current state until it converges to a stable state
  void network_update_dynamics();

  // Saves the current state (pattern and image) in
  // "../base_directory/corrupted_files/"
  void save_current_state(std::filesystem::path const& original_name) const;
};

} // namespace nn

#endif