#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <Windows.h>

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
        result.push_back(entry->d_name);
   }
   closedir(dir);

   return result;
}

vector<string> filter_seg_paths(vector<string> paths) {
    // all accepted extensions
    vector<string> exts = {"segy", "seg"};

    vector<string> result;

    bool cond = false;

    for(int i = 0; i < paths.size(); i++) {
        size_t pos_ext = paths[i].find_last_of(".");
        string ext = paths[i].substr(pos_ext + 1);

        // check if ext is indeed segy
        for(int j = 0; j < exts.size(); j++) {
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

int main(int argc, char **argv)
{
    // fetch args
    char* dir = argv[1];
    double deltaX = atof(argv[2]);
    double deltaY = atof(argv[3]);

    //cout << dir << " " << deltaX + deltaY;

    // get list of files in the directory
    vector<string> all = get_dir_paths(dir);
    vector<string> filtered = filter_seg_paths(all);

    for(int i = 0; i < all.size(); i++){
        cout << endl << all[i];
    }

    for(int i = 0; i < filtered.size(); i++){
        cout << endl << filtered[i];
    }

    DWORD length = 0;
    cout << endl << GetFileSecurityA("D:\Work\Cprogs\ForTests\f4.seg", FILE_GENERIC_WRITE, NULL, NULL, &length);

    return 0;
}
