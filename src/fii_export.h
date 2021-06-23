/*
  Export results in various file formats (e.g. html, csv, json, etc.)

  Author: Abhishek Dutta <adutta _at_ robots.ox.ac.uk>
  Date  : 17-Jun-2021
*/

#ifndef FII_EXPORT_H
#define FII_EXPORT_H

#include <set>
#include <vector>
#include <unordered_map>
#include <fstream>

const char *FII_EXPORT_HTML_JS_STR = R"TEXT(
var fii_toolbar = document.createElement('div');
fii_toolbar.setAttribute('class', 'fii_toolbar');
var fii_content = document.createElement('div');
fii_content.setAttribute('class', 'fii_content');

document.body.appendChild(fii_toolbar);
document.body.appendChild(fii_content);

if(typeof(_FII_DATA) !== 'undefined' &&
   _FII_DATA.hasOwnProperty('identical')) {
  fii_init_toolbar();
}

function fii_init_toolbar() {
  var set_selector = document.createElement('select');
  set_selector.setAttribute('id', 'set_selector');
  for(var img_dim in _FII_DATA['identical']) {
    var oi = document.createElement('option');
    oi.setAttribute('value', img_dim);
    var set_count = Object.keys(_FII_DATA['identical'][img_dim]).length;
    oi.innerHTML = set_count + ' sets of identical images exists in images with dimension = ' + img_dim;
    set_selector.appendChild(oi);
  }
  set_selector.addEventListener('change', fii_on_img_dim_select);
  set_selector.selectedIndex = 0;

  // export filenames in this set
  var download = document.createElement('button');
  download.setAttribute('onclick', 'fii_download_as_json()');
  download.setAttribute('type', 'button');
  download.innerHTML = 'Download JSON';

  fii_toolbar.appendChild(set_selector);
  fii_toolbar.appendChild(download);

  // select first element
  var event = new Event('change');
  set_selector.dispatchEvent(event);
}

function fii_on_img_dim_select(e) {
  var select = e.target;
  img_dim = select.options[select.selectedIndex].value;

  fii_content.innerHTML = '';
  for(var set_id in _FII_DATA['identical'][img_dim]) {
    var set_content = document.createElement('div');
    set_content.setAttribute('class', 'set');
    set_content.setAttribute('data-img_dim', img_dim);
    set_content.setAttribute('data-set_id', set_id);
    var set_id_container = document.createElement('div');
    set_id_container.setAttribute('class', 'set_id');
    set_id_container.innerHTML = set_id;
    set_content.appendChild(set_id_container);
    var set_size = _FII_DATA['identical'][img_dim][set_id].length;
    for(var i=0; i<set_size; ++i) {
      var img = document.createElement('img');
      var filename = _FII_DATA['identical'][img_dim][set_id][i];
      var filename_prefix_id = filename.split('/')[0] + '/';
      var filename_prefix = _FII_FILENAME_PREFIX_LIST[filename_prefix_id];
      var filename_abs_path = filename.replace(filename_prefix_id, filename_prefix);
      img.setAttribute('src', filename_abs_path);

      var figcaption = document.createElement('figcaption');
      figcaption.innerHTML = filename;

      var figure = document.createElement('figure');
      figure.appendChild(img);
      figure.appendChild(figcaption);
      set_content.appendChild(figure);
    }
    fii_content.appendChild(set_content);
  }
}

function fii_download_as_json() {
  var fii_data_blob = new Blob( [JSON.stringify(_FII_DATA)],
                                {type: 'text/json;charset=utf-8'});
  fii_save_data_to_local_file(fii_data_blob, 'identical.json');
}

function fii_save_data_to_local_file(data, filename) {
  var a      = document.createElement('a');
  a.href     = URL.createObjectURL(data);
  a.download = filename;

  // simulate a mouse click event
  var event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true
  });
  a.dispatchEvent(event);
}

)TEXT";

void fii_export_json_fstream(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
                             const std::vector<std::string> &filename_list1,
                             const std::string check_dir1,
                             const std::vector<std::string> &filename_list2,
                             const std::string check_dir2,
                             std::ofstream &json) {
  json << "{\"identical\":{";
  std::unordered_map<std::string, std::vector<std::set<uint32_t> > >::const_iterator bi;
  std::string dir1_name = fii::fs_dirname(check_dir1);
  std::string dir2_name = fii::fs_dirname(check_dir2);

  for(bi=image_groups.begin(); bi!=image_groups.end(); ++bi) {
    if(bi != image_groups.begin()) {
      json << ",";
    }
    json << "\"" << bi->first << "\":{";
    std::vector<std::set<uint32_t> >::const_iterator gi;
    for(std::size_t group_id=0; group_id!=bi->second.size(); ++group_id) {
      std::set<uint32_t> group_members(bi->second.at(group_id));
      std::set<uint32_t>::const_iterator si;
      if(group_id!=0) {
        json << ",";
      }
      json << "\"" << group_id << "\":[";
      for(si=group_members.begin(); si!=group_members.end(); ++si) {
        uint32_t findex = *si;
        std::string filename;
        std::string dir_name;
        if(findex >= filename_list1.size()) {
          // findex is from check_dir2
          findex = findex - filename_list1.size();
          filename = filename_list2.at(findex);
          dir_name = dir2_name;
        } else {
          // findex is from check_dir1
          filename = filename_list1.at(findex);
          dir_name = dir1_name;
        }

        if(si!= group_members.begin()) {
          json << ",\"" << dir_name << "/" << filename << "\"";
        } else {
          json << "\"" << dir_name << "/" << filename << "\"";
        }
      }
      json << "]";
    }
    json << "}";
  }
  json << "}}";
}

void fii_export_json(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
                     const std::string json_fn,
                     const std::vector<std::string> &filename_list1,
                     const std::string check_dir1,
                     const std::vector<std::string> &filename_list2=std::vector<std::string>(),
                     const std::string check_dir2="") {
  std::ofstream json(json_fn);
  fii_export_json_fstream(image_groups, filename_list1, check_dir1, filename_list2, check_dir2, json);
  json.close();
}

void fii_export_csv(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
                    const std::string csv_fn,
                    const std::vector<std::string> &filename_list1,
                    const std::string check_dir1,
                    const std::vector<std::string> &filename_list2=std::vector<std::string>(),
                    const std::string check_dir2="") {
  std::ofstream csv(csv_fn);
  std::string dir1_name = fii::fs_dirname(check_dir1);
  std::string dir2_name = fii::fs_dirname(check_dir2);
  std::unordered_map<std::string, std::vector<std::set<uint32_t> > >::const_iterator bi;
  for(bi=image_groups.begin(); bi!=image_groups.end(); ++bi) {
    uint32_t group_id = 0;
    std::vector<std::set<uint32_t> >::const_iterator gi;
    for(std::size_t group_id=0; group_id!=bi->second.size(); ++group_id) {
      std::set<uint32_t> group_members(bi->second.at(group_id));
      std::set<uint32_t>::const_iterator si;
      for(si=group_members.begin(); si!=group_members.end(); ++si) {
        uint32_t findex = *si;
        std::string filename;
        std::string dir_name;
        if(findex >= filename_list1.size()) {
          // findex is from check_dir2
          findex = findex - filename_list1.size();
          filename = filename_list2.at(findex);
          dir_name = dir2_name;
        } else {
          // findex is from check_dir1
          filename = filename_list1.at(findex);
          dir_name = dir1_name;
        }

        if(si!= group_members.begin()) {
          csv << ",\"" << dir_name << "/" << filename << "\"";
        } else {
          csv << "\"" << dir_name << "/" << filename << "\"";
        }
      }
      csv << std::endl;
    }
  }
  csv.close();
}

void fii_export_html(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
                     const std::string html_fn,
                     const std::vector<std::string> &filename_list1,
                     const std::string check_dir1,
                     const std::vector<std::string> &filename_list2=std::vector<std::string>(),
                     const std::string check_dir2="") {
  std::ofstream html(html_fn);
  html << "<!DOCTYPE html>\n"
       << "<html lang=\"en\">\n"
       << "<head>\n"
       << "<meta charset=\"UTF-8\">\n"
       << "<title>Identical Images</title>\n"
       << "<meta name=\"author\" content=\"Find Identical Images (FII) - https://www.robots.ox.ac.uk/~vgg/software/fii/\">\n"
       << "<style>.fii_toolbar button { margin:0 2em; } .set{ position:relative; display:inline-block; border:1px solid #cccccc; margin:2em 1em; padding:2em; font-size:small; } .set > .set_id{ position:absolute; top:0; left:0; padding:0.2em 0.5em; background-color:black; color:white; } .set figure{ display:inline-block; margin:0.2em 0.3em; } .set figure img{ max-width:300px; max-height:300px; }</style>\n";
  html << "</head>\n<body>";
  std::string dir1_name = fii::fs_dirname(check_dir1) + "/";
  std::string dir2_name = fii::fs_dirname(check_dir2) + "/";

  if(check_dir2 == "") {
    html << "<h1>Identical images in ["
         << dir1_name << "]</h1>\n";
  } else {
    html << "<h1>Identical images between ["
         << dir1_name << "] and ["
         << dir2_name << "]</h1>\n";
  }
  html << "<script>\n"
       << "var _FII_FILENAME_PREFIX_LIST = {\"" << dir1_name << "\":\"" << check_dir1 << "\""
       << ",\"" << dir2_name << "\":\"" << check_dir2 << "\"};\n"
       << "var _FII_DATA = ";
  fii_export_json_fstream(image_groups, filename_list1, check_dir1, filename_list2, check_dir2, html);
  html << ";\n";
  html << "\n"
       << FII_EXPORT_HTML_JS_STR
       << "\n</script>\n"
       << "</body>\n"
       << "</html>";

  html.close();
}

void fii_export_filelist(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
			 const std::string filelist_fn,
			 const std::vector<std::string> &filename_list1,
			 const std::string check_dir1,
			 const std::vector<std::string> &filename_list2=std::vector<std::string>(),
			 const std::string check_dir2="") {
  std::ofstream filelist(filelist_fn);
  std::string dir1_name = fii::fs_dirname(check_dir1);
  std::string dir2_name = fii::fs_dirname(check_dir2);
  std::unordered_map<std::string, std::vector<std::set<uint32_t> > >::const_iterator bi;
  for(bi=image_groups.begin(); bi!=image_groups.end(); ++bi) {
    std::vector<std::set<uint32_t> >::const_iterator gi;
    for(std::size_t group_id=0; group_id!=bi->second.size(); ++group_id) {
      std::set<uint32_t> group_members(bi->second.at(group_id));
      std::set<uint32_t>::const_iterator si;
      for(si=group_members.begin(); si!=group_members.end(); ++si) {
        uint32_t findex = *si;
        std::string filename;
        std::string dir_name;
        if(findex >= filename_list1.size()) {
          // findex is from check_dir2
          findex = findex - filename_list1.size();
          filename = filename_list2.at(findex);
          dir_name = dir2_name;
        } else {
          // findex is from check_dir1
          filename = filename_list1.at(findex);
          dir_name = dir1_name;
        }
	filelist << dir_name << "/" << filename << std::endl;
      }
    }
  }
  filelist.close();
}

void fii_export_delete_filelist(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
				const std::string filelist_fn,
				const std::vector<std::string> &filename_list1,
				const std::string check_dir1,
				const std::vector<std::string> &filename_list2=std::vector<std::string>(),
				const std::string check_dir2="") {
  std::ofstream filelist(filelist_fn);
  std::string dir1_name = fii::fs_dirname(check_dir1);
  std::string dir2_name = fii::fs_dirname(check_dir2);
  std::unordered_map<std::string, std::vector<std::set<uint32_t> > >::const_iterator bi;
  for(bi=image_groups.begin(); bi!=image_groups.end(); ++bi) {
    std::vector<std::set<uint32_t> >::const_iterator gi;
    for(std::size_t group_id=0; group_id!=bi->second.size(); ++group_id) {
      std::set<uint32_t> group_members(bi->second.at(group_id));
      std::set<uint32_t>::const_iterator si=group_members.begin();
      for(++si; si!=group_members.end(); ++si) {
        uint32_t findex = *si;
        std::string filename;
        std::string dir_name;
        if(findex >= filename_list1.size()) {
          // findex is from check_dir2
          findex = findex - filename_list1.size();
          filename = filename_list2.at(findex);
          dir_name = dir2_name;
        } else {
          // findex is from check_dir1
          filename = filename_list1.at(findex);
          dir_name = dir1_name;
        }
	filelist << dir_name << "/" << filename << std::endl;
      }
    }
  }
  filelist.close();
}

void fii_export_all(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
                    std::string export_dir,
                    const std::vector<std::string> &filename_list1,
                    std::string check_dir1,
                    const std::vector<std::string> &filename_list2=std::vector<std::string>(),
                    std::string check_dir2="") {
  try {
    std::string prefix = fii::fs_dirname(check_dir1);
    if(check_dir2 != "") {
      prefix += "-" + fii::fs_dirname(check_dir2);
    }
    std::string json_fn = export_dir + prefix + "-identical.json";
    fii_export_json(image_groups, json_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::string html_fn = export_dir + prefix + "-identical.html";
    fii_export_html(image_groups, html_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::string csv_fn = export_dir + prefix + "-identical.csv";
    fii_export_csv(image_groups, csv_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::string filelist_fn = export_dir + prefix + "-identical-filelist.txt";
    fii_export_filelist(image_groups, filelist_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::string delete_filelist_fn = export_dir + prefix + "-identical-delete-filelist.txt";
    fii_export_delete_filelist(image_groups, delete_filelist_fn, filename_list1, check_dir1, filename_list2, check_dir2);

    std::cout << "Results in " << export_dir << std::endl;
  } catch(std::exception &ex) {
    std::string json_fn = "identical.json";
    fii_export_json(image_groups, json_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::cerr << "Error while saving, dumping results to "
              << json_fn << std::endl;
  }
}

void fii_export(const std::unordered_map<std::string, std::vector<std::set<uint32_t> > > &image_groups,
                std::string export_fn,
                const std::vector<std::string> &filename_list1,
                std::string check_dir1,
                const std::vector<std::string> &filename_list2=std::vector<std::string>(),
                std::string check_dir2="") {
  std::string export_fn_ext = fii::fs_file_extension(export_fn);
  if(export_fn_ext == "csv" || export_fn_ext == "CSV") {
    fii_export_csv(image_groups, export_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::cout << "Results written to CSV file " << export_fn << std::endl;
  } else if(export_fn_ext == "json" || export_fn_ext == "JSON") {
    fii_export_json(image_groups, export_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::cout << "Results written to JSON file " << export_fn << std::endl;
  } else if(export_fn_ext == "html" || export_fn_ext == "HTML") {
    fii_export_html(image_groups, export_fn, filename_list1, check_dir1, filename_list2, check_dir2);
    std::cout << "Results written to HTML file " << export_fn << std::endl;
  } else {
    std::cout << "Unknown export file extension " << export_fn_ext
              << ", export to all available formats." << std::endl;
    fii_export_all(image_groups, "./", filename_list1, check_dir1, filename_list2, check_dir2);
  }
}

#endif
