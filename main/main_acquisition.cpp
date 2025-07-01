/*
 * To run this program, execute from the build/ directory.

 * For example:
 *
 * $ cd build/
 * build$ Debug/acquisition
 *
 * Running from other directory may cause errors due to incorrect relative
 * paths.
 */

#include "../include/acquisition.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
  try {
    nn::Acquisition acquisition;

    acquisition.acquire_and_save_patterns();
    acquisition.save_binarized_images();

  } catch (std::exception const& e) {
    std::cerr << "Caught exception: '" << e.what() << "'\n";
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Caught unknown exception\n";
    return EXIT_FAILURE;
  }
}