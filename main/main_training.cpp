/*
 * To run this program, execute from the build/ directory.

 * For example:
 *
 * $ cd build/
 * build$ Debug/training
 *
 * Running from other directory may cause errors due to incorrect relative
 * paths.
 */

#include "../include/training.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
  try {
    nn::Training training;

    training.acquire_and_save_weight_matrix();

  } catch (std::exception const& e) {
    std::cerr << "Caught exception: '" << e.what() << "'\n";
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Caught unknown exception\n";
    return EXIT_FAILURE;
  }
}