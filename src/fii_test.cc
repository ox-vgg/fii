#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <random>

#include "fii_util.h"
#include "fii_image_size.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

int main(int argc, char **argv) {
  int success = 0;
  fii::init_homedir_and_subdirs();

  std::string dir1 = fii::create_testdir("fii_test_dir1");
  std::string dir2 = fii::create_testdir("fii_test_dir2");

  std::vector<std::string> filename_list1;
  std::vector<std::string> filename_list2;
  success = create_dir_with_identical_img(dir1, dir2,
					  filename_list1,
					  filename_list2);
  if(success != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  std::ostringstream ss;
  std::string cmd;
  std::string json_fn;

  // test single dir
  ss << "./fii " << dir1 << std::endl;
  cmd = ss.str();
  std::cout << "====[ Test command: " << cmd << std::endl;
  success = system(cmd.c_str());
  if(success != EXIT_SUCCESS) {
    std::cerr << "failed to execute the following command"
	      << std::endl << cmd << std::endl;
    return EXIT_FAILURE;
  }
  std::string dir1_name = fii::fs_dirname(dir1);
  json_fn = fii::dir_to_cachedir(dir1) + dir1_name + "-identical.json";
  if(fii::fs_file_exists(json_fn)) {
    std::cerr << "Unexpected presence of file " << json_fn << std::endl;
    return EXIT_FAILURE;
  }
  fii::remove_cache(dir1);

  // test dir1 against dir1 (all identical images)
  ss.clear();
  ss.str("");
  ss << "./fii " << dir1 << " " << dir1 << std::endl;
  cmd = ss.str();
  std::cout << "====[ Test command: " << cmd << std::endl;
  success = system(cmd.c_str());
  if(success != EXIT_SUCCESS) {
    std::cerr << "failed to execute the following command"
	      << std::endl << cmd << std::endl;
    return EXIT_FAILURE;
  }

  json_fn = fii::dir_to_cachedir(dir1) + "fii_test_dir1-fii_test_dir1-identical.json";
  std::string json;
  if(! fii::fs_load_file(json_fn, json)) {
    std::cerr << "missing file " << json_fn << std::endl;
    return EXIT_FAILURE;
  }

  std::string csv_fn = fii::dir_to_cachedir(dir1) + "fii_test_dir1-fii_test_dir1-identical.csv";
  std::string csv;
  if(! fii::fs_load_file(csv_fn, csv)) {
    std::cerr << "missing file " << csv_fn << std::endl;
    return EXIT_FAILURE;
  }

  std::string html_fn = fii::dir_to_cachedir(dir1) + "fii_test_dir1-fii_test_dir1-identical.html";
  std::string html;
  if(! fii::fs_load_file(html_fn, html)) {
    std::cerr << "missing file " << html_fn << std::endl;
    return EXIT_FAILURE;
  }

  if(json.size() != 5377) {
    std::cerr << "unexpected content in " << json_fn << std::endl;
    return EXIT_FAILURE;
  }
  if(csv.size() != 4930) {
    std::cerr << "unexpected content in " << csv_fn << std::endl;
    return EXIT_FAILURE;
  }
  if(html.size() != 9626) {
    std::cerr << "unexpected content in " << html_fn << std::endl;
    return EXIT_FAILURE;
  }
  fii::remove_cache(dir1);

  // test dir1 against dir2 (0 identical images)
  ss.clear();
  ss.str("");
  ss << "./fii " << dir1 << " " << dir2 << std::endl;
  cmd = ss.str();
  std::cout << "====[ Test command: " << cmd << std::endl;
  success = system(cmd.c_str());
  if(success != EXIT_SUCCESS) {
    std::cerr << "failed to execute the following command"
	      << std::endl << cmd << std::endl;
    return EXIT_FAILURE;
  }
  json_fn = fii::dir_to_cachedir(dir1) + "fii_test_dir1-fii_test_dir2-identical.json";
  if(fii::fs_file_exists(json_fn)) {
    std::cerr << "Unexpect presence of file " << json_fn << std::endl;
    return EXIT_FAILURE;
  }
  fii::remove_cache(dir1);

  // cleanup
  fii::remove_testdir("fii_test_dir1");
  fii::remove_testdir("fii_test_dir2");
  return EXIT_SUCCESS;
}
