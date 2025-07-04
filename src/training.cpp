// All relative paths are relative to the "build/" directory

// These two paths are the only ones relative to "training.cpp"
#include "../include/training.hpp"
#include "../include/pattern.hpp"

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace nn {

void Training::validate_patterns_directory_() const
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

void Training::configure_output_directory_() const
{
  if (!std::filesystem::exists(weight_matrix_directory_)) {
    std::filesystem::create_directory(weight_matrix_directory_);
  }
  if (!std::filesystem::is_directory(weight_matrix_directory_)) {
    throw std::runtime_error("Path \"" + weight_matrix_directory_.string()
                             + "\" is not a directory.");
  }
  if (!std::filesystem::is_empty(weight_matrix_directory_)) {
    for (auto const& file :
         std::filesystem::directory_iterator(weight_matrix_directory_)) {
      std::filesystem::remove_all(file.path());
    }
  }
}

// base_directory can only be "" or "tests/"
Training::Training(std::filesystem::path const& base_directory)
    : weight_matrix_{}
    , patterns_directory_{"../" + base_directory.string() + "patterns/"}
    , weight_matrix_directory_{"../" + base_directory.string()
                               + "weight_matrix/"}
{
  assert(weight_matrix_.neurons() == 4096);

  validate_patterns_directory_();
  configure_output_directory_();

  assert(std::filesystem::is_directory(patterns_directory_)
         && !std::filesystem::is_empty(patterns_directory_));
  assert(std::filesystem::is_directory(weight_matrix_directory_)
         && std::filesystem::is_empty(weight_matrix_directory_));
}

Training::Training()
    : Training::Training("")
{}

const Weight_Matrix& Training::weight_matrix() const
{
  return weight_matrix_;
}

void Training::acquire_and_save_weight_matrix()
{
  std::vector<std::vector<int>> patterns;

  for (auto const& file :
       std::filesystem::directory_iterator(patterns_directory_)) {
    assert(file.is_regular_file());
    assert(file.path().extension() == ".txt");

    Pattern pattern;
    pattern.load_from_file(patterns_directory_, file.path().filename(), 4096);
    assert(pattern.size() == 4096);
    patterns.push_back(pattern.pattern());
  }

  assert(weight_matrix_.neurons() == 4096);
  weight_matrix_.fill(patterns, 4096);
  assert(weight_matrix_.weights().size() == 4095 * 4096 / 2);

  weight_matrix_.save_to_file(weight_matrix_directory_, "weight_matrix.txt",
                              4096);
}

} // namespace nn