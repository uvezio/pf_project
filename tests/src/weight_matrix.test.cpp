// All relative paths used at runtime are relative to the "build/" directory

/*
 * This test generates the files "empty_matrix.txt", "empty_matrix_1.txt",
 * "test1.txt", "test2.txt", "test.txt" in "../tests/weight_matrix/".
 * These files are implicitly removed in "training.test.cpp".
 *
 * This test does not use the patterns in "../tests/patterns/".
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// These two paths are the only ones relative to "weight_matrix.test.cpp"
#include "../../include/weight_matrix.hpp"
#include "../doctest.h"

#include <algorithm>
#include <fstream>

TEST_CASE("Testing index conversion")
{
  // N = 6
  // N * (N - 1) / 2 = 15

  SUBCASE("Checking matrix-to-vector index conversion")
  {
    CHECK(nn::matrix_to_vector_index(1, 2, 6) == 0);
    CHECK(nn::matrix_to_vector_index(1, 6, 6) == 4);
    CHECK(nn::matrix_to_vector_index(5, 6, 6) == 14);
    CHECK(nn::matrix_to_vector_index(3, 5, 6) == 10);
    CHECK(nn::matrix_to_vector_index(5, 3, 6) == 10);
    CHECK_THROWS(nn::matrix_to_vector_index(4, 4, 6));
    CHECK_THROWS(nn::matrix_to_vector_index(6, 6, 6));
    CHECK(nn::matrix_to_vector_index(4, 2, 6)
          == nn::matrix_to_vector_index(2, 4, 6));
  }

  SUBCASE("Checking vector-to-matrix implicit index conversion")
  {
    std::size_t i{1};
    std::size_t j{2};

    for (std::size_t k{0}; k != 15; ++k) {
      CHECK(nn::matrix_to_vector_index(i, j, 6) == k);
      nn::increment_ij(i, j, 6);

      CHECK(i + 1 <= j);
      if (k != 15 - 1) {
        CHECK(j <= 6);
      } else {
        CHECK(j == 6 + 1);
      }
    }
  }
}

TEST_CASE("Testing the ij element computation")
{
  SUBCASE("Five neurons and six patterns")
  {
    // N = 5

    std::vector<std::vector<int>> patterns{
        {1, -1, 1, 1, 1},   {-1, -1, 1, 1, -1},   {-1, 1, 1, -1, -1},
        {1, 1, -1, -1, -1}, {-1, -1, -1, -1, -1}, {1, 1, 1, 1, -1}};

    CHECK(nn::compute_weight_ij(1, 2, 5, patterns) == .4);
    CHECK(nn::compute_weight_ij(1, 5, 5, patterns) == .4);
    CHECK(nn::compute_weight_ij(2, 5, 5, patterns) == -.4);
    CHECK(nn::compute_weight_ij(3, 5, 5, patterns) == 0.);
    CHECK(nn::compute_weight_ij(2, 4, 5, patterns) == -.4);
    CHECK(nn::compute_weight_ij(4, 5, 5, patterns) == .4);
  }

  SUBCASE("Four neurons and two patterns")
  {
    nn::Weight_Matrix weight_matrix(4);

    std::vector<std::vector<int>> patterns{{-1, 1, 1, -1}, {1, -1, -1, 1}};

    CHECK(nn::compute_weight_ij(1, 2, 4, patterns) == -.5);
    CHECK(nn::compute_weight_ij(1, 3, 4, patterns) == -.5);
    CHECK(nn::compute_weight_ij(1, 4, 4, patterns) == .5);
    CHECK(nn::compute_weight_ij(2, 3, 4, patterns) == .5);
    CHECK(nn::compute_weight_ij(2, 4, 4, patterns) == -.5);
    CHECK(nn::compute_weight_ij(3, 4, 4, patterns) == -.5);
  }
}

TEST_CASE("Testing construction")
{
  nn::Weight_Matrix weight_matrix = nn::Weight_Matrix(6);
  REQUIRE(weight_matrix.neurons() == 6);
  REQUIRE(weight_matrix.weights().size() == 0);

  nn::Weight_Matrix weight_matrix_2 = nn::Weight_Matrix();
  REQUIRE(weight_matrix_2.neurons() == 4096);
  CHECK(weight_matrix_2.weights().size() == 0);
}

TEST_CASE("Testing the fill method")
{
  SUBCASE("Five neurons and six patterns")
  {
    nn::Weight_Matrix weight_matrix(5);
    REQUIRE(weight_matrix.weights().size() == 0);

    std::vector<std::vector<int>> patterns{
        {1, -1, 1, 1, 1},   {-1, -1, 1, 1, -1},   {-1, 1, 1, -1, -1},
        {1, 1, -1, -1, -1}, {-1, -1, -1, -1, -1}, {1, 1, 1, 1, -1}};

    weight_matrix.fill(patterns, 5);
    REQUIRE(weight_matrix.weights().size() == 10);

    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(1, 2, 5)] == .4);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(1, 5, 5)] == .4);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(2, 5, 5)]
            == -.4);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(3, 5, 5)] == 0.);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(2, 4, 5)]
            == -.4);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(4, 5, 5)] == .4);

    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(1, 3, 5)] == 0.);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(1, 4, 5)] == .4);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(2, 3, 5)] == 0.);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(3, 4, 5)] == .8);
  }

  SUBCASE("Four neurons and two patterns")
  {
    nn::Weight_Matrix weight_matrix(4);
    REQUIRE(weight_matrix.weights().size() == 0);

    std::vector<std::vector<int>> patterns{{-1, 1, 1, -1}, {1, -1, -1, 1}};

    weight_matrix.fill(patterns, 4);
    REQUIRE(weight_matrix.weights().size() == 6);

    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(1, 2, 4)]
            == -.5);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(1, 3, 4)]
            == -.5);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(1, 4, 4)] == .5);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(2, 3, 4)] == .5);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(2, 4, 4)]
            == -.5);
    REQUIRE(weight_matrix.weights()[nn::matrix_to_vector_index(3, 4, 4)]
            == -.5);
  }

  SUBCASE("Filling two times the same weight matrix")
  {
    nn::Weight_Matrix weight_matrix(4);
    REQUIRE(weight_matrix.weights().size() == 0);

    std::vector<std::vector<int>> patterns{{-1, 1, 1, -1}, {1, -1, -1, 1}};

    weight_matrix.fill(patterns, 4);
    REQUIRE(weight_matrix.weights().size() == 6);

    std::vector<std::vector<int>> patterns_{{-1, -1, 1, 1}, {1, 1, -1, -1}};

    weight_matrix.fill(patterns_, 4);
    REQUIRE(weight_matrix.weights().size() == 6);

    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(2, 3, 4)] == -.5);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(2, 4, 4)] == -.5);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(2, 1, 4)] == .5);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(3, 4, 4)] == .5);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(3, 1, 4)] == -.5);
    CHECK(weight_matrix.weights()[nn::matrix_to_vector_index(4, 1, 4)] == -.5);
  }
}

TEST_CASE("Testing the at method")
{
  nn::Weight_Matrix weight_matrix(4);
  REQUIRE(weight_matrix.weights().size() == 0);

  std::vector<std::vector<int>> patterns{{-1, 1, 1, -1}, {1, -1, -1, 1}};

  weight_matrix.fill(patterns, 4);
  REQUIRE(weight_matrix.weights().size() == 6);

  CHECK(weight_matrix.at(1, 2) == -.5);
  CHECK(weight_matrix.at(1, 3) == -.5);
  CHECK(weight_matrix.at(1, 4) == .5);
  CHECK(weight_matrix.at(2, 3) == .5);
  CHECK(weight_matrix.at(2, 4) == -.5);
  CHECK(weight_matrix.at(3, 4) == -.5);

  CHECK(weight_matrix.at(2, 1) == -.5);
  CHECK(weight_matrix.at(3, 1) == -.5);
  CHECK(weight_matrix.at(4, 1) == .5);
  CHECK(weight_matrix.at(3, 2) == .5);
  CHECK(weight_matrix.at(4, 2) == -.5);
  CHECK(weight_matrix.at(4, 3) == -.5);

  CHECK(weight_matrix.at(1, 1) == 0.);
  CHECK(weight_matrix.at(2, 2) == 0.);
  CHECK(weight_matrix.at(3, 3) == 0.);
  CHECK(weight_matrix.at(4, 4) == 0.);
}

TEST_CASE("Testing input and output")
{
  SUBCASE("Saving an empty weight matrix")
  {
    nn::Weight_Matrix weight_matrix(0);
    REQUIRE(weight_matrix.weights().size() == 0);
    weight_matrix.save_to_file("../tests/weight_matrix/", "empty_matrix.txt",
                               0);
    CHECK(std::filesystem::is_regular_file(
        "../tests/weight_matrix/empty_matrix.txt"));
    CHECK(std::filesystem::is_empty("../tests/weight_matrix/empty_matrix.txt"));
    weight_matrix.load_from_file("../tests/weight_matrix/", "empty_matrix.txt",
                                 0);
    CHECK(weight_matrix.weights().size() == 0);

    nn::Weight_Matrix weight_matrix_1(1);
    REQUIRE(weight_matrix_1.weights().size() == 0);
    weight_matrix_1.save_to_file("../tests/weight_matrix/",
                                 "empty_matrix_1.txt", 1);
    CHECK(std::filesystem::is_regular_file(
        "../tests/weight_matrix/empty_matrix_1.txt"));
    CHECK(
        std::filesystem::is_empty("../tests/weight_matrix/empty_matrix_1.txt"));
    weight_matrix_1.load_from_file("../tests/weight_matrix/",
                                   "empty_matrix_1.txt", 1);
    CHECK(weight_matrix_1.weights().size() == 0);
  }

  SUBCASE("Saving pattern to file and loading the same pattern")
  {
    nn::Weight_Matrix weight_matrix_1(5);
    std::vector<std::vector<int>> patterns_1{
        {1, -1, 1, 1, 1},   {-1, -1, 1, 1, -1},   {-1, 1, 1, -1, -1},
        {1, 1, -1, -1, -1}, {-1, -1, -1, -1, -1}, {1, 1, 1, 1, -1}};

    weight_matrix_1.fill(patterns_1, 5);
    REQUIRE(weight_matrix_1.weights().size() == 10);

    weight_matrix_1.save_to_file("../tests/weight_matrix/", "test1.txt", 5);
    CHECK(std::filesystem::is_regular_file("../tests/weight_matrix/test1.txt"));
    CHECK(!std::filesystem::is_empty("../tests/weight_matrix/test1.txt"));
    weight_matrix_1.load_from_file("../tests/weight_matrix/", "test1.txt", 5);
    CHECK(weight_matrix_1.weights().size() == 10);

    nn::Weight_Matrix weight_matrix_2(4);
    std::vector<std::vector<int>> patterns_2{{-1, 1, 1, -1}, {1, -1, -1, 1}};

    weight_matrix_2.fill(patterns_2, 4);
    REQUIRE(weight_matrix_2.weights().size() == 6);

    weight_matrix_2.save_to_file("../tests/weight_matrix/", "test2.txt", 4);
    CHECK(std::filesystem::is_regular_file("../tests/weight_matrix/test2.txt"));
    CHECK(!std::filesystem::is_empty("../tests/weight_matrix/test2.txt"));
    weight_matrix_2.load_from_file("../tests/weight_matrix/", "test2.txt", 4);
    CHECK(weight_matrix_2.weights().size() == 6);
  }

  std::vector<double> values{0.1,   -0.3,  -0.22, 0.,   1.3,
                             -10.2, -0.47, -2.56, 0.98, -0.03};
  std::ofstream test{"../tests/weight_matrix/test.txt"};
  for (auto v : values) {
    test << v << ' ';
  }
  test.close();
  // If test is not closed here, file at "../tests/weight_matrix/test.txt"
  // could be written after the is_empty() check in load_from_file()

  SUBCASE("Loading a well-sized weight matrix")
  {
    nn::Weight_Matrix wm(5);
    wm.load_from_file("../tests/weight_matrix/", "test.txt", 5);
    CHECK(wm.weights().size() == 10);

    std::size_t i{0};
    CHECK((wm.weights() == values));
    CHECK(std::all_of(wm.weights().begin(), wm.weights().end(),
                      [&i, &values](double w) {
                        ++i;
                        return w == values[i - 1];
                      }));
    CHECK(i == 10);
  }

  SUBCASE("Loading an under-sized weight matrix")
  {
    nn::Weight_Matrix wm(6);
    CHECK_THROWS(wm.load_from_file("../tests/weight_matrix/", "test.txt", 6));
  }

  SUBCASE("Loading an over-sized weight matrix")
  {
    nn::Weight_Matrix wm(4);
    CHECK_THROWS(wm.load_from_file("../tests/weight_matrix/", "test.txt", 4));
  }

  SUBCASE("Overwriting an existing weight matrix with a loaded one")
  {
    nn::Weight_Matrix wm(5);
    wm.load_from_file("../tests/weight_matrix/", "test.txt", 5);
    REQUIRE(wm.weights().size() == 10);
    wm.load_from_file("../tests/weight_matrix/", "test1.txt", 5);
    CHECK(wm.weights().size() == 10);
  }
}
