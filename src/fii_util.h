/*
utility functions

Author: Abhishek Dutta <http://abhishekdutta.org>
*/

#ifndef FII_UTIL_H
#define FII_UTIL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <regex>
#include <fstream>
#include <chrono>
#include <cstdlib>

namespace fii {
  void parse_command_line_args(int argc,
                               char **argv,
                               std::unordered_map<std::string, std::string> &options,
                               std::vector<std::string> &dir_list);

  bool fs_dir_exists(std::string p);
  bool fs_is_dir(std::string p);
  bool fs_file_exists(std::string p);
  bool fs_mkdir(std::string p);
  bool fs_mkdir_if_not_exists(std::string p);
  void fs_list_img_files(std::string target_dir,
                         std::vector<std::string> &fn_list,
                         std::size_t &discarded_file_count,
                         std::string filename_prefix="");

  uint32_t getmillisecs();
}

#endif
