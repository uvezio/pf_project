// All relative paths are relative to the "build/" directory
// These two paths are the only ones relative to "recall.cpp"
// This path is the only one relative to "recall.cpp"
#include "../include/recall.hpp"

#include <cassert>
#include <random>
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
    : weight_matrix_directory_{"../" + base_directory.string()
                               + "weight_matrix/"}
    , patterns_directory_{"../" + base_directory.string() + "patterns/"}
    , corrupted_directory_{"../" + base_directory.string() + "corrupted_files/"}
{
  validate_weight_matrix_directory_();
  validate_patterns_directory_();
  configure_corrupted_directory_();

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

void Recall::corrupt_pattern(std::filesystem::path const& name) const
{
  std::filesystem::path path{patterns_directory_};
  path.replace_filename(name);

  assert(std::filesystem::is_regular_file(path));
  assert(path.extension() == ".txt");

  Pattern pattern;
  pattern.load_from_file(patterns_directory_, name, 4096);
  assert(pattern.size() == 4096);

  auto noisy = pattern;
  noisy.add_noise(0.08, 4096);
  auto noisy_name = name.filename().replace_extension(".noise.txt");
  noisy.save_to_file(corrupted_directory_, noisy_name, 4096);
  noisy.create_image(corrupted_directory_, noisy_name, 64, 64);

  auto incomplete = pattern;
  incomplete.cut(-1, 34, 58, 11, 35, 64, 64);
  auto cut_name = name.filename().replace_extension(".cut.txt");
  incomplete.save_to_file(corrupted_directory_, cut_name, 4096);
  incomplete.create_image(corrupted_directory_, cut_name, 64, 64);
}

} // namespace nn