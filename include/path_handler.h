// Copyright (c) 2018 Data Analysis Center

#ifndef SEGY_INCLUDE_PATH_HANDLER_H_
#define SEGY_INCLUDE_PATH_HANDLER_H_

#include <utility>
#include <string>
#include <vector>

std::vector<std::string> get_dir_paths(std::string path);
std::vector<std::string> filter_seg_paths(std::vector<std::string> paths);
int _if_modifiable(std::string path);
std::pair< std::vector<std::string>, std::vector<std::string> > get_segy(std::string path);
