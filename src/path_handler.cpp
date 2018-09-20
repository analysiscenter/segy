#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include "include/path_handler.h"

std::vector<std::string> get_dir_paths(std::string path) {
/**
    Get list of full paths to all entry points from a directory

    @param path    directory
    @return vector
*/
   const char* path_char = path.c_str();
   std::vector<std::string> result;
   struct dirent *entry;
   DIR *dir = opendir(path_char);

   if (dir == NULL) {
      throw std::invalid_argument("Specified directory does not exist!");
      return result;
   }

   // loop over entry points
   while ((entry = readdir(dir)) != NULL) {
        std::string fullpath;
        std::string s_path = (std::string)path;

        // correctly join paths
        char last_symb = s_path[s_path.length() - 1];
        if (!(last_symb == '/' || last_symb == '\\')) {
            fullpath = s_path + '\\' + std::string(entry->d_name);
        }
        else {
            fullpath = s_path  + std::string(entry->d_name);
        }

        result.push_back(fullpath);
   }
   closedir(dir);

   return result;
}

std::vector<std::string> filter_segy_paths(std::vector<std::string> paths) {
/**
    Filter out vector of paths: leave only paths to files with segy-extension

    @param paths    vector of paths
    @return vector of paths to segy-files
*/
    std::vector<std::string> exts = {"segy", "seg", "sgy", "SGY", "SEGY", "SEG"};

    std::vector<std::string> result;

    bool cond = false;

    for(size_t  i = 0; i < paths.size(); i++) {
        size_t pos_ext = paths[i].find_last_of(".");
        std::string ext = paths[i].substr(pos_ext + 1);

        // check if ext is indeed segy
        for(size_t  j = 0; j < exts.size(); j++) {
            if (ext.compare(exts[j]) == 0){
                cond = true;
                break;
            }
        }
        if (cond)
            result.push_back(paths[i]);

        cond = false;
    }

    return result;
}

int _if_modifiable(std::string path){
/**
    Check if a file can be modified

    @param path    path to file
    @return 1 if modifiable, -1 otherwise
*/
    std::ofstream file;
    file.open(path, std::ios::out | std::ios::app | std::ios::binary);
    if(!file){
        file.close();
        return -1;
    }
    file.close();
    return 1;
}

std::pair< std::vector<std::string>, std::vector<std::string> > get_segy(std::string path)
{
/**
    Put modifiable and nonmodifiable paths to segy-files from a directory
    into different components of a pair

    @param path    directory
    @return resulting pair
*/
    // get list of files in the directory
    std::vector<std::string> all = get_dir_paths(path);
    std::vector<std::string> filtered = filter_segy_paths(all);

    // push modifiables, nonmodifiables into different components of the pair
    std::pair< std::vector<std::string>, std::vector<std::string> > result;
    for (size_t i = 0; i < filtered.size(); i++) {
        int flag = _if_modifiable(filtered[i]);
        if (flag == -1){
            result.second.push_back(filtered[i]);
        } else {
            result.first.push_back(filtered[i]);
        }
    }
    return result;
}
