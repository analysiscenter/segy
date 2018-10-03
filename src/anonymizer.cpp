// Copyright (c) 2018 Data Analysis Center

#include <math.h>
#include <limits.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include "include/anonymizer.h"
#include "include/shift_geo.h"

#define ENDL std::endl;

// data sample format
const int FORMATS[6] = {4, 4, 2, 4, 4, 1};
// position of coordinates in trace header
int MAIN_COORD[6] = {72, 76, 80, 84, 180, 184};

// format of coordinates
const int MAIN_COORD_LENGTH = 4;
// position of coordinates in trace header extension
int ADD_COORD[6] = {96, 104, 112, 120, 160, 168};
// format of coordinates
const int ADD_COORD_LENGTH = 8;

size_t fileLength(std::string name) {
/**
    Get length of the file.

    @param name
    @return length
*/
    std::ifstream fl(name);
    fl.seekg(0, std::ios::end);
    size_t length = fl.tellg();
    fl.close();
    return length;
}

char* readFileBytes(std::string name, long long start, int length) {
/**
    Read binary file to char array.

    @param name
    @return bytes
*/
    FILE *file;
    file = fopen(name.c_str(), "r+b");
    char* bytes = new char[length];
    fseeko64(file, start, SEEK_SET);
    fread(bytes, sizeof(char), length, file);
    fclose(file);
    return bytes;
}

void writeBytes(std::string name, long long start, int length, char* bytes) {
/**
    Write *char array into binary file.

    @param bytes Array to write
    @param length
    @param name
*/
    FILE *file;
    file = fopen(name.c_str(), "r+b");
    fseeko64(file, start, SEEK_SET);
    fwrite(bytes, sizeof(char), length, file);
    fclose(file);
}

char* getBlock(char* bytes, int start, int length) {
/**
    Get block of the elements from the array.

    @param bytes
    @param start
    @param length
    @return block
*/
    char* block = new char[length];
    for (int i=0; i < length; i++) {
        block[i] = bytes[start+i];
    }
    return block;
}


void putBlock(char* bytes, char* block, int start, int length) {
/**
    Put block of the elements to the array.

    @param bytes
    @param block
    @param start
    @param length
*/
    for (int i=0; i < length; i++) {
        bytes[start+i] = block[i];
    }
}

void clearHeader(char* bytes, int start, int* lines, int n_lines) {
/**
    Fill specific lines of header by '*' in EBCDIC. bytes changes in-place.

    @param bytes
    @param start
    @param lines The lines of text header to be starred
    @param n_lines The number of lines of the text header to be starred
*/
    for (int i=0; i < n_lines; i++) {
        int line = lines[i];
        for (int symb=3; symb < 80; symb++) {
            bytes[line*80+symb] = 92;
        }
    }
}

int bytesToInt(char* bytes, int start, int length) {
/**
    Convert sequence of bytes into int.

    @param bytes
    @param start
    @param length
    @return a      The resulting integer
*/
    int a = (unsigned char)(bytes[start]) << (8 * (length-1));
    for (int i=0; i < length-1; i++) {
        a = a | (unsigned char)(bytes[start+(length-1-i)]) << (8 * i);
    }
    a = a << ((4 - length) * 8);
    a = a / ((1 << length * 8));
    return a;
}

double parseDMS(int coord, int order) {
/**
    Convert integer coordinate in DMS format to decimal degrees.

    @param coord    Integer coordinate in DMS format
    @param order    Factor to multiply coord. If negative, coord must be divided by -order.
    @return         The resulting double
*/
    int degrees, minutes, seconds, subseconds;
    switch (order) {
        case 1:
            degrees = coord / 10000;
            coord = coord % 10000;

            minutes = coord / 100;
            seconds = coord % 100;

            subseconds = 0;
            break;
        case -100:
            degrees = coord / 1000000;
            coord = coord % 1000000;

            minutes = coord / 10000;
            coord = coord % 10000;

            seconds = coord / 100;
            subseconds = coord % 100;
            break;
        default:
            throw std::invalid_argument("Order for DMS must be 1 or -100");
            break;
    }
    return degrees + minutes / 60.0 + (seconds + subseconds / 100.0) / 3600.0;
}

int getDMS(double coord, int order) {
/**
    Convert decimal degrees to integer coordinate in DMS format.

    @param coord    Integer coordinate in DMS format
    @param order    Factor to multiply coord. If negative, coord must be divided by -order.
    @return         The resulting integer.
*/
    int dmsCoord;
    int degrees, minutes, seconds, subseconds;

    degrees = static_cast<int>(coord);
    coord = (coord - degrees) * 60;

    minutes = static_cast<int>(coord);
    coord = (coord - minutes) * 60;

    seconds = static_cast<int>(coord);
    coord = (coord - seconds) * 100;

    subseconds = static_cast<int>(coord);

    dmsCoord = degrees * 10000 + minutes * 100 + seconds;
    return dmsCoord;
}

std::vector<int> transformCoord(int coordX, int coordY, double distance,
    double azimut, int format, int order, int measSystem) {
/**
    Shift coordinates.

    @param coordX
    @param coordY
    @param distance       Distance in km to move coordinates.
    @param azimut         Azimut of shifting.
    @param format         Format of coordinates (meters, arcseconds, degrees, DMS)
    @param order          Factor to multiply coord. If negative, coord must be divided by -order.
    @param measSystem     Meters or feet.
    @return result        The resulting vector of X and Y coordinates.
*/
    std::vector<int> result = {0, 0};
    double dOrder = static_cast<double>(order);
    if (dOrder < 0) {
        dOrder = 1. / (-dOrder);
    }
    switch (format) {
        case 1: {  // meters or feet
            double factor = 1.;
            if (measSystem == 2) {
                factor = 0.305;
            }
            double shiftX = distance * cos(azimut * PI / 180);
            double shiftY = distance * sin(azimut * PI / 180);

            long long _coordX = static_cast<long long>(coordX) * factor +
                                static_cast<long long>(shiftX) * 1000 / dOrder;
            long long _coordY = static_cast<long long>(coordY) * factor +
                                static_cast<long long>(shiftY) * 1000 / dOrder;

            if ((_coordX > INT_MAX) || (_coordX < INT_MIN)) {
                throw std::out_of_range("Coordinate X after shifting is out of range");
            }
            if ((_coordY > INT_MAX) || (_coordY < INT_MIN)) {
                throw std::out_of_range("Coordinate Y after shifting is out of range");
            }
            result[0] = static_cast<int>(_coordX / factor);
            result[1] = static_cast<int>(_coordY / factor);
            break;
        }
        case 2: {  // arcseconds
            double longitude = static_cast<double>(coordX * dOrder / 3600);
            double latitude = static_cast<double>(coordY * dOrder / 3600);

            std::vector<double> cartesian_res =
                shift_geo_coordinates(latitude, longitude, distance, azimut);
            std::vector<double> geo_res = cartesian2geo(cartesian_res);

            result[0] = static_cast<int>(geo_res[1] * 3600 / dOrder);
            result[1] = static_cast<int>(geo_res[0] * 3600 / dOrder);

            break;
        }
        case 3: {  // decimal degrees
            double latitude = static_cast<double>(coordY * dOrder);
            double longitude = static_cast<double>(coordX * dOrder);

            std::vector<double> cartesian_res =
                shift_geo_coordinates(latitude, longitude, distance, azimut);
            std::vector<double> geo_res = cartesian2geo(cartesian_res);

            result[0] = static_cast<int>(geo_res[1] / dOrder);
            result[1] = static_cast<int>(geo_res[0] / dOrder);
            break;
        }
        case 4: {  // DMS
            double latitude = parseDMS(coordY, order);
            double longitude = parseDMS(coordX, order);

            std::vector<double> cartesian_res =
                shift_geo_coordinates(latitude, longitude, distance, azimut);
            std::vector<double> geo_res = cartesian2geo(cartesian_res);

            result[0] = getDMS(geo_res[1], order);
            result[1] = getDMS(geo_res[0], order);
            break;
        }
    }
    return result;
}

void printBytes(char* bytes, int length, std::ofstream& logfile) {
    for (int i=0; i < length; i++) {
        logfile << static_cast<int>(bytes[i]) << ' ';
    }
    logfile << ENDL;
}

char* intToBytes(int a, int length) {
/**
    Convert integer into sequence of bytes.

    @param a
    @param length
    @return bytes
*/
    char* bytes = new char[length];
    for (int i=0; i < length; i++) {
        bytes[length-1-i] = a >> (i * 8);
    }
    return bytes;
}

void transformCoords(char* bytes, int posX, int posY, int size, double distance, double azimut, int format, int order, int measSystem) {
    int coordX = bytesToInt(bytes, posX, size);
    int coordY = bytesToInt(bytes, posY, size);

    std::vector<int> result{coordX, coordY};
    result = transformCoord(coordX, coordY, distance, azimut, format, order, measSystem);

    putBlock(bytes, intToBytes(result[0], size), coord[j], size);
    putBlock(bytes, intToBytes(result[1], size), coord[j+1], size);	
}

int anonymize(std::string filename, double distance,
    double azimut, std::ofstream& logfile, int* lines, int n_lines, int group, int ensemble) {
/**
    Anonymize SEG-Y file. Remove confident information from text headers and shif coordinates.

    @param filename
    @param distance       Distance in km to move coordinates.
    @param azimut         Azimut of shifting.
    @param logfile        The file to put logging into
    @param lines          The lines of textual header to be starred
    @param n_lines        The number of lines from textual header to be starred
    @return               Exit code
*/
    char* bytes = readFileBytes(filename, 0, 3600);

    // anonymize text line header
    char *textLineHeader = getBlock(bytes, 0, 3200);
    clearHeader(textLineHeader, 0, lines, n_lines);
    putBlock(bytes, textLineHeader, 0, 3200);

    // binary header does not change, only text header
    writeBytes(filename, 0, 3600, bytes);

    // get information from binary line header
    int numberTraces = bytesToInt(bytes, 3212, 2);

    int traceLength = bytesToInt(bytes, 3220, 2);
    int bytesPerRecord = FORMATS[bytesToInt(bytes, 3224, 2) - 1];
    // meters or feet
    int measSystem = bytesToInt(bytes, 3254, 2);
    unsigned char majorRevision = bytesToInt(bytes, 3500, 1);
    unsigned char minorRevision = bytesToInt(bytes, 3501, 1);
    // do all traces has the same length or not
    int fixedTraces = bytesToInt(bytes, 3502, 2);
    int numberExtendedHeaders = bytesToInt(bytes, 3504, 2);
    int maxTraceHeaders = bytesToInt(bytes, 3506, 2);

    if (maxTraceHeaders > 0) {
        throw std::runtime_error("Extended Trace Headers are not supported.");
    }

    size_t file_length = fileLength(filename);

    logfile << "File length: " << file_length / 1024.0 / 1024.0 << " MB" << ENDL;
    logfile << "Format Revision Number: " << static_cast<int>(majorRevision)
            << '.' << static_cast<int>(minorRevision) << ENDL;
    logfile << "Additional Trace Headers: " << maxTraceHeaders << ENDL;
    logfile << "Fixed length: " << fixedTraces << ENDL;
    logfile << "Number of traces: " << numberTraces << ENDL;
    logfile << "Trace length: " << traceLength << ENDL;
    logfile << "Extended Headers: " << numberExtendedHeaders << ENDL;

    // read extended text headers
    for (int extHeader=0; extHeader < numberExtendedHeaders; extHeader++) {
        // bytes = readFileBytes(filename, 3600+extHeader*3200, 3200);
        // char *textLineHeader = getBlock(bytes, 0, 3200);
        // clearHeader(textLineHeader, 0, lines, n_lines);
        // putBlock(bytes, textLineHeader, 3600+extHeader*3200, 3200);
        // writeBytes(filename, 3600+extHeader*3200, 3200, bytes);
        throw std::runtime_error("Extended Binary Headers are not supported.");
    }

    long long actualNumber = (file_length - 3600) / (240 + traceLength*bytesPerRecord);

    if ((file_length - 3600) % (240 + traceLength*bytesPerRecord) != 0) {
        throw std::length_error("Incorrect file length.");
        return -1;
    }

    if (actualNumber != numberTraces) {
        logfile << "Warning (unstructured file): the number of traces in binary header ("<< numberTraces
                << ") is not equal to the actual number of traces in file ("
                << actualNumber << ")" << ENDL;
        numberTraces = actualNumber;
    }

    long long shift = 3600 + numberExtendedHeaders * 3200;

    // anonymize binary trace headers
    for (int i=0; i < numberTraces; i++) {
        logfile << "=============" << ENDL;
        int traceBlockLength = 240;
        if (maxTraceHeaders > 0) {
            traceBlockLength += 240;
        }
        bytes = readFileBytes(filename, shift, traceBlockLength);
        logfile << "Trace " << bytesToInt(bytes, 0, 4) << ", shift (bytes): " << shift << ENDL;
        int numberExtendedTraceHeaders = 0;
        if (maxTraceHeaders > 0) {
            numberExtendedTraceHeaders = bytesToInt(bytes, 240+156, 2);
        }

        logfile << "Additional headers: " << numberExtendedTraceHeaders << ENDL;
        int numberHeaders = 1 + numberExtendedTraceHeaders;

        if (!fixedTraces) {
            traceLength = bytesToInt(bytes, 114, 2);
        }

        logfile << "Length: " << traceLength << ENDL;

        int order = bytesToInt(bytes, 70, 2);  // coordinates factor
        if (order == 0) {
            order = 1;
        }
        int format = bytesToInt(bytes, 88, 2);  // meters or feet

        for (int nHeader=0; nHeader < numberHeaders; nHeader++) {
            bytes = readFileBytes(filename, shift, 240);

            int *coord;
            int size;

            if (nHeader == 0) {
                coord = MAIN_COORD;
                size = MAIN_COORD_LENGTH;
            } else {
                coord = ADD_COORD;
                size = ADD_COORD_LENGTH;
            }

			transformCoords(bytes, coord[0], coord[1], size, distance, azimut, format, order, measSystem)            
            if (group == 1) {
            	transformCoords(bytes, coord[2], coord[3], size, distance, azimut, format, order, measSystem)            
            }
            if (ensemble == 1) {
            	transformCoords(bytes, coord[4], coord[5], size, distance, azimut, format, order, measSystem)            
            }
            writeBytes(filename, shift, 240, bytes);
            shift += 240;
        }
        shift += traceLength*bytesPerRecord;
    }
    return 0;
}