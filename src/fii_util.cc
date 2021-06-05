#include "fii_util.h"

//
// parse command line arguments
//
void fii::parse_command_line_args(int argc,
                                  char **argv,
                                  std::unordered_map<std::string, std::string> &options,
                                  std::vector<std::string> &dir_list) {
  for(std::size_t i=1; i<argc; ++i) {
    std::string arg(argv[i]);
    if(arg.size() < 3) {
      continue; // discard
    }
    if(arg[0] == '-' && arg[1] == '-') {
      // this is an option
      std::size_t eq_pos = arg.find('=');
      std::string key, val;
      if(eq_pos == std::string::npos) {
        // for arguments without a value (e.g. --version, --help, ...)
        key = arg.substr(2, arg.size());
        val = "";
      } else {
        key = arg.substr(2, eq_pos-2);
        val = arg.substr(eq_pos+1);
      }
      if(options.count(key) == 1) {
        std::cout << "Duplicate arguments for [" << key << "], "
                  << "last one will be used."
                  << std::endl;
      }
      options[key] = val;
    } else {
      dir_list.push_back(arg);
    }
  }
}

//
// unix filesystem utility
//
bool fii::fs_is_dir(std::string p) {
  struct stat p_stat;
  stat(p.c_str(), &p_stat);
  if( S_ISDIR(p_stat.st_mode) ) {
    return true;
  } else {
    return false;
  }
}

bool fii::fs_dir_exists(std::string p) {
  struct stat p_stat;
  if(stat(p.c_str(), &p_stat) != 0) {
    return false;
  }
  if( S_ISDIR(p_stat.st_mode) ) {
    return true;
  } else {
    return false;
  }
}

bool fii::fs_file_exists(std::string p) {
  struct stat p_stat;
  return (stat(p.c_str(), &p_stat) == 0);
}

bool fii::fs_mkdir(std::string p) {
  int result = mkdir(p.c_str(), 0755);
  if(result==0) {
    return true;
  } else {
    return false;
  }
}

bool fii::fs_mkdir_if_not_exists(std::string p) {
  if(!fii::fs_dir_exists(p)) {
    return fii::fs_mkdir(p);
  } else {
    return true;
  }
}

void fii::fs_list_img_files(std::string dirpath,
                            std::vector<std::string> &imfn_list,
                            std::size_t &discarded_file_count,
                            std::string filename_prefix) {
  discarded_file_count = 0;
  std::string imfn_regex(".*(.jpg|.jpeg|.png|.bmp|.pnm|.tif)$");
  std::regex filename_regex(imfn_regex,
                            std::regex_constants::extended |
                            std::regex_constants::icase);

  struct dirent *dp;
  DIR *dfd = opendir(dirpath.c_str());
  if(!dfd) {
    std::cout << "fs_list_img_files(): cannot open dir: "
              << dirpath << std::endl;
    return;
  }

  while( (dp = readdir(dfd)) != NULL ) {
    std::string name(dp->d_name);
    if(name=="." || name=="..") {
      continue;
    }
    std::string path = dirpath + name;
    if(fs_is_dir(path)) {
      std::string subdir_name = filename_prefix + name + "/";
      std::string subdirpath = path + "/";
      std::vector<std::string> imfn_sublist;
      std::size_t subdir_discarded_file_count;
      fs_list_img_files(subdirpath, imfn_sublist, subdir_discarded_file_count, subdir_name);
      discarded_file_count += subdir_discarded_file_count;
      imfn_list.insert(imfn_list.end(),
                       std::make_move_iterator(imfn_sublist.begin()),
                       std::make_move_iterator(imfn_sublist.end()));
    } else {
      if( std::regex_match(name, filename_regex) ) {
        if(filename_prefix.empty()) {
          imfn_list.push_back(name);
        } else {
          std::string name_with_prefix = filename_prefix + name;
          imfn_list.push_back(name_with_prefix);
        }
      } else {
        discarded_file_count++;
        //std::cout << "discarded: " << name << std::endl;
      }
    }
  }
  closedir(dfd);
}

uint32_t fii::getmillisecs() {
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}
