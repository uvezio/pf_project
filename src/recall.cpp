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

void Recall::configure_output_directories_() const
{
  for (auto const& output_dir :
       {noisy_patterns_directory_, incomplete_patterns_directory_,
        noisy_images_directory_, incomplete_images_directory_}) {
    if (!std::filesystem::exists(output_dir)) {
      std::filesystem::create_directory(output_dir);
    }
    if (!std::filesystem::is_directory(output_dir)) {
      throw std::runtime_error("Path \"" + output_dir.string()
                               + "\" is not a directory.");
    }
    if (!std::filesystem::is_empty(output_dir)) {
      for (auto const& file : std::filesystem::directory_iterator(output_dir)) {
        std::filesystem::remove_all(file.path());
      }
    }
  }
}

// base_directory can only be "" or "tests/"
Recall::Recall(std::filesystem::path const& base_directory)
    : weight_matrix_directory_{"../" + base_directory.string()
                               + "weight_matrix/"}
    , patterns_directory_{"../" + base_directory.string() + "patterns/"}
    , noisy_patterns_directory_{"../" + base_directory.string()
                                + "corrupted_patterns/noisy_patterns/"}
    , incomplete_patterns_directory_{"../" + base_directory.string()
                                     + "corrupted_patterns/"
                                       "incomplete_patterns/"}
    , noisy_images_directory_{"../" + base_directory.string()
                              + "images/corrupted_images/noisy_images/"}
    , incomplete_images_directory_{
          "../" + base_directory.string()
          + "images/corrupted_images/incomplete_images/"}
{
  validate_weight_matrix_directory_();
  validate_patterns_directory_();
  configure_output_directories_();

  assert(std::filesystem::exists(weight_matrix_directory_.string()
                                 + "weight_matrix.txt"));
  assert(std::filesystem::is_directory(patterns_directory_)
         && !std::filesystem::is_empty(patterns_directory_));
  assert(std::filesystem::is_directory(noisy_patterns_directory_)
         && std::filesystem::is_empty(noisy_patterns_directory_));
  assert(std::filesystem::is_directory(incomplete_patterns_directory_)
         && std::filesystem::is_empty(incomplete_patterns_directory_));
  assert(std::filesystem::is_directory(noisy_images_directory_)
         && std::filesystem::is_empty(noisy_images_directory_));
  assert(std::filesystem::is_directory(incomplete_images_directory_)
         && std::filesystem::is_empty(incomplete_images_directory_));
}

Recall::Recall()
    : Recall::Recall("")
{}

void Recall::corrupt_patterns() const
{
  std::random_device r;
  std::default_random_engine eng{r()};
  std::uniform_real_distribution<double> uniform{0., 0.25};
  std::uniform_int_distribution<unsigned int> from_uniform_int{1, 48};

  for (auto const& file :
       std::filesystem::directory_iterator(patterns_directory_)) {
    assert(file.is_regular_file());
    assert(file.path().extension() == ".txt");

    Pattern pattern;
    pattern.load_from_file(patterns_directory_, file.path().filename(), 4096);
    assert(pattern.size() == 4096);

    auto noisy = pattern;
    noisy.add_noise(uniform(eng), 4096);
    noisy.save_to_file(noisy_patterns_directory_, file.path().filename(), 4096);
    noisy.create_image(noisy_images_directory_, file.path().filename(), 64, 64);

    auto from_x = from_uniform_int(eng);
    auto from_y = from_uniform_int(eng);
    std::uniform_int_distribution<unsigned int> to_x_uniform_int{from_x + 16,
                                                                 64};
    auto to_x = to_x_uniform_int(eng);
    std::uniform_int_distribution<unsigned int> to_y_uniform_int{from_y + 16,
                                                                 64};
    auto to_y = to_y_uniform_int(eng);

    auto incomplete = pattern;
    incomplete.cut(from_y, to_y, from_x, to_x, 64, 64);
    incomplete.save_to_file(incomplete_patterns_directory_,
                            file.path().filename(), 4096);
    incomplete.create_image(incomplete_images_directory_,
                            file.path().filename(), 64, 64);
  }
}

} // namespace nn