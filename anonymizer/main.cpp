#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <Windows.h>
#include <math.h>
#include <fstream>
#include "app.h"

using namespace std;

vector<string> get_dir_paths(const char *path) {
   // get list of paths (entry points) from a specific path
   vector<string> result;
   struct dirent *entry;
   DIR *dir = opendir(path);

   if (dir == NULL) {
      return result;
   }

   // loop over entry points
   while ((entry = readdir(dir)) != NULL) {
        string fullpath;
        string s_path = (string)path;

        // correctly join paths
        char last_symb = s_path[s_path.length() - 1];
        if (!(last_symb == '/' || last_symb == '\\')) {
            fullpath = s_path + '\\' + string(entry->d_name);
        }
        else {
            fullpath = s_path  + string(entry->d_name);
        }

        result.push_back(fullpath);
   }
   closedir(dir);

   return result;
}

vector<string> filter_seg_paths(vector<string> paths) {
    // all accepted extensions
    vector<string> exts = {"segy", "seg", "sgy", "SGY", "SEGY", "SEG"};

    vector<string> result;

    bool cond = false;

    for(size_t  i = 0; i < paths.size(); i++) {
        size_t pos_ext = paths[i].find_last_of(".");
        string ext = paths[i].substr(pos_ext + 1);

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

int _if_modifiable(string path)
{
    // check that a file can be changed
    ofstream file;
    file.open(path, ios::out | ios::app | ios::binary);
    if(!file){
        file.close();
        return -1;
    }
    file.close();
    return 1;
}


int main(int argc, char **argv)
{
    // fetch args
    if (argc < 3)
    {
        cout << "2 arguments expected: folder name and distance in km" << endl;
        return -1;
    }
    char* dir = argv[1];
    double shift = atof(argv[2]);

    double distance = shift * sqrt(2);
    double azimut = 45.;

    ofstream logfile;
    logfile.open ((string)dir + "\\log.txt");

    // get list of files in the directory
    vector<string> all = get_dir_paths(dir);
    vector<string> filtered = filter_seg_paths(all);

    // check if files are modifiable
    vector<int> ixs;
    for (size_t  i = 0; i < filtered.size(); i++){
        int flag = _if_modifiable(filtered[i]);
        if (flag == -1){
            ixs.push_back(i);
        }
    }

    // if not all files can be modified, do not do anything
    if (ixs.size() > 0) {
        logfile << "The following files cannot be modified. Check its permissions: " << endl;
        for (size_t  i = 0; i < ixs.size(); i++){
            logfile << endl << "file " << ixs[i];
        }
        logfile << endl << "The program didn't do anything.";
        return -1;
    }

    // anonymize files
    logfile << "The following files are to be anonymized: " << endl;
    for (size_t i = 0; i < filtered.size(); i++){
        logfile << filtered[i] << endl;
    }

    for (size_t  i = 0; i < filtered.size(); i++){
        char* cstr = new char[filtered[i].length()];
        strcpy(cstr, filtered[i].c_str());
        logfile << "-------------------------------" << endl;
        logfile << "Filename: " << cstr << endl;
        anonymize(cstr, distance, azimut, logfile);
    }
    logfile.close();
    return 0;
}
