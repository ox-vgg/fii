#include "fii_util.h"

//
// parse command line arguments
//
void fii::parse_command_line_args(int argc,
                                  char **argv,
                                  std::unordered_map<std::string, std::string> &options,
                                  std::vector<std::string> &dir_list) {
  for(uint32_t i=1; i<argc; ++i) {
    std::string arg(argv[i]);
    if(arg.size() < 3) {
      continue; // discard
    }
    if(arg[0] == '-' && arg[1] == '-') {
      // this is an option
      uint32_t eq_pos = arg.find('=');
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
      if(arg.back() != '/') {
        arg = arg + "/";
      }
      dir_list.push_back(arg);
    }
  }
}

//
// unix filesystem utility
//
bool fii::fs_is_dir(const std::string p) {
  struct stat p_stat;
  stat(p.c_str(), &p_stat);
  if( S_ISDIR(p_stat.st_mode) ) {
    return true;
  } else {
    return false;
  }
}

bool fii::fs_dir_exists(const std::string p) {
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

bool fii::fs_file_exists(const std::string p) {
  struct stat p_stat;
  return (stat(p.c_str(), &p_stat) == 0);
}

bool fii::fs_mkdir(const std::string p) {
  int result = mkdir(p.c_str(), 0755);
  if(result==0) {
    return true;
  } else {
    return false;
  }
}

bool fii::fs_mkdir_if_not_exists(const std::string p) {
  if(!fii::fs_dir_exists(p)) {
    return fii::fs_mkdir(p);
  } else {
    return true;
  }
}

void fii::fs_list_img_files(const std::string dirpath,
                            std::vector<std::string> &imfn_list,
                            uint32_t &discarded_file_count,
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
      uint32_t subdir_discarded_file_count;
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

bool fii::fs_load_file(const std::string fn, std::string& file_content) {
  if( !fs_file_exists(fn) ) {
    std::cout << "file does not exist [" << fn << "]" << std::endl;
    return false;
  }

  try {
    std::ifstream f;
    f.open(fn.c_str(), std::ios::in | std::ios::binary);
    f.seekg(0, std::ios::end);
    file_content.clear();
    file_content.reserve( f.tellg() );
    f.seekg(0, std::ios::beg);
    file_content.assign( (std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>() );
    f.close();
    return true;
  } catch(std::exception &e) {
    std::cout << "failed to load file [" << fn << "]" << std::endl;
    return false;
  }
}

std::string fii::fs_dirname(const std::string p) {
  std::vector<std::string> tokens;
  split(p, '/', tokens);
  if(tokens.size()) {
    return tokens.at(tokens.size() - 1);
  } else {
    return "";
  }
}

std::string fii::fs_file_extension(const std::string p) {
  std::size_t last_dot_index = p.rfind(".");
  if(last_dot_index == std::string::npos) {
    return "";
  } else {
    return p.substr(last_dot_index+1);
  }
}

//
// homedir and subdirs
//
std::string fii::homedir() {
  std::string user_home = getenv("HOME");
  return user_home + "/.fii/";
}

std::string fii::cachedir() {
  std::string fii_home = homedir();
  return fii_home + "cache/";
}

std::string fii::testdir() {
  std::string fii_home = homedir();
  return fii_home + "test/";
}

bool fii::init_homedir_and_subdirs() {
  if( !fs_mkdir_if_not_exists(homedir()) ) {
    std::cout << "failed to create fii home folder: "
              << fii::homedir() << std::endl;
    return false;
  }
  if( !fs_mkdir_if_not_exists(testdir()) ) {
    std::cout << "failed to create fii test folder: "
              << fii::testdir() << std::endl;
    return false;
  }
  if( !fs_mkdir_if_not_exists(cachedir()) ) {
    std::cout << "failed to create fii cache folder: "
              << fii::cachedir() << std::endl;
    return false;
  }

  return true;
}

std::string fii::create_cache_dir(const std::string target_dir) {
  std::string cache_home = cachedir();
  std::vector<std::string> target_dirname_list;
  split(target_dir, '/', target_dirname_list);

  std::string cache_dir(cache_home);
  for(uint32_t i=1; i<target_dirname_list.size(); ++i) {
    cache_dir += target_dirname_list[i] + "/";
    if(!fs_dir_exists(cache_dir)) {
      if(!fs_mkdir(cache_dir)) {
        return "";
      }
    }
  }
  return cache_dir;
}

std::string fii::dir_to_cachedir(const std::string target_dir) {
  std::string cache_home = cachedir();
  std::string target_dir_norm = target_dir;
  if(target_dir.front() == '/') {
    target_dir_norm = target_dir.substr(1); // discard the initial '/'
  }
  if(target_dir.back() != '/') {
    target_dir_norm = target_dir_norm + "/";
  }
  return cache_home + target_dir_norm;
}

bool fii::remove_cache(const std::string target_dir) {
  std::string cache_dir = dir_to_cachedir(target_dir);
  std::cout << "remove_cache(): " << cache_dir << std::endl;

  struct dirent *dp;
  DIR *dfd = opendir(cache_dir.c_str());
  if(!dfd) {
    std::cout << "cannot open cache dir: "
              << cache_dir << std::endl;
    return false;
  }

  while( (dp = readdir(dfd)) != NULL ) {
    std::string name(dp->d_name);
    if(name=="." || name=="..") {
      continue;
    }
    std::string filepath = cache_dir + name;
    if(fs_is_dir(filepath)) {
      continue; // avoid deleting directories
    } else {
      int file_result = unlink(filepath.c_str());
      std::cout << "file=" << filepath << ", "
                << "file_result=" << file_result
                << std::endl;
      if(file_result!=0) {
        return false;
      }
    }
  }
  int dir_result = rmdir(cache_dir.c_str());
  // rmdir() failure does not indicate an error
  // because other non-empty subfolders may exits.
  return true;
}

bool fii::clear_all_cache() {
  std::string cache_home = cachedir();
  std::cout << "Clearing cache: " << cache_home << std::endl;

  struct dirent *dp;
  DIR *dfd = opendir(cache_home.c_str());
  if(!dfd) {
    std::cout << "cannot open cache home: "
              << cache_home << std::endl;
    return false;
  }

  while( (dp = readdir(dfd)) != NULL ) {
    std::string name(dp->d_name);
    if(name=="." || name=="..") {
      continue;
    }
    std::string filepath = cache_home + name;
    if(fs_is_dir(filepath)) {
      std::string dirpath = filepath + "/";
      if(!clear_all_cache_subfolders(dirpath)) {
        return false;
      }
    } else {
      int file_result = unlink(filepath.c_str());
      if(file_result!=0) {
        return false;
      }
    }
  }

  return true;
}

bool fii::clear_all_cache_subfolders(std::string dirpath) {
  // for safety, ensure that dirpath is a subfolder of cache_home
  std::string cache_home = cachedir();
  std::string prefix = dirpath.substr(0, cache_home.length());
  if(prefix!=cache_home) {
    std::cout << "path must be a subfolder in cache" << std::endl;
    return false;
  }

  struct dirent *dp;
  DIR *dfd = opendir(dirpath.c_str());
  if(!dfd) {
    std::cout << "cannot open "
              << dirpath << std::endl;
    return false;
  }

  while( (dp = readdir(dfd)) != NULL ) {
    std::string name(dp->d_name);
    if(name=="." || name=="..") {
      continue;
    }
    std::string filepath = dirpath + name;
    if(fs_is_dir(filepath)) {
      std::string dirpath = filepath + "/";
      if(!clear_all_cache_subfolders(dirpath)) {
        return false;
      }
    } else {
      int file_result = unlink(filepath.c_str());
      if(file_result!=0) {
        return false;
      }
    }
  }
  int dir_result = rmdir(dirpath.c_str());
  if(dir_result!=0) {
    return false;
  }

  return true;
}

//
// testdir
//
std::string fii::create_testdir(const std::string test_name) {
  std::string test_home = testdir();
  std::string testdir = test_home + test_name + "/";
  if(!fs_dir_exists(testdir)) {
    if(!fs_mkdir(testdir)) {
      return "";
    }
  }
  return testdir;
}

bool fii::remove_testdir(const std::string test_name) {
  std::string test_home = testdir();
  std::string testdir = test_home + test_name + "/";

  struct dirent *dp;
  DIR *dfd = opendir(testdir.c_str());
  if(!dfd) {
    std::cout << "cannot open testdir: "
              << testdir << std::endl;
    return false;
  }

  while( (dp = readdir(dfd)) != NULL ) {
    std::string name(dp->d_name);
    if(name=="." || name=="..") {
      continue;
    }
    std::string filepath = testdir + name;
    int file_result = unlink(filepath.c_str());
    if(file_result!=0) {
      return false;
    }
  }
  int dir_result = rmdir(testdir.c_str());
  if(dir_result!=0) {
    return false;
  } else {
    return true;
  }
}

//
// misc
//
void fii::split(const std::string &s,
		const char separator,
		std::vector<std::string> &chunks) {
  chunks.clear();
  std::vector<uint32_t> seperator_index_list;

  uint32_t start = 0;
  uint32_t sep_index;
  while ( start < s.length() ) {
    sep_index = s.find(separator, start);
    if ( sep_index == std::string::npos ) {
      break;
    } else {
      chunks.push_back( s.substr(start, sep_index - start) );
      start = sep_index + 1;
    }
  }
  if ( start != s.length() ) {
    chunks.push_back( s.substr(start) );
  }
}

uint32_t fii::getmillisecs() {
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}
