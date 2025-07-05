// All relative paths are relative to the "build/" directory
// These two paths are the only ones relative to "recall.cpp"
// This path is the only one relative to "recall.cpp"
#include "../include/recall.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace nn {

int hopfield_rule(std::size_t index, std::vector<int> const& pattern_t0,
                  std::vector<double> const& weights)
{
  std::size_t j{0};
  auto new_value = std::accumulate(
      pattern_t0.begin(), pattern_t0.end(), 0,
      [&j, index, &weights](double sum, int value) {
        ++j;
        double j_addend;
        if (j != index) {
          j_addend = weights[matrix_to_vector_index(index, j, 4096)] * value;
        } else {
          j_addend = 0;
        };
        return sum + j_addend;
      });

  auto value_t1 = (new_value >= 0) ? +1 : -1;

  return value_t1;
}

void Recall::validate_weight_matrix_directory_() const
{
  if (!std::filesystem::exists(weight_matrix_directory_)) {
    throw std::runtime_error("Directory \"" + weight_matrix_directory_.string()
                             + "\" not found.");
  }
  if (!std::filesystem::is_directory(weight_matrix_directory_)) {
    throw std::runtime_error("Path \"" + weight_matrix_directory_.string()
                             + "\" is not a directory.");
  }
  if (std::filesystem::is_empty(weight_matrix_directory_)) {
    throw std::runtime_error("Directory \"" + weight_matrix_directory_.string()
                             + "\" is empty.");
  }

  for (auto const& file :
       std::filesystem::directory_iterator(weight_matrix_directory_)) {
    if (!file.is_regular_file()) {
      throw std::runtime_error("File \"" + weight_matrix_directory_.string()
                               + file.path().filename().string()
                               + "\" is not a regular file.");
    }
    if (file.path().filename() != "weight_matrix.txt") {
      throw std::runtime_error(
          "In directory \"" + weight_matrix_directory_.string()
          + "\" there must be only the file \"weight_matrix.txt\".\nFile \""
          + file.path().filename().string() + "\" was found.");
    }
  }
}

void Recall::validate_patterns_directory_() const
{
  if (!std::filesystem::exists(patterns_directory_)) {
    throw std::runtime_error("Directory \"" + patterns_directory_.string()
                             + "\" not found.");
  }
  if (!std::filesystem::is_directory(patterns_directory_)) {
    throw std::runtime_error("Path \"" + patterns_directory_.string()
                             + "\" is not a directory.");
  }
  if (std::filesystem::is_empty(patterns_directory_)) {
    throw std::runtime_error("Directory \"" + patterns_directory_.string()
                             + "\" is empty.");
  }

  for (auto const& file :
       std::filesystem::directory_iterator(patterns_directory_)) {
    if (!file.is_regular_file()) {
      throw std::runtime_error("File \"" + patterns_directory_.string()
                               + file.path().filename().string()
                               + "\" is not a regular file.");
    }
    if (file.path().extension() != ".txt") {
      throw std::runtime_error("File \"" + patterns_directory_.string()
                               + file.path().filename().string()
                               + "\" has an invalid extension.");
    }
  }
}

void Recall::configure_corrupted_directory_() const
{
  if (!std::filesystem::exists(corrupted_directory_)) {
    std::filesystem::create_directory(corrupted_directory_);
  }
  if (!std::filesystem::is_directory(corrupted_directory_)) {
    throw std::runtime_error("Path \"" + corrupted_directory_.string()
                             + "\" is not a directory.");
  }
  if (!std::filesystem::is_empty(corrupted_directory_)) {
    for (auto const& file :
         std::filesystem::directory_iterator(corrupted_directory_)) {
      std::filesystem::remove_all(file.path());
    }
  }
}

// base_directory can only be "" or "tests/"
Recall::Recall(std::filesystem::path const& base_directory)
    : weight_matrix_{}
    , original_pattern_{}
    , noisy_pattern_{}
    , cut_pattern_{}
    , weight_matrix_directory_{"../" + base_directory.string()
                               + "weight_matrix/"}
    , patterns_directory_{"../" + base_directory.string() + "patterns/"}
    , corrupted_directory_{"../" + base_directory.string() + "corrupted_files/"}
{
  validate_weight_matrix_directory_();
  validate_patterns_directory_();
  configure_corrupted_directory_();

  weight_matrix_.load_from_file(weight_matrix_directory_, "weight_matrix.txt",
                                4096);
  assert(weight_matrix_.neurons() == 4096);
  assert(weight_matrix_.weights().size() == 8'386'560);

  assert(original_pattern_.size() == 0);
  assert(noisy_pattern_.size() == 0);
  assert(cut_pattern_.size() == 0);

  assert(std::filesystem::exists(weight_matrix_directory_.string()
                                 + "weight_matrix.txt"));
  assert(std::filesystem::is_directory(patterns_directory_)
         && !std::filesystem::is_empty(patterns_directory_));
  assert(std::filesystem::is_directory(corrupted_directory_)
         && std::filesystem::is_empty(corrupted_directory_));
}

Recall::Recall()
    : Recall::Recall("")
{}

const Weight_Matrix& Recall::weight_matrix() const
{
  return weight_matrix_;
}

const Pattern& Recall::original_pattern() const
{
  return original_pattern_;
}

const Pattern& Recall::noisy_pattern() const
{
  return noisy_pattern_;
}

const Pattern& Recall::cut_pattern() const
{
  return cut_pattern_;
}

void Recall::corrupt_pattern(std::filesystem::path const& name)
{
  std::filesystem::path path{patterns_directory_};
  path.replace_filename(name);

  assert(std::filesystem::is_regular_file(path));
  assert(path.extension() == ".txt");

  original_pattern_.load_from_file(patterns_directory_, name, 4096);
  assert(original_pattern_.size() == 4096);
  assert(std::all_of(original_pattern_.pattern().begin(),
                     original_pattern_.pattern().end(),
                     [](int value) { return value == +1 || value == -1; }));

  noisy_pattern_ = original_pattern_;
  noisy_pattern_.add_noise(0.08, 4096);
  assert(noisy_pattern_.size() == 4096);
  assert(std::all_of(noisy_pattern_.pattern().begin(),
                     noisy_pattern_.pattern().end(),
                     [](int value) { return value == +1 || value == -1; }));

  auto noisy_name = name.filename().replace_extension(".noise.txt");
  noisy_pattern_.save_to_file(corrupted_directory_, noisy_name, 4096);
  noisy_pattern_.create_image(corrupted_directory_, noisy_name, 64, 64);

  cut_pattern_ = original_pattern_;
  cut_pattern_.cut(-1, 34, 58, 11, 35, 64, 64);
  assert(noisy_pattern_.size() == 4096);
  assert(std::all_of(cut_pattern_.pattern().begin(),
                     cut_pattern_.pattern().end(),
                     [](int value) { return value == +1 || value == -1; }));

  auto cut_name = name.filename().replace_extension(".cut.txt");
  cut_pattern_.save_to_file(corrupted_directory_, cut_name, 4096);
  cut_pattern_.create_image(corrupted_directory_, cut_name, 64, 64);
}

void Recall::network_update_dynamics()
{
  auto pattern       = noisy_pattern_.pattern();
  auto const weights = weight_matrix_.weights();

  std::vector<int> pattern_t0;
  bool has_converged{false};
  std::size_t index{0};
  while (!has_converged) {
    ++index;
    has_converged   = true;
    pattern_t0 = pattern;
    std::transform(pattern_t0.begin(), pattern_t0.end(), pattern.begin(),
                   [&index, &weights, &has_converged, &pattern_t0](int value) {
                     auto new_value = hopfield_rule(index, pattern_t0, weights);
                     if (has_converged == true && new_value != value) {
                       has_converged = false;
                     }
                     return new_value;
                   });
  }
}

} // namespace nn