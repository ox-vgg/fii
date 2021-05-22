#include <iostream>
#include <string>
#include <vector>
#include <cstdio>

#include "fii_util.h"
#include "fii_image_size.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char **argv) {
  std::string filename_template = "fii_image_size_test_tmp_file";
  std::vector<int> image_width_list = {3, 5000, 1};
  std::vector<int> image_height_list = {15, 1, 4652};
  std::vector<int> image_nchannel_list = {3};
  std::vector<std::string> image_type_list = {"jpg", "png", "bmp"};

  for(std::size_t iw=0; iw<image_width_list.size(); ++iw) {
    int width = image_width_list.at(iw);
    for(std::size_t ih=0; ih<image_height_list.size(); ++ih) {
      int height = image_height_list.at(ih);
      for(std::size_t ic=0; ic<image_nchannel_list.size(); ++ic) {
        int nchannel = image_nchannel_list.at(ic);
        for(std::size_t it=0; it<image_type_list.size(); ++it) {
          std::string type = image_type_list.at(it);
          std::string filename = filename_template + "." + type;

          std::cout << "Testing " << type << " image of size "
                    << width << "x" << height << "x" << nchannel
                    << " ..." << std::endl;
          int npixel = width * height * nchannel;
          std::vector<uint8_t> image_data(npixel);

          int quality = 100;
          int success;
          if(type == "jpg") {
            success = stbi_write_jpg(filename.c_str(),
                                     width, height, nchannel,
                                     image_data.data(),
                                     quality);
          } else if(type == "png") {
            success = stbi_write_png(filename.c_str(),
                                     width, height, nchannel,
                                     image_data.data(),
                                     width * nchannel);
          } else if(type == "bmp") {
            success = stbi_write_bmp(filename.c_str(),
                                     width, height, nchannel,
                                     image_data.data());
          }
          if(!success) {
            std::cout << "failed to create " << type << " test image: "
                      << filename << std::endl;
            return EXIT_FAILURE;
          }

          // read image size
          int got_width, got_height, got_nchannel;
          fii_image_size(filename.c_str(),
                         &got_width, &got_height, &got_nchannel);

          std::remove(filename.c_str());

          if(got_width != width ||
             got_height != height ||
             got_nchannel != nchannel) {
            std::cout << type << " image size mismatch!"
                      << " got: "
                      << got_width << "x" << got_height << "x" << got_nchannel
                      << " expected: "
                      << width << "x" << height << "x" << nchannel
                      << std::endl;
            return EXIT_FAILURE;
          }
        }
      }
    }
  }
  return EXIT_SUCCESS;
}
