// All relative paths are relative to the "build/" directory

/*
 * This test takes as input the patterns "1.txt", "2.txt", "3.txt", "4.txt" in
 * "../tests/patterns/" the weight matrix "weight_matrix.txt" in
 * "../tests/weight_matrix/" and generates the output files in
 * "../tests/corrupted_files/".
 *
 * This test writes temporary files to perform the necessary checks.
 *
 * This test uses implicitly output files of the test in "pattern.test.cpp"
 * controlling their actual removal during the construction of the nn::Recall
 * object in TEST_CASE "Testing the Recall class on invalid directories".
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// These two paths are the only ones relative to "recall.test.cpp"
#include "../../include/recall.hpp"
#include "../doctest.h"

#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>

TEST_CASE("Testing the free functions")
{
  nn::Weight_Matrix weight_matrix(4);
  REQUIRE(weight_matrix.weights().size() == 0);

  std::vector<std::vector<int>> patterns{{-1, 1, 1, -1}, {1, -1, -1, 1}};

  weight_matrix.fill(patterns, 4);
  REQUIRE(weight_matrix.weights().size() == 6);
  REQUIRE(weight_matrix.at(1, 2) == -.5);
  REQUIRE(weight_matrix.at(1, 3) == -.5);
  REQUIRE(weight_matrix.at(1, 4) == .5);
  REQUIRE(weight_matrix.at(2, 3) == .5);
  REQUIRE(weight_matrix.at(2, 4) == -.5);
  REQUIRE(weight_matrix.at(3, 4) == -.5);

  std::vector<int> current_state{-1, -1, 1, -1};

  SUBCASE("Checking the local field computations and the sign function")
  {
    CHECK(nn::hopfield_local_field(1, current_state, weight_matrix) == -.5);
    CHECK(nn::hopfield_local_field(2, current_state, weight_matrix) == 1.5);
    CHECK(nn::hopfield_local_field(3, current_state, weight_matrix) == .5);
    CHECK(nn::hopfield_local_field(4, current_state, weight_matrix) == -.5);

    CHECK(nn::sign(-.5) == -1);
    CHECK(nn::sign(1.5) == +1);
    CHECK(nn::sign(.5) == +1);
    CHECK(nn::sign(-.5) == -1);

    CHECK(nn::sign(0.) == +1);

    CHECK(nn::hopfield_local_field(1, {1}, weight_matrix) == 0.);
  }

  std::vector<int> state{1, -1, 1, -1};

  SUBCASE("Checking the local fields vector")
  {
    std::vector<double> local_fields{-.5, 1.5, .5, -.5};
    REQUIRE(nn::hopfield_local_fields(current_state, weight_matrix)
            == local_fields);

    std::vector fields{-.5, .5, -.5, .5};
    CHECK(nn::hopfield_local_fields(state, weight_matrix) == fields);
  }

  SUBCASE("Checking the energy computation")
  {
    CHECK(nn::hopfield_energy(current_state, weight_matrix) == 0.);
    CHECK(nn::hopfield_energy(state, weight_matrix) == 1.);
    CHECK(nn::hopfield_energy(patterns[0], weight_matrix) == -3.);
    CHECK(nn::hopfield_energy(patterns[1], weight_matrix) == -3.);
  }
}

TEST_CASE("Testing the Recall class on invalid directories")
{
  SUBCASE("Non existing patterns and weight matrix directory "
          "(\"../non_existing/patterns/\", \"../non_existing/weight_matrix/\")")
  {
    REQUIRE(!std::filesystem::exists("../non_existing/weight_matrix/"));
    REQUIRE(!std::filesystem::exists("../non_existing/patterns/"));
    CHECK_THROWS(nn::Recall("non_existing/"));
    REQUIRE(!std::filesystem::exists("../non_existing/weight_matrix/"));
    REQUIRE(!std::filesystem::exists("../non_existing/patterns/"));
  }

  SUBCASE("Weight matrix directory with a non regular file "
          "(\"../tests/weight_matrix/dir/\")")
  {
    std::filesystem::create_directory("../tests/weight_matrix/dir/");
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/weight_matrix/dir/");
  }

  SUBCASE("Weight matrix directory with a file different from "
          "\"weight_matrix.txt\"")
  {
    std::ofstream other{"../tests/weight_matrix/other.txt"};
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/weight_matrix/other.txt");
  }

  SUBCASE("Patterns directory with a non regular file "
          "(\"../tests/patterns/dir/\")")
  {
    std::filesystem::create_directory("../tests/patterns/dir/");
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/patterns/dir/");
  }

  SUBCASE("Patterns containing files with invalid extensions "
          "(\"../tests/patterns/invalid_extension.pdf\")")
  {
    std::ofstream inv{"../tests/patterns/invalid_extension.pdf"};
    CHECK_THROWS(nn::Recall("tests/"));
    std::filesystem::remove("../tests/patterns/invalid_extension.pdf");
  }

  SUBCASE("Non existing corrupted directory")
  {
    REQUIRE(std::filesystem::exists("../tests/corrupted_files/"));
    for (auto const& file :
         std::filesystem::directory_iterator("../tests/corrupted_files/")) {
      std::filesystem::remove_all(file);
    }
    std::filesystem::remove("../tests/corrupted_files/");
    REQUIRE(!std::filesystem::exists("../tests/corrupted_files/"));
    nn::Recall rec{"tests/"};
    CHECK(std::filesystem::exists("../tests/corrupted_files/"));
  }

  SUBCASE("Not empty corrupted directory")
  {
    std::ofstream img{"../tests/corrupted_files/0.txt"};
    REQUIRE(!std::filesystem::is_empty("../tests/corrupted_files/"));
    nn::Recall rec{"tests/"};
    CHECK(std::filesystem::is_empty("../tests/corrupted_files/"));
  }
}

nn::Recall recall{"tests/"};

TEST_CASE("Testing corrupt_pattern()")
{
  SUBCASE("Acquiring an under-sized pattern \"(under_sized.txt)\"")
  {
    std::ofstream undersized{"../tests/patterns/under_sized.txt"};
    for (int i{0}; i != 4095; ++i) {
      undersized << ((i % 3 == 0) ? +1 : -1);
    }
    undersized.close();
    // If incorrect is not closed here, file at
    // "../tests/patterns/under_sized.txt" could be written after the is_empty()
    // check in load_from_file()

    CHECK_THROWS(recall.corrupt_pattern("under_sized.txt"));
    std::filesystem::remove("../tests/patterns/under_sized.txt");
    REQUIRE(!std::filesystem::exists("../tests/patterns/under_sized.txt"));
  }

  SUBCASE("Corrupting and saving all the patterns in the directory")
  {
    for (int i{1}; i != 5; ++i) {
      std::filesystem::path name{std::to_string(i) + ".txt"};

      recall.corrupt_pattern(name);

      REQUIRE(recall.original_pattern().size() == 4096);
      REQUIRE(recall.noisy_pattern().size() == 4096);
      REQUIRE(recall.cut_pattern().size() == 4096);

      nn::Pattern corrupted_pattern;
      sf::Image corrupted_image;

      auto noisy_name = name;
      noisy_name.replace_extension(".noise.txt");
      CHECK(std::filesystem::is_regular_file("../tests/corrupted_files/"
                                             + noisy_name.string()));
      corrupted_pattern.load_from_file("../tests/corrupted_files/", noisy_name,
                                       4096);
      CHECK(corrupted_pattern.size() == 4096);

      noisy_name.replace_extension(".png");
      CHECK(corrupted_image.loadFromFile("../tests/corrupted_files/"
                                         + noisy_name.string()));
      CHECK(corrupted_image.getSize().x == 64);
      CHECK(corrupted_image.getSize().y == 64);

      auto cut_name = name;
      cut_name.replace_extension(".cut.txt");
      CHECK(std::filesystem::is_regular_file("../tests/corrupted_files/"
                                             + cut_name.string()));
      corrupted_pattern.load_from_file("../tests/corrupted_files/", cut_name,
                                       4096);
      CHECK(corrupted_pattern.size() == 4096);

      cut_name.replace_extension(".png");
      CHECK(corrupted_image.loadFromFile("../tests/corrupted_files/"
                                         + cut_name.string()));
      CHECK(corrupted_image.getSize().x == 64);
      CHECK(corrupted_image.getSize().y == 64);
    }
  }
}

std::vector<double> energies{-2075.32, -2261.52, -2071.11, -2257.45};

TEST_CASE("Testing network_update_dynamics()")
{
  REQUIRE(recall.weight_matrix().neurons() == 4096);
  REQUIRE(recall.weight_matrix().weights().size() == 8'386'560);

  for (std::size_t i{1}; i != 5; ++i) {
    std::filesystem::path name{std::to_string(i) + ".txt"};

    recall.corrupt_pattern(name);

    REQUIRE(recall.original_pattern().size() == 4096);
    REQUIRE(recall.noisy_pattern().size() == 4096);
    REQUIRE(recall.cut_pattern().size() == 4096);

    auto original_energy = nn::hopfield_energy(
        recall.original_pattern().pattern(), recall.weight_matrix());

    CHECK(original_energy == doctest::Approx(energies[i - 1]).epsilon(0.01));

    recall.clear_state();

    REQUIRE(recall.current_state().size() == 0);
    REQUIRE(recall.current_iteration() == 0);

    recall.network_update_dynamics();

    CHECK(recall.current_state().size() == 4096);
    CHECK(recall.current_iteration() > 0);

    auto final_energy =
        nn::hopfield_energy(recall.current_state(), recall.weight_matrix());

    // If the network converged to a pattern different from the original one,
    // the following check could fail.
    CHECK(final_energy >= original_energy);
  }
}

TEST_CASE("Testing the correct saving of the recomposed images")
{
  for (int i{1}; i != 5; ++i) {
    recall.clear_state();

    std::filesystem::path name{std::to_string(i) + ".txt"};
    recall.corrupt_pattern(name);
    recall.network_update_dynamics();
    recall.save_current_state(name);
  }
}