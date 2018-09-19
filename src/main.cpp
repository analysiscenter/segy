#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <cstdio>
#include <math.h>
#include <Windows.h>
#include "include/anonymizer.h"
#include "include/path_handler.h"

#define ENDL std::endl;

int main(int argc, char **argv) {
    // fetch args
    if (argc < 3)
    {
        std::cout << "2 arguments expected: folder name and distance in km" << ENDL;
        return -1;
    }
    std::string dir = (std::string)argv[1];
    double shift = atof(argv[2]);

    double distance = shift * sqrt(2);
    double azimut = 45.;

    std::ofstream logfile;
    std::string logname = (std::string)dir + "\\log.txt";
    logfile.open (logname);
    freopen(logname.c_str(), "w", stderr);

    // get modifiable and nonmodifiable segys
    std::pair< std::vector<std::string>, std::vector<std::string> > groups = get_segy(dir);

    // if not all files can be modified, do not do anything
    if (groups.second.size() > 0) {
        logfile << "The following files cannot be modified. Check its permissions: " << ENDL;
        for (size_t  i = 0; i < groups.second.size(); i++){
            logfile << ENDL;
            logfile << "file " << groups.second[i];
        }
        logfile << ENDL;
        logfile << "The program didn't do anything.";
        return -1;
    }

    // anonymize files
    logfile << "The following files are to be anonymized: " << ENDL;
    for (size_t i = 0; i < groups.first.size(); i++){
        logfile << groups.first[i] << ENDL;
    }

    for (size_t  i = 0; i < groups.first.size(); i++){
        logfile << "-------------------------------" << ENDL;
        logfile << "Filename: " << groups.first[i] << ENDL;
        anonymize(groups.first[i], distance, azimut, logfile);
        logfile << "Success!" << ENDL;
    }
    logfile.close();
    return 0;
}
