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

  // filesystem
  bool fs_dir_exists(const std::string p);
  bool fs_is_dir(const std::string p);
  bool fs_file_exists(const std::string p);
  bool fs_mkdir(const std::string p);
  bool fs_mkdir_if_not_exists(const std::string p);
  void fs_list_img_files(const std::string target_dir,
                         std::vector<std::string> &fn_list,
                         uint32_t &discarded_file_count,
                         std::string filename_prefix="");
  bool fs_load_file(const std::string fn, std::string& file_content);
  std::string fs_dirname(const std::string p);
  std::string fs_file_extension(const std::string p);

  // homedir and subdirs
  std::string homedir();
  std::string cachedir();
  std::string testdir();
  bool init_homedir_and_subdirs();
  
  // cache
  std::string create_cache_dir(const std::string target_dir);
  std::string dir_to_cachedir(const std::string target_dir);
  bool remove_cache(const std::string target_dir);
  bool clear_all_cache();
  bool clear_all_cache_subfolders(std::string dirpath);
  
  // tests
  std::string create_testdir(const std::string test_name);
  bool remove_testdir(const std::string test_name);

  // misc
  uint32_t getmillisecs();
  void split(const std::string &s,
	     const char separator,
	     std::vector<std::string> &chunks);
}

#endif
