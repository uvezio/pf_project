// All relative paths are relative to the "build/" directory
// These two paths are the only ones relative to "recall.cpp"
// This path is the only one relative to "recall.cpp"
#include "../include/recall.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>

namespace nn {

int sign(double value)
{
  return (value >= 0) ? +1 : -1;
}

double hopfield_local_field(std::size_t index,
                            std::vector<int> const& current_state,
                            Weight_Matrix const& weight_matrix)
{
  assert(index >= 1 && index <= weight_matrix.neurons());
  assert(current_state.size() >= index);

  std::size_t j{0};
  auto local_field =
      std::accumulate(current_state.begin(), current_state.end(), 0.,
                      [index, &j, &weight_matrix](double sum, int value) {
                        ++j;
                        return sum + weight_matrix.at(index, j) * value;
                      });

  assert(j == current_state.size());

  return local_field;
}

std::vector<double> hopfield_local_fields(std::vector<int> const& current_state,
                                          Weight_Matrix const& weight_matrix)
{
  assert(weight_matrix.weights().size()
         == weight_matrix.neurons() * (weight_matrix.neurons() - 1) / 2);
  assert(current_state.size() == weight_matrix.neurons());

  std::vector<double> local_fields;
  std::size_t i{0};
  std::generate_n(
      std::back_inserter(local_fields), current_state.size(), [&]() {
        ++i;
        return hopfield_local_field(i, current_state, weight_matrix);
      });

  assert(i == current_state.size());
  assert(local_fields.size() == current_state.size());

  return local_fields;
}

double hopfield_energy(std::vector<int> const& current_state,
                       Weight_Matrix const& weight_matrix)
{
  assert(weight_matrix.weights().size()
         == weight_matrix.neurons() * (weight_matrix.neurons() - 1) / 2);

  auto local_fields = hopfield_local_fields(current_state, weight_matrix);

  double energy;
  energy = std::inner_product(current_state.begin(), current_state.end(),
                              local_fields.begin(), 0.);
  energy = -energy / 2;

  return energy;
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
    , current_state_{}
    , current_iteration_{0}
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

  assert(current_state_.size() == 0);
  assert(current_iteration_ == 0);

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

const std::vector<int>& Recall::current_state() const
{
  return current_state_;
}

std::size_t Recall::current_iteration() const
{
  return current_iteration_;
}

void Recall::clear_state()
{
  current_state_.clear();
  current_iteration_ = 0;
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
  noisy_pattern_.add_noise(0.1, 4096);
  assert(noisy_pattern_.size() == 4096);
  assert(std::all_of(noisy_pattern_.pattern().begin(),
                     noisy_pattern_.pattern().end(),
                     [](int value) { return value == +1 || value == -1; }));

  auto noisy_name = name.filename().replace_extension(".noise.txt");
  noisy_pattern_.save_to_file(corrupted_directory_, noisy_name, 4096);
  noisy_pattern_.create_image(corrupted_directory_, noisy_name, 64, 64);

  cut_pattern_ = original_pattern_;
  cut_pattern_.cut(-1, 34, 58, 11, 35, 64, 64);
  assert(cut_pattern_.size() == 4096);
  assert(std::all_of(cut_pattern_.pattern().begin(),
                     cut_pattern_.pattern().end(),
                     [](int value) { return value == +1 || value == -1; }));

  auto cut_name = name.filename().replace_extension(".cut.txt");
  cut_pattern_.save_to_file(corrupted_directory_, cut_name, 4096);
  cut_pattern_.create_image(corrupted_directory_, cut_name, 64, 64);
}

bool Recall::single_network_update()
{
  assert(current_state_.size() == 4096);
  assert(std::all_of(current_state_.begin(), current_state_.end(),
                     [](int value) { return value == +1 || value == -1; }));

  std::vector<int> new_state;
  std::size_t i{0};
  std::generate_n(std::back_inserter(new_state), current_state_.size(), [&]() {
    ++i;
    auto local_field = hopfield_local_field(i, current_state_, weight_matrix_);
    return sign(local_field);
  });

  assert(new_state.size() == 4096);
  assert(std::all_of(new_state.begin(), new_state.end(),
                     [](int value) { return value == +1 || value == -1; }));

  auto has_converged = (new_state == current_state_);

  current_state_ = new_state;
  ++current_iteration_;

  return !has_converged;
}

void Recall::network_update_dynamics()
{
  assert(weight_matrix_.neurons() == 4096);
  assert(weight_matrix_.weights().size() == 8'386'560);

  assert(noisy_pattern_.size() == 4096);
  assert(std::all_of(noisy_pattern_.pattern().begin(),
                     noisy_pattern_.pattern().end(),
                     [](int value) { return value == +1 || value == -1; }));
  assert(cut_pattern_.size() == 4096);
  assert(std::all_of(cut_pattern_.pattern().begin(),
                     cut_pattern_.pattern().end(),
                     [](int value) { return value == +1 || value == -1; }));

  assert(current_state_.size() == 0);

  // Choose between noisy_pattern_ and cut_pattern_
  current_state_ = noisy_pattern_.pattern();

  assert(current_state_.size() == 4096);
  assert(current_state_ == noisy_pattern_.pattern());

  auto original_energy =
      hopfield_energy(original_pattern_.pattern(), weight_matrix_);
  std::cout << "Original pattern's energy: " << original_energy << '\n';

  auto current_energy = hopfield_energy(current_state_, weight_matrix_);
  std::cout << "Initial energy: " << current_energy << '\n';

  assert(current_iteration_ == 0);

  while (single_network_update()) {
    current_energy = hopfield_energy(current_state_, weight_matrix_);
    std::cout << "Iteration " << current_iteration_
              << ". Current energy: " << current_energy << '\n';
  }

  assert(!single_network_update());
  assert(current_energy == hopfield_energy(current_state_, weight_matrix_));

  if (current_state_ == original_pattern_.pattern()) {
    assert(current_energy == original_energy);
    std::cout << "The original pattern has been recomposed." << '\n';
  } else {
    std::cout << "The original pattern has not been recomposed." << '\n';
  }
}

/*void Recall::network_update_dynamics()
{
  sf::Image starting_image, updated_image;
  starting_image.create(64, 64);
  updated_image.create(64, 64);

  unsigned int x{0}, y{0};

  for (auto value : noisy_pattern_.pattern()) {
    auto color = compute_color(value);
    starting_image.setPixel(x, y, color);
    updated_image.setPixel(x, y, color);

    if (x + 1 == 64) {
      x = 0;
      ++y;
    } else {
      ++x;
    }
  }
  assert(x == 0 && y == 64);
  assert(starting_image.getSize().x == 64 && starting_image.getSize().y == 64);
  assert(updated_image.getSize().x == 64 && updated_image.getSize().y == 64);

  // float scale = 5.f;
  sf::RenderWindow window(sf::VideoMode(2 * 64, 64), "Network update dynamics");

  sf::Texture starting_texture, updated_texture;

  starting_texture.loadFromImage(starting_image);
  sf::Sprite starting_sprite(starting_texture);
  // starting_sprite.setScale(scale, scale);
  starting_sprite.setPosition(0, 0);

  updated_texture.loadFromImage(updated_image);
  sf::Sprite updated_sprite(updated_texture);
  // updated_sprite.setScale(scale, scale);
  updated_sprite.setPosition(63, 0);

  window.clear();
  window.draw(starting_sprite);
  window.draw(updated_sprite);
  window.display();

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    x = 0;
    y = 0;

    for (auto current_value : current_state_) {
      updated_image.setPixel(x, y, compute_color(new_value));
      if (x + 1 == 64) {
        x = 0;
        ++y;
      } else {
        ++x;
      }
    }

    updated_texture.update(updated_image);

    window.clear();
    window.draw(starting_sprite);
    window.draw(updated_sprite);
    window.display();
  }
}*/

} // namespace nn