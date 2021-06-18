#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <fstream>

#include "fii_util.h"
#include "fii_image_size.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int create_dir_with_identical_img(const std::string dir1,
                                  std::vector<std::string> &filename_list1) {
  filename_list1.clear();
  std::string filename_template = "fii_test_tmp_file";
  std::vector<int> image_width_list = {50, 200, 1600};
  std::vector<int> image_height_list = {80, 225, 1000};
  std::vector<int> image_nchannel_list = {1, 3};
  std::vector<std::string> image_type_list = {"jpg", "png", "bmp"};

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 rand_gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> rand_pixel(0, 255);

  std::size_t filename_index = 0;
  std::size_t rand_pixel_data_id = 0;
  for(std::size_t iw=0; iw<image_width_list.size(); ++iw) {
    int width = image_width_list.at(iw);
    for(std::size_t ih=0; ih<image_height_list.size(); ++ih) {
      int height = image_height_list.at(ih);
      for(std::size_t ic=0; ic<image_nchannel_list.size(); ++ic) {
        int nchannel = image_nchannel_list.at(ic);
        for(std::size_t it=0; it<image_type_list.size(); ++it) {
          std::string type = image_type_list.at(it);

          // initialize image with random data
          int npixel = width * height * nchannel;
          std::vector<uint8_t> image_data(npixel);
          for(std::size_t px=0; px<npixel; ++px) {
            image_data[px] = rand_pixel(rand_gen);
          }
          rand_pixel_data_id++;

          int quality = 100;
          int success;
          std::ostringstream ss;
          ss << "fii_test_file" << "-"
             << width << "x" << height << "x" << nchannel
             << "-rand" << rand_pixel_data_id << "-" << filename_index
             << "." << type;
          std::string filename = dir1 + ss.str();

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
          if(success) {
            filename_list1.push_back(ss.str());
          }

          filename_index++;
        } // end of image_type_list
      } // end of image_nchannel_list
    } // end of image_height_list
  } // end of image_width_list

  // select 2 files and make a copy of them
  std::vector<uint32_t> file_index_list = {0, 7, 14, 17};

  for(std::size_t i=0; i<file_index_list.size(); ++i) {
    std::string src_filename = dir1 + filename_list1[ file_index_list[i] ];
    std::string src_ext = fii::fs_file_extension(filename_list1[ file_index_list[i] ]);
    std::string dst_filename =  filename_list1[ file_index_list[i] ] + "-COPY." + src_ext;
    std::string dst_filename_abspath = dir1 + dst_filename;
    std::ifstream fin(src_filename, std::ios::binary);
    std::ofstream fout(dst_filename_abspath, std::ios::binary);
    fout << fin.rdbuf();
    fin.close();
    fout.close();
    filename_list1.push_back(dst_filename);
  }

  return EXIT_SUCCESS;
}

int create_dir_with_identical_img(const std::string dir1,
                                  const std::string dir2,
                                  std::vector<std::string> &filename_list1,
                                  std::vector<std::string> &filename_list2) {
  filename_list1.clear();
  filename_list2.clear();
  std::string filename_template = "fii_test_tmp_file";
  std::vector<int> image_width_list = {3, 50, 443, 3};
  std::vector<int> image_height_list = {15, 373, 83, 7};
  std::vector<int> image_nchannel_list = {3};
  std::vector<std::string> image_type_list = {"jpg", "png", "bmp"};
  std::vector<std::string> outdir_list = {dir1, dir2};

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 rand_gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> rand_pixel(0, 255);

  std::size_t filename_index = 0;
  std::size_t rand_pixel_data_id = 0;
  for(std::size_t di=0; di<outdir_list.size(); ++di) {
    for(std::size_t iw=0; iw<image_width_list.size(); ++iw) {
      int width = image_width_list.at(iw);
      for(std::size_t ih=0; ih<image_height_list.size(); ++ih) {
        int height = image_height_list.at(ih);
        for(std::size_t ic=0; ic<image_nchannel_list.size(); ++ic) {
          int nchannel = image_nchannel_list.at(ic);
          for(std::size_t it=0; it<image_type_list.size(); ++it) {
            std::string type = image_type_list.at(it);

            // initialize image with random data
            int npixel = width * height * nchannel;
            std::vector<uint8_t> image_data(npixel);
            for(std::size_t px=0; px<npixel; ++px) {
              image_data[px] = rand_pixel(rand_gen);
            }
            rand_pixel_data_id++;

            int quality = 100;
            int success;
            std::ostringstream ss;
            ss << "fii_test_file" << "-"
               << width << "x" << height << "x" << nchannel
               << "-rand" << rand_pixel_data_id << "-" << filename_index
               << "." << type;
            std::string filename = outdir_list[di] + ss.str();

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
            if(success) {
              if(di == 0) {
                filename_list1.push_back(ss.str());
              }
              if(di == 1) {
                filename_list2.push_back(ss.str());
              }
            }

            filename_index++;
          } // end of outdir_list
        } // end of image_type_list
      } // end of image_nchannel_list
    } // end of image_height_list
  } // end of image_width_list
  return EXIT_SUCCESS;
}

int test_fii_on_dir(const std::string test_id,
                    const std::string dir1,
                    const std::string args,
                    const std::unordered_map<std::string, uint32_t> expected_output,
                    const std::string dir2="") {
  std::ostringstream ss;
  std::string cmd;
  if(dir2 == "") {
    ss << "./fii " << args << dir1 << std::endl;
  } else {
    ss << "./fii " << args << dir1 << " " << dir2 << std::endl;
  }
  cmd = ss.str();
  std::cout << "==[ " << test_id << ": " << cmd << std::endl;
  bool success = system(cmd.c_str());
  if(success != EXIT_SUCCESS) {
    std::cerr << "failed to execute the following command"
              << std::endl << cmd << std::endl;
    return EXIT_FAILURE;
  }

  std::unordered_map<std::string, uint32_t>::const_iterator it;
  for(it=expected_output.begin(); it!=expected_output.end(); ++it) {
    std::string expected_filename = fii::dir_to_cachedir(dir1) + it->first;
    uint32_t expected_filesize = it->second;
    if(expected_filesize == 0) {
      // file should not exist
      if(fii::fs_file_exists(expected_filename)) {
        std::cerr << test_id << " : unexpected presence of file "
                  << expected_filename << std::endl;
        return EXIT_FAILURE;
      }
    } else {
      std::string file_content;
      if(! fii::fs_load_file(expected_filename, file_content)) {
        std::cerr << test_id << " : missing file " << expected_filename
                  << std::endl;
        return EXIT_FAILURE;
      }
      if(file_content.size() != expected_filesize) {
        std::cerr << test_id << " : expected file size = " << expected_filesize
                  << ", actual file size = " << file_content.size()
                  << std::endl;
        return EXIT_FAILURE;
      }
    }
  }

  if(expected_output.size() == 0) {
    // ensure that no output files are produced
    std::string outdir = fii::dir_to_cachedir(dir1);
    std::vector<std::string> fn_list;
    fii::fs_list_all_files(outdir, fn_list);
    if(fn_list.size()) {
      std::cerr << test_id << " : produced unexpected files"
                << std::endl;
      return EXIT_FAILURE;
    }
  }

  if(!fii::remove_cache(dir1)) {
    std::cerr << test_id << " : failed to remove cache" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  int success = 0;
  fii::init_homedir_and_subdirs();

  std::string dir1 = fii::create_testdir("fii_test_dir1");
  std::string dir2 = fii::create_testdir("fii_test_dir2");
  std::string dir3 = fii::create_testdir("fii_test_dir3");

  std::vector<std::string> filename_list1;
  std::vector<std::string> filename_list2;
  std::vector<std::string> filename_list3;

  success = create_dir_with_identical_img(dir1, dir2,
                                          filename_list1,
                                          filename_list2);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  success = create_dir_with_identical_img(dir3, filename_list3);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  // test on a single folder containing no identical images
  success = test_fii_on_dir("dir1-no-identical",
                            dir1,
                            "",
                            {});
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  // test on a single folder containing 3 identical images
  std::unordered_map<std::string, uint32_t> dir3_3_identical = {
                                                                {"fii_test_dir3-identical.json",  532},
                                                                {"fii_test_dir3-identical.csv",   454},
                                                                {"fii_test_dir3-identical.html", 4708}
  };
  success = test_fii_on_dir("dir3-3-identical",
                            dir3,
                            "",
                            dir3_3_identical);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  // test on a single folder containing 3 identical images (exhaustive pixel search)
  success = test_fii_on_dir("dir3-3-identical-exhaustive",
                            dir3,
                            "--check-all-pixels ",
                            dir3_3_identical);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  // test on two folders (same dir1) resulting in all identical images
  success = test_fii_on_dir("dir1-dir1-all-identical",
                            dir1,
                            "",
                            {
                             {"fii_test_dir1-fii_test_dir1-identical.json", 5377},
                             {"fii_test_dir1-fii_test_dir1-identical.csv",  4930},
                             {"fii_test_dir1-fii_test_dir1-identical.html", 9626}
                            },
                            dir1);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  // test on two folders (dir1, dir2) resulting in 0 identical image
  success = test_fii_on_dir("dir1-dir2-0-identical",
                            dir1,
                            "",
                            {},
                            dir2);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  // test on two folders (same dir1) resulting in all identical images
  success = test_fii_on_dir("dir2-dir2-all-identical-exhaustive",
                            dir2,
                            "--check-all-pixels ",
                            {
                             {"fii_test_dir2-fii_test_dir2-identical.json", 5415},
                             {"fii_test_dir2-fii_test_dir2-identical.csv",  4968},
                             {"fii_test_dir2-fii_test_dir2-identical.html", 9664}
                            },
                            dir2);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  // cleanup
  fii::remove_testdir("fii_test_dir1");
  fii::remove_testdir("fii_test_dir2");
  fii::remove_testdir("fii_test_dir3");
  return EXIT_SUCCESS;
}
