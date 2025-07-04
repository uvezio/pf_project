#include "../include/recall.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

namespace nn {

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

// base_directory can only be "" or "tests/"
Recall::Recall(std::filesystem::path const& base_directory)
    : weight_matrix_directory_{"../" + base_directory.string()
                               + "weight_matrix/"}
    , patterns_directory_{"../" + base_directory.string() + "patterns/"}
{
  validate_weight_matrix_directory_();
  validate_patterns_directory_();
  nn::Pattern::set_directory(patterns_directory_);

  assert(std::filesystem::exists(weight_matrix_directory_.string()
                                 + "weight_matrix.txt"));
  assert(std::filesystem::is_directory(patterns_directory_)
         && !std::filesystem::is_empty(patterns_directory_));
}

Recall::Recall()
    : Recall::Recall("")
{}

} // namespace nn