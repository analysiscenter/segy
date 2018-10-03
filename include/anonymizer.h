// Copyright (c) 2018 Data Analysis Center

#ifndef INCLUDE_ANONYMIZER_H_
#define INCLUDE_ANONYMIZER_H_

#include <string>

int anonymize(std::string filename, double shift,
    std::ofstream& logfile, int* lines, int n_lines, int groups);

#endif  // INCLUDE_ANONYMIZER_H_
