#include <iostream>
#include <iomanip>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#endif

int main(int argc, char **argv) {
  if(argc != 2) {
    std::cout << "Usage: " << argv[0] << " image_filename" << std::endl;
    return EXIT_FAILURE;
  }

  int width, height, nchannel;
  unsigned char *img_data = stbi_load(argv[1], &width, &height, &nchannel, 0);
  if(!img_data) {
    // malformed image, discard
    std::cout << "malformed image: " << argv[1] << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "showing " << width << "x" << height << "x" << nchannel
	    << " pixel values of " << argv[1] << std::endl;
  for(uint32_t yi=0; yi<height; ++yi) {
    std::cout << "[" << std::setw(2) << yi << "] ";
    for(uint32_t xi=0; xi<width; ++xi) {
      std::cout << "(";
      std::cout << std::setw(3) << (int) img_data[yi*width*nchannel + xi*nchannel];
      for(uint32_t ci=1; ci<nchannel; ++ci) {
	std::cout << "," << std::setw(3) << (int) img_data[yi*width*nchannel + xi*nchannel];
      }
      std::cout << ") ";
    }
    std::cout << std::endl;
  }
  stbi_image_free(img_data);
}
