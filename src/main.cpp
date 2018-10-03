// Copyright (c) 2018 Data Analysis Center

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>
#include <iostream>
#include <utility>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include "include/anonymizer.h"
#include "include/path_handler.h"

#define ENDL std::endl;

int main(int argc, char **argv) {
    // fetch args
    if (argc < 6) {
        std::cout << "At least 5 arguments expected: folder name, distance in km, azimut, flags for anonymization of groups and ensembles. " << ENDL;
        return -1;
    }
    std::string dir = (std::string)argv[1];
    int n_lines;
    int* lines;

    double distance = atof(argv[2]);
    double azimut = atof(argv[3]);
    int group = atoi(argv[4]);
    int ensemble = atof(argv[5]);
    if (argc == 6) {
        // no line-numbers are supplied; star them all
        n_lines = 40;
        lines = new int[n_lines];
        for (int j = 0; j < n_lines; j++) {
            lines[j] = j;
        }
    }
    else {
        // specific lines are to be starred in this case
        n_lines = argc - 6;
        lines = new int[n_lines];
        for (int j = 6; j < argc; j++) {
            lines[j - 6] = atoi(argv[j]);
        }
    }

    std::pair< std::vector<std::string>, std::vector<std::string> > groups = get_segy(dir);

    std::string logname = (std::string)dir + "\\log.txt";
    std::string errorsname = (std::string)dir + "\\errors.txt";
    freopen(errorsname.c_str(), "w", stderr);
    std::ofstream logfile;
    logfile.open(logname);

    try {
        // get modifiable and nonmodifiable segys

        // if not all files can be modified, do not do anything
        if (groups.second.size() > 0) {
            logfile << "The following files cannot be modified. Check its permissions: " << ENDL;
            for (size_t  i = 0; i < groups.second.size(); i++) {
                logfile << ENDL;
                logfile << "file " << groups.second[i];
            }
            logfile << ENDL;
            logfile << "The program didn't do anything.";
            return -1;
        }

        // anonymize files
        logfile << "The following files are to be anonymized: " << ENDL;
        for (size_t i = 0; i < groups.first.size(); i++) {
            logfile << groups.first[i] << ENDL;
        }

        for (size_t  i = 0; i < groups.first.size(); i++) {
            logfile << "-------------------------------" << ENDL;
            logfile << "Filename: " << groups.first[i] << ENDL;
            anonymize(groups.first[i], distance, azimut, logfile, lines, n_lines, group, ensemble);
            logfile << "Success!" << ENDL;
        }
    } catch (const std::exception &e) {
        logfile.close();
        std::cout << "There were errors, see errors log: " << errorsname << ENDL;
        throw;
    }
    logfile.close();
    return 0;
}
