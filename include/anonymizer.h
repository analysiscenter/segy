// Copyright (c) 2018 Data Analysis Center

#ifndef SEGY_INCLUDE_ANONYMIZER_H_
#define SEGY_INCLUDE_ANONYMIZER_H_

#include <string>

int anonymize(std::string filename, double distance,
    double azimut, std::ofstream& logfile, int lines);
