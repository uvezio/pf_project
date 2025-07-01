#include <cstdlib>
#include <exception>
#include <iostream>

#include "../include/acquisition.hpp"

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