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
#include <sstream>

#include "omp.h"

#include "fii_version.h"
#include "fii_usage.h"
#include "fii_util.h"
#include "fii_image_size.h"
#include "fii.h"

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

  int nthread = omp_get_max_threads();
  omp_set_num_threads(nthread);
  std::cout << "using " << nthread << " threads" << std::endl;

  uint32_t t0, t1;
  std::string target_dir(dir_list.at(0));
  std::vector<std::string> filename_list;
  std::size_t discarded_file_count;
  std::cout << "Collecting list of filenames ..." << std::flush;
  t0 = fii::getmillisecs();
  fii::fs_list_img_files(target_dir, filename_list, discarded_file_count);
  t1 = fii::getmillisecs();
  std::cout << " found " << filename_list.size()
	    << ", discarded " << discarded_file_count
	    << " (" << (((double)(t1 - t0)) / 1000.0) << "s)"
            << std::endl;

  int width, height, nchannel;
  std::unordered_map<std::string, std::vector<std::size_t> > buckets_of_img_index;
  std::unordered_map<std::string, std::size_t > buckets_img_count;
  uint32_t tstart = fii::getmillisecs();
  std::cout << "Grouping " << filename_list.size() << " files based on their size ..."
            << std::flush;
  t0 = fii::getmillisecs();

  std::vector<int> filename_width_list(filename_list.size());
  std::vector<int> filename_height_list(filename_list.size());
  std::vector<int> filename_nchannel_list(filename_list.size());

  omp_set_num_threads(16);
#pragma omp parallel
  {
    int nt = omp_get_num_threads();
    int rank = omp_get_thread_num();

    // this thread is taking care of files from index fi0 to fi1
    std::size_t fi0 = (filename_list.size() * rank) / nt;
    std::size_t fi1 = (filename_list.size() * (rank + 1)) / nt;
    std::ostringstream ss;
    ss << "[" << rank << "] processing files from " << fi0 << " to " << fi1 << std::endl;
    std::cout << ss.str();
    for(std::size_t i=fi0; i<fi1; ++i) {
      std::string file_path = target_dir + "/" + filename_list[i];
      fii_image_size(file_path.c_str(),
		     &filename_width_list[i],
		     &filename_height_list[i],
		     &filename_nchannel_list[i]);
    }
  } // end of omp parallel
  
  for(std::size_t i=0; i<filename_list.size(); ++i) {
    std::string img_dim_id = fii_img_dim_id(filename_width_list[i],
					    filename_height_list[i],
					    filename_nchannel_list[i]);
    buckets_of_img_index[img_dim_id].push_back(i);
    buckets_img_count[img_dim_id] += 1;
  }
  t1 = fii::getmillisecs();
  std::cout << " found " << buckets_img_count.size() << " unique image sizes."
	    << " (" << (((double)(t1 - t0)) / 1000.0) << "s)"
            << std::endl;

  // find identical image in each bucket
  std::unordered_map<std::string, std::size_t>::const_iterator itr;
  for(itr=buckets_img_count.begin(); itr!=buckets_img_count.end(); ++itr) {
    if(itr->second == 1) {
      continue; // a bucket with only 1 image cannot contain have identical images
    } else {
      std::string bucket_id = itr->first;
      std::unordered_map<std::size_t, std::set<std::size_t> > image_groups;
      fii_find_identical_img(filename_list,
                             buckets_of_img_index[bucket_id],
                             target_dir,
                             image_groups);
      if(image_groups.size()) {
        std::cout << itr->second << " images of size "
                  << itr->first << " have "
                  << image_groups.size() << " identical groups."
                  << std::endl;

        std::unordered_map<std::size_t, std::set<std::size_t> >::const_iterator gi;
        for(gi=image_groups.begin(); gi!=image_groups.end(); ++gi) {
          std::size_t group_id = gi->first;
          std::set<std::size_t> group_members(gi->second);
          std::set<std::size_t>::const_iterator si;
          std::cout << "  [" << group_id << "] ";
          for(si=group_members.begin(); si!=group_members.end(); ++si) {
            std::size_t findex = *si;
            if(si!= group_members.begin()) {
              std::cout << ", " << filename_list.at(findex);
            } else {
              std::cout << filename_list.at(findex);
            }
          }
          std::cout << std::endl;
        }
      }
    }
  }
  uint32_t tend = fii::getmillisecs();
  double elapsed_sec = ((double)(tend - tstart)) / 1000.0;

  double time_per_image = elapsed_sec / ((double) filename_list.size());
  std::cout << "processed " << filename_list.size() << " images in "
            << elapsed_sec << "s (" << time_per_image << "s per image)"
            << std::endl;

  return 0;
}
