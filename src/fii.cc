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

  std::string target_dir(dir_list.at(0));
  std::vector<std::string> filename_list;
  fii::fs_list_img_files(target_dir, filename_list);

  int width, height, nchannel;
  std::unordered_map<std::string, std::vector<std::size_t> > buckets_of_img_index;
  std::unordered_map<std::string, std::size_t > buckets_img_count;
  std::string file_path;
  uint32_t tstart = fii::getmillisecs();
  std::cout << "Grouping " << filename_list.size() << " files based on their size ..."
            << std::flush;
  for(std::size_t i=0; i<filename_list.size(); ++i) {
    file_path = target_dir + "/" + filename_list[i];
    fii_image_size(file_path.c_str(),
                   &width,
                   &height,
                   &nchannel);

    std::string img_dim_id = fii_img_dim_id(width, height, nchannel);
    buckets_of_img_index[img_dim_id].push_back(i);
    buckets_img_count[img_dim_id] += 1;
  }
  std::cout << " found " << buckets_img_count.size() << " unique image sizes."
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
