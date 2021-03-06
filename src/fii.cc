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

#include "omp.h"

#include "fii_version.h"
#include "fii_usage.h"
#include "fii_util.h"
#include "fii_image_size.h"
#include "fii_export.h"
#include "fii.h"

int main(int argc, char **argv) {
  if(argc == 1) {
    std::cout << fii::FII_HELP_STR << std::endl;
    return EXIT_FAILURE;
  }

  uint32_t tstart = fii::getmillisecs();

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
  if(dir_list.size() > 2) {
    std::cout << "Only TARGET_DIR and CHECK_DIR should be provided."
              << std::endl;
    return EXIT_FAILURE;
  }

  fii::init_homedir_and_subdirs();

  // use all available threads by default
  int nthread = omp_get_max_threads();
  if(options.count("nthread")) {
    nthread = std::atoi(options["nthread"].c_str());
  }
  std::cout << "Using " << nthread << " threads" << std::endl;
  omp_set_dynamic(0);
  omp_set_num_threads(nthread);

  if(options.count("check-all-pixels")) {
    std::cout << "Performing exhaustive comparison of every pixels "
              << "(this is slower and requires more memory)" << std::endl;
  }

  std::string check_dir1(dir_list.at(0));
  std::string dir1_name = fii::fs_dirname(check_dir1);
  std::string cache_dir1 = fii::create_cache_dir(check_dir1);

  std::vector<std::string> filename_list1;
  uint32_t discarded_file_count1;
  std::unordered_map<std::string, std::vector<uint32_t> > buckets_of_img_index1;
  std::unordered_map<std::string, std::vector<uint32_t> > bucket_img_dim_list1;
  std::vector<std::string> bucket_id_list1;
  fii_group_by_img_dimension(check_dir1,
                             filename_list1,
                             buckets_of_img_index1,
                             bucket_img_dim_list1,
                             bucket_id_list1);

  // save histogram of images grouped by their dimension
  //std::string hist1_fn = cache_dir1 + dir1_name + "-img-dimension-histogram.csv";
  //fii_save_img_dimension_histogram(buckets_of_img_index1, bucket_id_list1, hist1_fn);

  if(dir_list.size() == 2) {
    // find identical images between check_dir1 and check_dir2
    std::string check_dir2(dir_list.at(1));
    std::string dir2_name = fii::fs_dirname(check_dir2);
    std::string cache_dir2 = fii::create_cache_dir(check_dir2);

    std::vector<std::string> filename_list2;
    uint32_t discarded_file_count2;
    std::unordered_map<std::string, std::vector<uint32_t> > buckets_of_img_index2;
    std::unordered_map<std::string, std::vector<uint32_t> > bucket_img_dim_list2;
    std::vector<std::string> bucket_id_list2;
    fii_group_by_img_dimension(check_dir2,
                               filename_list2,
                               buckets_of_img_index2,
                               bucket_img_dim_list2,
                               bucket_id_list2);

    // save histogram of images grouped by their dimension
    //std::string hist2_fn = cache_dir2 + dir2_name + "-img-dimension-histogram.csv";
    //fii_save_img_dimension_histogram(buckets_of_img_index2, bucket_id_list2, hist2_fn);

    uint32_t identical_img_count = 0;
    std::set<std::string> set_of_bucket_id2;
    for(uint32_t bindex=0; bindex!=bucket_id_list2.size(); ++bindex) {
      set_of_bucket_id2.insert(bucket_id_list2.at(bindex));
    }

    std::unordered_map<std::string, std::vector<std::set<uint32_t> > > image_groups;
    for(uint32_t bindex=0; bindex!=bucket_id_list1.size(); ++bindex) {
      std::string bucket_id = bucket_id_list1.at(bindex);
      if(set_of_bucket_id2.count(bucket_id) == 0) {
        // matching bucket does not exist in CHECK_DIR2
        // hence, no identical image possible
        continue;
      }
      if(bucket_id == "0x0x0") {
        // indicates malformed image, discard
        continue;
      }

      std::vector<std::set<uint32_t> > bucket_image_groups;
      // if check_all_pixels is requested, use the following two pass approach:
      // Pass 1. first compute identical images using sparse features
      // Pass 2. find identical images found in step [1] using exhaustive pixelwise search
      // this avoids running exhaustive pixelwise search on all images
      bool check_all_pixels = false;
      if(options.count("check-all-pixels")) {
	check_all_pixels = true;
	options.erase("check-all-pixels");
      }
      // Pass 1 
      fii_find_identical_img(filename_list1,
                             buckets_of_img_index1[bucket_id],
                             check_dir1,
                             filename_list2,
                             buckets_of_img_index2[bucket_id],
                             check_dir2,
                             bucket_img_dim_list1[bucket_id],
                             options,
                             bucket_image_groups);
      if(check_all_pixels) {
	// Pass 2
	std::vector<uint32_t> filename_index_list1_pass2;
	std::vector<uint32_t> filename_index_list2_pass2;
        std::vector<std::set<uint32_t> >::const_iterator gi;
        for(std::size_t group_id=0; group_id<bucket_image_groups.size(); ++group_id) {
          std::set<uint32_t> group_members(bucket_image_groups.at(group_id));
          std::set<uint32_t>::const_iterator si;
          for(si=group_members.begin(); si!=group_members.end(); ++si) {
            uint32_t findex = *si;
            if(findex >= filename_list1.size()) {
              // findex is from check_dir2
              findex = findex - filename_list1.size();
	      filename_index_list2_pass2.push_back(findex);
            } else {
              // findex is from check_dir1
	      filename_index_list1_pass2.push_back(findex);
	    }
          }
        }
	bucket_image_groups.clear();
	options["check-all-pixels"] = "";
	fii_find_identical_img(filename_list1,
			       filename_index_list1_pass2,
			       check_dir1,
			       filename_list2,
			       filename_index_list2_pass2,
			       check_dir2,
			       bucket_img_dim_list1[bucket_id],
			       options,
			       bucket_image_groups);
      }
      
      if(bucket_image_groups.size()) {
        image_groups[bucket_id] = bucket_image_groups;

        std::cout << bucket_id
                  << " (" << buckets_of_img_index1[bucket_id].size() << " images)"
                  << std::endl;

        for(std::size_t group_id=0; group_id<bucket_image_groups.size(); ++group_id) {
          std::set<uint32_t> group_members(bucket_image_groups.at(group_id));
          std::set<uint32_t>::const_iterator si;
          std::cout << "  [" << group_id << "] : ";
          for(si=group_members.begin(); si!=group_members.end(); ++si) {
            uint32_t findex = *si;
            std::string filename;
            std::string dir_name;
            if(findex >= filename_list1.size()) {
              // findex is from check_dir2
              findex = findex - filename_list1.size();
              filename = filename_list2.at(findex);
              dir_name = dir2_name + "/";
            } else {
              // findex is from check_dir1
              filename = filename_list1.at(findex);
              dir_name = dir1_name + "/";
            }
            if(si!= group_members.begin()) {
              std::cout << ", " << dir_name << filename;
            } else {
              std::cout << "" << dir_name << filename;
            }
          }

          identical_img_count += group_members.size() - 1;
          std::cout << std::endl;
        }
      }
    }
    std::cout << std::endl;

    uint32_t tend = fii::getmillisecs();
    double elapsed_sec = ((double)(tend - tstart)) / 1000.0;

    double time_per_image = elapsed_sec / ((double) (filename_list1.size()+filename_list2.size()));
    if(buckets_of_img_index1.count("0x0x0")) {
      std::cout << "Discarded " << buckets_of_img_index1["0x0x0"].size()
                << " malformed images in " << check_dir1 << std::endl;
    }
    if(buckets_of_img_index2.count("0x0x0")) {
      std::cout << "Discarded " << buckets_of_img_index2["0x0x0"].size()
                << " malformed images in " << check_dir2 << std::endl;
    }

    std::cout << "Processed two folders with " << filename_list1.size()
              << " and " << filename_list2.size() << " images in "
              << elapsed_sec << "s (" << time_per_image << "s per image)."
              << std::endl;
    if(image_groups.size()) {
      std::cout << "Found " << identical_img_count << " identical images."
                << std::endl;
      std::string export_dir = cache_dir1;
      if(options.count("export")) {
        export_dir = options.at("export");
      }
      fii_export_all(image_groups, export_dir, filename_list1, check_dir1, filename_list2, check_dir2);
    } else {
      std::cout << "Identical images not found." << std::endl;
    }
  } else {
    // find identical images within check_dir1
    bool is_first_entry = true;
    uint32_t identical_img_count = 0;
    std::unordered_map<std::string, std::vector<std::set<uint32_t> > > image_groups;
    for(uint32_t bindex=0; bindex!=bucket_id_list1.size(); ++bindex) {
      std::string bucket_id = bucket_id_list1.at(bindex);
      if(bucket_id == "0x0x0") {
        // indicates malformed image, discard
        continue;
      }

      std::vector<std::set<uint32_t> > bucket_image_groups;

      // if check_all_pixels is requested, use the following two pass approach:
      // Pass 1. first compute identical images using sparse features
      // Pass 2. find identical images found in step [1] using exhaustive pixelwise search
      // this avoids running exhaustive pixelwise search on all images
      bool check_all_pixels = false;
      if(options.count("check-all-pixels")) {
	check_all_pixels = true;
	options.erase("check-all-pixels");
      }
      // Pass 1 
      fii_find_identical_img(filename_list1,
                             buckets_of_img_index1[bucket_id],
                             check_dir1,
                             bucket_img_dim_list1[bucket_id],
                             options,
                             bucket_image_groups);
      if(check_all_pixels) {
	// Pass 2
	std::vector<uint32_t> filename_index_list1_pass2;
        std::vector<std::set<uint32_t> >::const_iterator gi;
        for(std::size_t group_id=0; group_id!=bucket_image_groups.size(); ++group_id) {
          std::set<uint32_t> group_members(bucket_image_groups.at(group_id));
          std::set<uint32_t>::const_iterator si;
          for(si=group_members.begin(); si!=group_members.end(); ++si) {
	    filename_index_list1_pass2.push_back(*si);
	  }
	}
	bucket_image_groups.clear();
	options["check-all-pixels"] = "";
	fii_find_identical_img(filename_list1,
			       filename_index_list1_pass2,
			       check_dir1,
			       bucket_img_dim_list1[bucket_id],
			       options,
			       bucket_image_groups);
      }
      
      if(bucket_image_groups.size()) {
        image_groups[bucket_id] = bucket_image_groups;

        std::cout << "Listing identical images of size " << bucket_id
                  << " (checked " << buckets_of_img_index1[bucket_id].size() << " images)"
                  << std::endl;

        std::vector<std::set<uint32_t> >::const_iterator gi;
        for(std::size_t group_id=0; group_id!=bucket_image_groups.size(); ++group_id) {
          std::set<uint32_t> group_members(bucket_image_groups.at(group_id));
          std::set<uint32_t>::const_iterator si;
          std::cout << "  [" << group_id << "] : ";
          for(si=group_members.begin(); si!=group_members.end(); ++si) {
            uint32_t findex = *si;
            if(si!= group_members.begin()) {
              std::cout << ", " << filename_list1.at(findex);
            } else {
              std::cout << filename_list1.at(findex);
            }
          }
          std::cout << std::endl;

          identical_img_count += group_members.size() - 1;
        }
      }
    }
    std::cout << std::endl;
    uint32_t tend = fii::getmillisecs();
    double elapsed_sec = ((double)(tend - tstart)) / 1000.0;

    double time_per_image = elapsed_sec / ((double) filename_list1.size());

    if(buckets_of_img_index1.count("0x0x0")) {
      std::cout << "Discarded " << buckets_of_img_index1["0x0x0"].size()
                << " malformed images in " << check_dir1 << std::endl;
    }

    if(image_groups.size()) {
      // @todo: remove non-portable Linux console colour code
      std::cout << "\033[0;31m"
		<< "Processed " << filename_list1.size() << " images and found "
		<< identical_img_count << " identical images. ("
		<< elapsed_sec << "s)"
		<< "\033[0m"
		<< std::endl;
      std::string export_dir = cache_dir1;
      if(options.count("export")) {
        export_dir = options.at("export");
      }
      fii_export_all(image_groups, export_dir, filename_list1, check_dir1);
    } else {
      // @todo: remove non-portable Linux console colour code
      std::cout << "\033[0;32m"
		<< "Processed " << filename_list1.size() << " images and found "
		<< "no identical images. ("
		<< elapsed_sec << "s)"
		<< "\033[0m"
		<< std::endl;
    }
  }
  return EXIT_SUCCESS;
}
