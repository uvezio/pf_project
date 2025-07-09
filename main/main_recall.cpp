/*
 * To run this program, execute from the build/ directory.

 * For example:
 *
 * $ cd build/
 * build$ Debug/recall
 *
 * Running from other directory may cause errors due to incorrect relative
 * paths.
 */

#include "../include/recall.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
  try {
    nn::Recall recall;

    recall.corrupt_pattern("ae.txt");
    recall.network_update_dynamics();
    recall.save_current_state("ae.txt");

  } catch (std::exception const& e) {
    std::cerr << "Caught exception: '" << e.what() << "'\n";
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Caught unknown exception\n";
    return EXIT_FAILURE;
  }
}