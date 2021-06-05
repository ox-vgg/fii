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

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#endif

void fii_depth_first_search(const std::unordered_map<std::size_t, std::set<std::size_t> > &match_graph,
                            std::unordered_map<std::size_t, uint8_t> &vertex_flag,
                            std::size_t vertex,
                            std::set<std::size_t> &visited_nodes);

void fii_compute_img_feature(const std::string filename,
                             const std::size_t feature_start_index,
                             const std::size_t feature_count,
                             std::vector<uint8_t> &features) {
  // feature_end_index = feature_start_index + feature_count
  // fill in features[feature_start_index : feature_end_index]
  int width, height, nchannel;
  // @todo: there is no need to load the full image
  // we only need access to a small number of pixel locations
  unsigned char *img_data = stbi_load(filename.c_str(), &width, &height, &nchannel, 0);

  uint32_t pixel_count = width * height * nchannel;
  std::size_t cx = width / 2;
  std::size_t cy = height / 2;

  // @todo : compute features from strategically located pixel locations
  // level 0 : (cx,cy)
  // level 1 : (cx,cy*0.5) (cx,cy*0.75) (cx*0.5,cy) (cx*0.75,cy) (cx*0.5,cy*0.5) (cx*1.5,cy*0.5) (cx*0.5,cy*1.5) (cx*1.5,cy*1.5)
  // level 2 : ...
  std::size_t xp[] = {cx, cx    , cx,      cx*0.5, cx*0.75, cx*0.5, cx*1.5, cx*0.5, cx*1.5};
  std::size_t yp[] = {cy, cy*0.5, cy*0.75, cy    , cy     , cy*0.5, cy*0.5, cy*1.5, cy*1.5};

  for(std::size_t i=0; i<feature_count; ++i) {
    features.at(feature_start_index + i) = img_data[ yp[i]*width*nchannel + xp[i]*nchannel ];
  }
  stbi_image_free(img_data);
}

void fii_find_identical_img(const std::vector<std::string> &filename_list,
                            const std::vector<std::size_t> &filename_index_list,
                            const std::string filename_prefix,
                            std::unordered_map<std::size_t, std::set<std::size_t> > &image_groups) {
  image_groups.clear();
  std::size_t img_count = filename_index_list.size();

  if(img_count < 2) {
    return; // identical images not possible
  }

  const std::size_t img_feature_count = 9; // @todo: improve by using more features
  std::size_t feature_count = img_count * img_feature_count;

  std::vector<uint8_t> features(feature_count);
  int nthread = omp_get_max_threads();
  omp_set_num_threads(nthread);
  std::cout << "using " << nthread << " threads" << std::endl;

#pragma omp parallel for
  for(std::size_t i=0; i<img_count; ++i) {
    std::size_t filename_index = filename_index_list.at(i);
    std::string file_path = filename_prefix + "/" + filename_list.at(filename_index);

    std::size_t img_feature_start_index = i * img_feature_count;

    fii_compute_img_feature(file_path,
                            img_feature_start_index,
                            img_feature_count,
                            features);
  }

  // compute image graph between each image
  std::unordered_map<std::size_t, std::set<std::size_t> > match_graph;
  std::unordered_map<std::size_t, uint8_t> vertex_flag;
  for(std::size_t qi=0; qi<img_count; ++qi) {
    for(std::size_t mj=qi+1; mj<img_count; ++mj) {
      // check if the distance between query qi and match mj is 0
      bool distance_is_zero = true;
      for(std::size_t fi=0; fi<img_feature_count; ++fi) {
        if( (features[qi*img_feature_count + fi] ^ features[mj*img_feature_count + fi]) != 0 ) {
          distance_is_zero = false;
          break;
        }
      }
      if(distance_is_zero) {
        std::size_t qindex = filename_index_list.at(qi);
        std::size_t mindex = filename_index_list.at(mj);
        // insert undirected edge between query and match
        if(match_graph.find(qindex) == match_graph.end()) {
          match_graph[qindex] = std::set<std::size_t>();
        }
        if(match_graph.find(mindex) == match_graph.end()) {
          match_graph[mindex] = std::set<std::size_t>();
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
  std::unordered_map<std::size_t, std::set<std::size_t> >::const_iterator itr;
  std::size_t set_id = 0;
  for(itr=match_graph.begin(); itr!=match_graph.end(); ++itr) {
    std::size_t query_id = itr->first;

    if(vertex_flag[query_id] == 1) {
      continue; // discard already visited nodes
    }
    std::set<std::size_t> match_id_list(itr->second);
    std::set<std::size_t> visited_nodes;
    std::set<std::size_t>::const_iterator mitr = match_id_list.begin();
    for(; mitr != match_id_list.end(); ++mitr) {
      std::size_t match_id = *mitr;
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
    image_groups[set_id] = visited_nodes;

    // move to next set
    set_id = set_id + 1;
  }
}


void fii_depth_first_search(const std::unordered_map<std::size_t, std::set<std::size_t> > &match_graph,
                            std::unordered_map<std::size_t, uint8_t> &vertex_flag,
                            std::size_t vertex,
                            std::set<std::size_t> &visited_nodes) {
  if(match_graph.count(vertex) == 0) {
    return; // this vertex is not connected to any other nodes
  }

  std::set<std::size_t> match_file_id_list( match_graph.at(vertex) );
  std::set<std::size_t>::const_iterator mitr = match_file_id_list.begin();
  for(; mitr != match_file_id_list.end(); ++mitr) {
    std::size_t visited_vertex = *mitr;
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

#endif
