// All relative paths are relative to the "build/" directory
// These two paths are the only ones relative to "recall.cpp"
// This path is the only one relative to "recall.cpp"
#include "../include/recall.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cassert>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace nn {

int hopfield_rule(std::size_t index, std::vector<int> const& pattern,
                  Weight_Matrix const& weight_matrix)
{
  assert(index >= 1 && index <= weight_matrix.neurons());
  assert(weight_matrix.weights().size()
         == weight_matrix.neurons() * (weight_matrix.neurons() - 1) / 2);

  std::size_t j{0};
  auto weighted_sum =
      std::accumulate(pattern.begin(), pattern.end(), 0.,
                      [index, &j, &weight_matrix](double sum, int value) {
                        ++j;
                        return sum + weight_matrix.at(index, j) * value;
                      });

  auto new_value = (weighted_sum >= 0) ? +1 : -1;

  return new_value;
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
  assert(weight_matrix_.neurons() == 4096);
  assert(weight_matrix_.weights().size() == 8'386'560);

  auto pattern = noisy_pattern_.pattern();
  assert(pattern.size() == 4096);

  sf::RenderWindow window(sf::VideoMode(64, 64), "Network update dynamics");
  window.setFramerateLimit(60);

  sf::Texture texture;
  texture.create(64, 64);
  sf::Sprite sprite(texture);
  sprite.setScale(8., 8.);

  std::vector<sf::Uint8> pixels;
  //std::generate_n(std::back_inserter(pixels), 3 * pattern.size(), []() { ; });

  bool has_converged{false};

  while (!has_converged || window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    for (std::size_t i{0}; i != pattern.size(); ++i) {
    }

    texture.update(pixels.data());

    window.clear(sf::Color::Black);
    window.draw(sprite);
    window.display();
  }

  std::vector<int> pattern_t0;
  std::size_t index{0};
  while (!has_converged) {
    ++index;
    has_converged = true;
    pattern_t0    = pattern;
    std::transform(pattern_t0.begin(), pattern_t0.end(), pattern.begin(),
                   [&index, this, &has_converged, &pattern_t0](int value) {
                     auto new_value =
                         hopfield_rule(index, pattern_t0, weight_matrix_);
                     if (has_converged == true && new_value != value) {
                       has_converged = false;
                     }
                     return new_value;
                   });

    /*sf::Image image;
    image.create(64, 64);

    for (unsigned int y{0}; y < 64; ++y) {
      for (unsigned int x{0}; x < 64; ++x) {
        auto index = y * 64 + x;
        auto value = pattern[index];
        assert(value == +1 || value == -1);
        auto color = value == +1 ? sf::Color::White : sf::Color::Black;
        image.setPixel(x, y, color);
      }
    }*/
  }
}

} // namespace nn