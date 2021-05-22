/*
A command line tool to find all identical images (i.e. exact duplicate images
that have same pixel values) in a folder.

Author: Abhishek Dutta <http://abhishekdutta.org>
Date:   06-Dec-2020

Revision History: see CHANGELOG.txt file
*/

#include <iostream>
#include <string>
#include <unordered_map>
#include <array>

#include "fii_version.h"
#include "fii_usage.h"
#include "fii_util.h"
#include "fii_image_size.h"

int main(int argc, char **argv) {
  if(argc == 1 || argc != 2) {
    std::cout << fii::FII_USAGE_STR << std::endl;
    return EXIT_FAILURE;
  }

  // parse command line arguments
  std::unordered_map<std::string, std::string> options;
  std::vector<std::string> dir_list;
  fii::parse_command_line_args(argc, argv, options, dir_list);

  if(options.count("help")) {
    std::cout << fii::FII_HELP_STR << std::endl;
    return EXIT_FAILURE;
  }

  if(options.count("version")) {
    std::cout << FII_FULLNAME << " (" << FII_NAME << ") "
              << FII_VERSION_MAJOR << "." << FII_VERSION_MINOR << "."
              << FII_VERSION_PATCH
              << std::endl;
    return EXIT_SUCCESS;
  }

  if(dir_list.size() == 0) {
    std::cout << "At least one folder path must be provided."
              << std::endl;
    return EXIT_FAILURE;
  }

  std::string target_dir(dir_list.at(0));
  std::vector<std::string> filename_list;
  fii::fs_list_img_files(target_dir, filename_list);

  int width, height, nchannel;
  std::string file_path;
  uint32_t tstart = fii::getmillisecs();
  for(std::size_t i=0; i<filename_list.size(); ++i) {
    file_path = target_dir + "/" + filename_list[i];
    fii_image_size(file_path.c_str(),
                   &width,
                   &height,
                   &nchannel);
    std::cout << filename_list[i] << " : "
              << width << " x " << height << " x " << nchannel
              << std::endl;
  }
  uint32_t tend = fii::getmillisecs();
  double elapsed_sec = ((double)(tend - tstart)) / 1000.0;

  double time_per_image = (double) filename_list.size() / elapsed_sec;
  std::cout << "processed " << filename_list.size() << " images in "
            << elapsed_sec << "s (" << time_per_image << "s per image)"
            << std::endl;
  return 0;
}
