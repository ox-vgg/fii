/*
  find identical images based on analysis of sparse set of pixel values.

  Author: Abhishek Dutta <http://abhishekdutta.org>

  Revision History:
  04-Jun-2021 : image_feature = pixel values at 8 feature points around image center

*/

#ifndef FII_H
#define FII_H

#include <sstream>
#include <cmath>
#include <set>
#include <unordered_map>
#include <vector>
#include <iomanip>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#endif

// a sparse sample of pixel values are compared
// if W and H are the image width and image height respectively
// then pixel value features are extracted from the following pixel locations
// for x in FII_IMG_FEATURE_LOC_SCALE .* W
//   for y in FII_IMG_FEATURE_LOC_SCALE .* H
//      PUSH feature_list, IMAGE(x,y)
const std::vector<float> FII_IMG_FEATURE_LOC_SCALE {0.1, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.9};

void fii_depth_first_search(const std::unordered_map<uint32_t, std::set<uint32_t> > &match_graph,
                            std::unordered_map<uint32_t, uint8_t> &vertex_flag,
                            uint32_t vertex,
                            std::set<uint32_t> &visited_nodes);

void fii_compute_img_feature(const std::string filename,
                             const uint64_t feature_start_index,
                             const uint64_t feature_count,
                             std::vector<uint8_t> &features,
                             const bool check_all_pixels=false) {
  // feature_end_index = feature_start_index + feature_count
  // fill in features[feature_start_index : feature_end_index]
  int width, height, nchannel;
  // @todo: we can improve speed here by only loading the
  // location of sparse set of pixel locations
  // e.g. features = stbi_load_sparse(...)
  unsigned char *img_data = stbi_load(filename.c_str(), &width, &height, &nchannel, 0);
  if(!img_data) {
    // malformed image, discard
    return;
  }

  if(check_all_pixels) {
    uint32_t npixel = width * height * nchannel;
    for(uint32_t feature_index=0; feature_index<npixel; ++feature_index) {
      features.at(feature_start_index + feature_index) = img_data[feature_index];
    }
  } else {
    std::vector<uint32_t> xp, yp;
    xp.reserve(FII_IMG_FEATURE_LOC_SCALE.size());
    yp.reserve(FII_IMG_FEATURE_LOC_SCALE.size());
    for(uint32_t i=0; i<FII_IMG_FEATURE_LOC_SCALE.size(); ++i) {
      xp.push_back( uint32_t(width  * FII_IMG_FEATURE_LOC_SCALE.at(i)) );
      yp.push_back( uint32_t(height * FII_IMG_FEATURE_LOC_SCALE.at(i)) );
    }

    uint64_t feature_index = 0;
    for(uint32_t xi=0; xi<xp.size(); ++xi) {
      for(uint32_t yi=0; yi<yp.size(); ++yi) {
        features.at(feature_start_index + feature_index) = img_data[ yp[yi]*width*nchannel + xp[xi]*nchannel ];
        feature_index++;
      }
    }
  }
  stbi_image_free(img_data);
}

void fii_find_identical_img(const std::vector<std::string> &filename_list1,
                            const std::vector<uint32_t> &filename_index_list1,
                            const std::string filename_prefix1,
                            const std::vector<std::string> &filename_list2,
                            const std::vector<uint32_t> &filename_index_list2,
                            const std::string filename_prefix2,
                            const std::vector<uint32_t> &img_dim,
                            const std::unordered_map<std::string, std::string> &options,
                            std::vector<std::set<uint32_t> > &image_groups) {
  image_groups.clear();

  uint32_t img_count1 = filename_index_list1.size();
  uint32_t img_count2 = filename_index_list2.size();
  if(img_count2 == 0 || img_count1 == 0) {
    return; // identical images not possible
  }

  uint64_t feature_count1 = img_count1;
  uint64_t feature_count2 = img_count2;
  uint32_t img_feature_count = 1;
  bool check_all_pixels = false;
  if(options.count("check-all-pixels")) {
    // run exhaustive comparison of every pixel
    // this is not necessary most of the time
    check_all_pixels = true;
    for(std::size_t i=0; i<img_dim.size(); ++i) {
      feature_count1 = feature_count1 * img_dim[i];
      feature_count2 = feature_count2 * img_dim[i];
      img_feature_count = img_feature_count * img_dim[i];
    }
  } else {
    const uint32_t FII_IMG_FEATURE_LOC_SCALE_COUNT = FII_IMG_FEATURE_LOC_SCALE.size();
    img_feature_count = FII_IMG_FEATURE_LOC_SCALE_COUNT * FII_IMG_FEATURE_LOC_SCALE_COUNT;
    feature_count1 = feature_count1 * img_feature_count;
    feature_count2 = feature_count2 * img_feature_count;
  }

  // use all available threads by default
  int nthread = omp_get_max_threads();
  if(options.count("nthread")) {
    nthread = std::atoi(options.at("nthread").c_str());
  }
  omp_set_dynamic(0);
  omp_set_num_threads(nthread);

  // extract features from filename_index_list1
  std::vector<uint8_t> features1(feature_count1);
#pragma omp parallel for
  for(uint32_t i=0; i<img_count1; ++i) {
    uint32_t filename_index1 = filename_index_list1.at(i);
    std::string file_path1 = filename_prefix1 + filename_list1.at(filename_index1);

    uint64_t img_feature_start_index = i * img_feature_count;
    fii_compute_img_feature(file_path1,
                            img_feature_start_index,
                            img_feature_count,
                            features1,
                            check_all_pixels);
  }

  // extract features from filename_index_list1
  std::vector<uint8_t> features2(feature_count2);
#pragma omp parallel for
  for(uint32_t i=0; i<img_count2; ++i) {
    uint32_t filename_index2 = filename_index_list2.at(i);
    std::string file_path2 = filename_prefix2 + filename_list2.at(filename_index2);

    uint64_t img_feature_start_index = i * img_feature_count;
    fii_compute_img_feature(file_path2,
                            img_feature_start_index,
                            img_feature_count,
                            features2,
                            check_all_pixels);
  }

  // compute image graph between each image
  std::unordered_map<uint32_t, std::set<uint32_t> > match_graph;
  std::unordered_map<uint32_t, uint8_t> vertex_flag;
  uint32_t match_findex_offset = filename_list1.size();
  for(uint32_t qi=0; qi<img_count1; ++qi) {
    for(uint32_t mj=0; mj<img_count2; ++mj) {
      // check if the distance between query qi and match mj is 0
      bool distance_is_zero = true;
      for(uint32_t fi=0; fi<img_feature_count; ++fi) {
        if( (features1[qi*img_feature_count + fi] ^ features2[mj*img_feature_count + fi]) != 0 ) {
          distance_is_zero = false;
          break;
        }
      }
      if(distance_is_zero) {
        uint32_t qindex = filename_index_list1.at(qi);
        uint32_t mindex = match_findex_offset + filename_index_list2.at(mj);
        // insert undirected edge between query and match
        if(match_graph.find(qindex) == match_graph.end()) {
          match_graph[qindex] = std::set<uint32_t>();
        }
        if(match_graph.find(mindex) == match_graph.end()) {
          match_graph[mindex] = std::set<uint32_t>();
        }
        match_graph[qindex].insert(mindex);
        match_graph[mindex].insert(qindex);
        // maintain vertex list
        if(vertex_flag.count(qindex) == 0) {
          vertex_flag[qindex] = 0;
        }
        if(vertex_flag.count(mindex) == 0) {
          vertex_flag[mindex] = 0;
        }
      }
    }
  }

  // perform depth first search to find all the connected components
  std::unordered_map<uint32_t, std::set<uint32_t> >::const_iterator itr;
  for(itr=match_graph.begin(); itr!=match_graph.end(); ++itr) {
    uint32_t query_id = itr->first;

    if(vertex_flag[query_id] == 1) {
      continue; // discard already visited nodes
    }
    std::set<uint32_t> match_id_list(itr->second);
    std::set<uint32_t> visited_nodes;
    std::set<uint32_t>::const_iterator mitr = match_id_list.begin();
    for(; mitr != match_id_list.end(); ++mitr) {
      uint32_t match_id = *mitr;
      if(vertex_flag[match_id] == 1) {
        continue; // discard already visited nodes
      }
      vertex_flag[match_id] = 1;
      visited_nodes.insert(match_id);
      fii_depth_first_search(match_graph, vertex_flag, match_id, visited_nodes);
    }
    if(visited_nodes.size() == 0) {
      continue; // discard empty components
    }
    //visited_nodes.push_back(query_id); // add query node
    image_groups.push_back(visited_nodes);
  }
}

void fii_find_identical_img(const std::vector<std::string> &filename_list,
                            const std::vector<uint32_t> &filename_index_list,
                            const std::string filename_prefix,
                            const std::vector<uint32_t> &img_dim,
                            const std::unordered_map<std::string, std::string> &options,
                            std::vector<std::set<uint32_t> > &image_groups) {
  image_groups.clear();

  uint32_t img_count = filename_index_list.size();
  if(img_count < 2) {
    return; // identical images not possible
  }

  uint32_t img_feature_count = 1;
  uint64_t feature_count = img_count;
  bool check_all_pixels = false;
  if(options.count("check-all-pixels")) {
    check_all_pixels = true;
    for(std::size_t i=0; i<img_dim.size(); ++i) {
      feature_count = feature_count * img_dim[i];
      img_feature_count = img_feature_count * img_dim[i];
    }
  } else {
    const uint32_t FII_IMG_FEATURE_LOC_SCALE_COUNT = FII_IMG_FEATURE_LOC_SCALE.size();
    img_feature_count = FII_IMG_FEATURE_LOC_SCALE_COUNT * FII_IMG_FEATURE_LOC_SCALE_COUNT;
    feature_count = img_count * img_feature_count;
  }

  std::vector<uint8_t> features(feature_count);

  // use all available threads by default
  int nthread = omp_get_max_threads();
  if(options.count("nthread")) {
    nthread = std::atoi(options.at("nthread").c_str());
  }
  omp_set_dynamic(0);
  omp_set_num_threads(nthread);

#pragma omp parallel for
  for(uint32_t i=0; i<img_count; ++i) {
    uint32_t filename_index = filename_index_list.at(i);
    std::string file_path = filename_prefix + filename_list.at(filename_index);

    uint64_t img_feature_start_index = i * img_feature_count;

    fii_compute_img_feature(file_path,
                            img_feature_start_index,
                            img_feature_count,
                            features,
                            check_all_pixels);
  }

  // compute image graph between each image
  std::unordered_map<uint32_t, std::set<uint32_t> > match_graph;
  std::unordered_map<uint32_t, uint8_t> vertex_flag;
  for(uint32_t qi=0; qi<img_count; ++qi) {
    for(uint32_t mj=qi+1; mj<img_count; ++mj) {
      // check if the distance between query qi and match mj is 0
      bool distance_is_zero = true;
      for(uint32_t fi=0; fi<img_feature_count; ++fi) {
        if( (features[qi*img_feature_count + fi] ^ features[mj*img_feature_count + fi]) != 0 ) {
          distance_is_zero = false;
          break;
        }
      }
      if(distance_is_zero) {
        uint32_t qindex = filename_index_list.at(qi);
        uint32_t mindex = filename_index_list.at(mj);
        // insert undirected edge between query and match
        if(match_graph.find(qindex) == match_graph.end()) {
          match_graph[qindex] = std::set<uint32_t>();
        }
        if(match_graph.find(mindex) == match_graph.end()) {
          match_graph[mindex] = std::set<uint32_t>();
        }
        match_graph[qindex].insert(mindex);
        match_graph[mindex].insert(qindex);
        // maintain vertex list
        if(vertex_flag.count(qindex) == 0) {
          vertex_flag[qindex] = 0;
        }
        if(vertex_flag.count(mindex) == 0) {
          vertex_flag[mindex] = 0;
        }
      }
    }
  }

  // perform depth first search to find all the connected components
  std::unordered_map<uint32_t, std::set<uint32_t> >::const_iterator itr;
  for(itr=match_graph.begin(); itr!=match_graph.end(); ++itr) {
    uint32_t query_id = itr->first;

    if(vertex_flag[query_id] == 1) {
      continue; // discard already visited nodes
    }
    std::set<uint32_t> match_id_list(itr->second);
    std::set<uint32_t> visited_nodes;
    std::set<uint32_t>::const_iterator mitr = match_id_list.begin();
    for(; mitr != match_id_list.end(); ++mitr) {
      uint32_t match_id = *mitr;
      if(vertex_flag[match_id] == 1) {
        continue; // discard already visited nodes
      }
      vertex_flag[match_id] = 1;
      visited_nodes.insert(match_id);
      fii_depth_first_search(match_graph, vertex_flag, match_id, visited_nodes);
    }
    if(visited_nodes.size() == 0) {
      continue; // discard empty components
    }
    //visited_nodes.push_back(query_id); // add query node
    image_groups.push_back(visited_nodes);
  }
}


void fii_depth_first_search(const std::unordered_map<uint32_t, std::set<uint32_t> > &match_graph,
                            std::unordered_map<uint32_t, uint8_t> &vertex_flag,
                            uint32_t vertex,
                            std::set<uint32_t> &visited_nodes) {
  if(match_graph.count(vertex) == 0) {
    return; // this vertex is not connected to any other nodes
  }

  std::set<uint32_t> match_file_id_list( match_graph.at(vertex) );
  std::set<uint32_t>::const_iterator mitr = match_file_id_list.begin();
  for(; mitr != match_file_id_list.end(); ++mitr) {
    uint32_t visited_vertex = *mitr;
    if(vertex_flag[visited_vertex] == 1) {
      continue; // discard visited vertices
    }
    vertex_flag[visited_vertex] = 1;
    visited_nodes.insert(visited_vertex);
    fii_depth_first_search(match_graph, vertex_flag, visited_vertex, visited_nodes);
  }
}

std::string fii_img_dim_id(const int &width,
                           const int &height,
                           const int &nchannel) {
  std::ostringstream ss;
  ss << width << "x" << height << "x" << nchannel;
  return ss.str();
}

bool fii_compare_bucket_by_value(std::pair<std::string, uint32_t>& a,
                                 std::pair<std::string, uint32_t>& b ) {
  return a.second > b.second;
}

void fii_group_by_img_dimension(const std::string check_dir,
                                std::vector<std::string> &filename_list,
                                std::unordered_map<std::string, std::vector<uint32_t> > &buckets_of_img_index,
                                std::unordered_map<std::string, std::vector<uint32_t> > &bucket_dim_list,
                                std::vector<std::string> &sorted_bucket_id_list,
                                bool verbose=true) {
  uint32_t t0, t1; // for recording elapsed time
  if(verbose) {
    std::cout << "Processing " << check_dir << std::endl;
  }
  t0 = fii::getmillisecs();
  uint32_t discarded_file_count;
  std::cout << "  collecting filenames : " << std::flush;
  fii::fs_list_img_files(check_dir, filename_list, discarded_file_count);
  t1 = fii::getmillisecs();
  if(verbose) {
    std::cout << "found " << filename_list.size()
              << " images";
    if(discarded_file_count) {
      std::cout << ", discarded " << discarded_file_count
                << " non-image files";
    }
    std::cout << " (" << (((double)(t1 - t0)) / 1000.0) << "s)"
              << std::endl;
  }

  int width, height, nchannel;
  uint32_t tstart = fii::getmillisecs();
  if(verbose) {
    std::cout << "  grouping images : " << std::flush;
  }
  t0 = fii::getmillisecs();

  std::vector<int> filename_width_list(filename_list.size());
  std::vector<int> filename_height_list(filename_list.size());
  std::vector<int> filename_nchannel_list(filename_list.size());

#pragma omp parallel
  {
    int nt = omp_get_num_threads();
    int rank = omp_get_thread_num();

    // this thread is taking care of files from index fi0 to fi1
    uint32_t fi0 = (filename_list.size() * rank) / nt;
    uint32_t fi1 = (filename_list.size() * (rank + 1)) / nt;
    for(uint32_t i=fi0; i<fi1; ++i) {
      std::string file_path = check_dir + "/" + filename_list[i];
      fii_image_size(file_path.c_str(),
                     &filename_width_list[i],
                     &filename_height_list[i],
                     &filename_nchannel_list[i]);
    }
  } // end of omp parallel

  buckets_of_img_index.clear();
  std::unordered_map<std::string, uint32_t> buckets_img_count;
  for(uint32_t i=0; i<filename_list.size(); ++i) {
    std::string img_dim_id = fii_img_dim_id(filename_width_list[i],
                                            filename_height_list[i],
                                            filename_nchannel_list[i]);
    if(bucket_dim_list.count(img_dim_id) == 0) {
      bucket_dim_list[img_dim_id] = std::vector<uint32_t>(3);
      bucket_dim_list[img_dim_id][0] = filename_width_list[i];
      bucket_dim_list[img_dim_id][1] = filename_height_list[i];
      bucket_dim_list[img_dim_id][2] = filename_nchannel_list[i];
    }
    buckets_of_img_index[img_dim_id].push_back(i);
    buckets_img_count[img_dim_id] += 1;
  }
  t1 = fii::getmillisecs();
  if(verbose) {
    std::cout << "found " << buckets_img_count.size() << " unique image dimensions."
              << " (" << (((double)(t1 - t0)) / 1000.0) << "s)"
              << std::endl;

    // show a histogram of image dimensions
    std::vector<std::pair<std::string, uint32_t> > sorted_img_dim_list;
    std::unordered_map<std::string, uint32_t>::const_iterator it;
    for(it=buckets_img_count.begin(); it!=buckets_img_count.end(); ++it) {
      if(it->second > 1) { // discard groups with only 1 image
        sorted_img_dim_list.push_back(*it);
      }
    }
    std::sort(sorted_img_dim_list.begin(),
              sorted_img_dim_list.end(),
              fii_compare_bucket_by_value);

    // bucket_id_list is sorted in ascending order
    std::vector<std::pair<std::string, uint32_t> >::reverse_iterator rit2;
    for(rit2=sorted_img_dim_list.rbegin(); rit2!=sorted_img_dim_list.rend(); ++rit2) {
      sorted_bucket_id_list.push_back(rit2->first);
    }

    // bucket_img_count histogram is sorted in descending order
    if(sorted_img_dim_list.size()) {
      std::ostringstream line1, line2, head;
      head  << "  +-------------+";
      line1 << "  | Image Dim.  |";
      line2 << "  | Image Count |";
      std::vector<std::pair<std::string, uint32_t> >::const_iterator it2;
      for(it2=sorted_img_dim_list.begin(); it2!=sorted_img_dim_list.end(); ++it2) {
	if(it2!=sorted_img_dim_list.begin()) {
	  head  << "------------+";
	  line1 << " |";
	  line2 << " |";
	}
	line1 << std::setw(11) << it2->first;
	line2 << std::setw(11) << it2->second;
	if(line1.str().size() > 50) {
	  head  << "------------+";
	  line1 << " |" << std::setw(11) << "...";
	  line2 << " |" << std::setw(11) << "...";
	  break;
	}
      }
      head  << "------------+" << std::endl;
      line1 << " |" << std::endl;
      line2 << " |" << std::endl;
      std::cout << head.str() << line1.str() << head.str() << line2.str() << head.str();
    }
  }
}

void fii_save_img_dimension_histogram(std::unordered_map<std::string, std::vector<uint32_t> > &buckets_of_img_index,
                                      std::vector<std::string> &bucket_id_list,
                                      std::string hist_fn) {
  std::ofstream hist(hist_fn);
  hist << "image_dimension,image_count" << std::endl;
  for(std::size_t i=0; i<bucket_id_list.size(); ++i) {
    std::string bucket_id = bucket_id_list[i];
    hist << bucket_id << "," << buckets_of_img_index[bucket_id].size() << std::endl;
  }
  hist.close();
}

#endif
