#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <math.h>
#include "anonymizer.h"
#include "shift_geo.h"

#define ENDL std::endl;

const int FORMATS[6] = {4, 4, 2, 4, 4, 1};              // data sample format
int MAIN_COORD[6] = {72, 76, 80, 84, 180, 184};         // position of coordinates
                                                        // in trace header

const int MAIN_COORD_LENGTH = 4;                        // format of coordinates
int ADD_COORD[6] = {96, 104, 112, 120, 160, 168};       // position of coordinates
                                                        // in trace header extension

const int ADD_COORD_LENGTH = 8;                         // format of coordinates

const int MAX_RANGE = 2147483647;

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

char* readFileBytes(std::string name) {
/**
    Read binary file to char array.

    @param name
    @return bytes
*/
    std::ifstream fl(name);
    fl.seekg(0, std::ios::end);
    size_t len = fl.tellg();
    char* bytes = new char[len];
    fl.seekg(0, std::ios::beg);
    fl.read(bytes, len);
    fl.close();
    return bytes;
}

void writeBytes(char* bytes, int length, std::string name) {
/**
    Write *char array into binary file.

    @param bytes Array to write
    @param length
    @param name
*/
    std::ofstream fs(name, std::ios::out | std::ios::binary | std::ios::trunc);
    fs.seekp(0, std::ios::beg);
    fs.write(bytes, length);
    fs.close();
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
    for (int i=0; i<length; i++) {
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
    for (int i=0; i<length; i++) {
        bytes[start+i] = block[i];
    }
}

void clearHeader(char* bytes, int start) {
/**
    Fill the first two lines of header by '*' in EBCDIC. bytes changes in-place.

    @param bytes
    @param start
*/
    for (int line=0; line < 3; line++) {
        for (int symb=3; symb<80; symb++) {
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
    for (int i=0; i<length-1; i++) {
        a = a | (unsigned char)(bytes[start+(length-1-i)]) << (8 * i);
    }
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

    degrees = (int) coord;
    coord = (coord - degrees) * 60;

    minutes = (int) coord;
    coord = (coord - minutes) * 60;

    seconds = (int) coord;
    coord = (coord - seconds) * 100;

    subseconds = (int) coord;

    dmsCoord = degrees * 10000 + minutes * 100 + seconds;
    return dmsCoord;
}

std::vector<int> transformCoord(int coordX, int coordY, double distance, double azimut,
                           int format, int order, int measSystem) {
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
    double dOrder = (double) order;
    if (dOrder < 0) {
        dOrder = 1. / (-dOrder);
    }
    switch (format) {
        case 1: { // meters or feet 
            double factor = 1.;
            if (measSystem == 2)
            {
                factor = 0.305;
            }
            double shiftX = distance * cos(azimut * PI / 180);
            double shiftY = distance * sin(azimut * PI / 180);

            long long _coordX = (long long) coordX * factor + (long long) (shiftX * 1000 / dOrder);
            long long _coordY = (long long) coordY * factor + (long long) (shiftY * 1000 / dOrder);

            if ((_coordX > MAX_RANGE) || (_coordX < -MAX_RANGE))
            {
                throw std::invalid_argument("Coordinate is too large"); 
            }
            result[0] = (int) (_coordX / factor);
            result[1] = (int) (_coordY / factor);
            break;
        }
        case 2: { // arcseconds
            double longitude = (double)(coordX * dOrder / 3600);
            double latitude = (double)(coordY * dOrder / 3600);

            std::vector<double> cartesian_res = shift_geo_coordinates(latitude, longitude, distance, azimut);
            std::vector<double> geo_res = cartesian2geo(cartesian_res);

            result[0] = (int) (geo_res[1] * 3600 / dOrder);
            result[1] = (int) (geo_res[0] * 3600 / dOrder);

            break;
        }
        case 3: {// decimal degrees
            double latitude = (double)coordY * dOrder;
            double longitude = (double)coordX * dOrder;

            std::vector<double> cartesian_res = shift_geo_coordinates(latitude, longitude, distance, azimut);
            std::vector<double> geo_res = cartesian2geo(cartesian_res);

            result[0] = (int) (geo_res[1] / dOrder);
            result[1] = (int) (geo_res[0] / dOrder);
            break;
        }
        case 4: { // DMS
            double latitude = parseDMS(coordY, order);
            double longitude = parseDMS(coordX, order);

            std::vector<double> cartesian_res = shift_geo_coordinates(latitude, longitude, distance, azimut);
            std::vector<double> geo_res = cartesian2geo(cartesian_res);

            result[0] = getDMS(geo_res[1], order);
            result[1] = getDMS(geo_res[0], order);
            break;
        }
    }
    return result;
}

char* intToBytes(int a, int length) {
/**
    Convert integer into sequence of bytes.

    @param a
    @param length
    @return bytes
*/
    char* bytes = new char[length];
    for (int i=0; i<length; i++) {
        bytes[length-1-i] = a >> (i * 8);
    }
    return bytes;
}

int anonymize(std::string filename, double distance, double azimut, std::ofstream& logfile) {
/**
    Anonymize SEG-Y file. Remove confident information from text headers and shif coordinates.

    @param filename
    @param distance       Distance in km to move coordinates.
    @param azimut         Azimut of shifting.
    @param logfile
    @return               Exit code
*/
    char* bytes = readFileBytes(filename);

    // anonymize text line header
    char *textLineHeader = getBlock(bytes, 0, 3200);
    clearHeader(textLineHeader, 0);
    putBlock(bytes, textLineHeader, 0, 3200);

    // get information from binary line header
    int numberTraces = bytesToInt(bytes, 3212, 2);
    int traceLength = bytesToInt(bytes, 3220, 2);
    int bytesPerRecord = FORMATS[bytesToInt(bytes, 3224, 2) - 1];
    int measSystem = bytesToInt(bytes, 3254, 2); // meters or feet
    unsigned char majorRevision = bytesToInt(bytes, 3500, 1);
    unsigned char minorRevision = bytesToInt(bytes, 3501, 1);
    int fixedTraces = bytesToInt(bytes, 3502, 2); // do all traces has the same length or not
    int numberExtendedHeaders = bytesToInt(bytes, 3504, 2);
    int maxTraceHeaders = bytesToInt(bytes, 3506, 2);

    int file_length = fileLength(filename);

    logfile << "Format Revision Number: " << (int)majorRevision << '.' << (int)minorRevision << ENDL;
    logfile << "Additional Trace Headers: " << maxTraceHeaders << ENDL;
    logfile << "Fixed length: " << fixedTraces << ENDL;
    logfile << "Number of traces: " << numberTraces << ENDL;
    logfile << "Trace length: " << traceLength << ENDL;
    logfile << "Extended Headers: " << numberExtendedHeaders << ENDL;

    // read extended text headers
    for (int extHeader=0; extHeader<numberExtendedHeaders; extHeader++) {
        char *textLineHeader = getBlock(bytes, 3600+extHeader*3200, 3200);
        clearHeader(textLineHeader, 0);
        putBlock(bytes, textLineHeader, 3600+extHeader*3200, 3200);
    }
    
    int actualNumber = (file_length - 3600) / (240 + traceLength*bytesPerRecord);
    
    if ((file_length - 3600) % (240 + traceLength*bytesPerRecord) != 0) {
        logfile << "Error: incorrect file length" << ENDL;
        return -1;
    }

    if (actualNumber != numberTraces) {
        logfile << "Warning: the number of traces in header (" << numberTraces << ") is not equal to the actual number of traces in file (" << actualNumber << ")" << ENDL;
        numberTraces = actualNumber;
    }

    int shift = 3600;

    // anonymize binary trace headers
    for (int i=0; i < numberTraces; i++) {
        if (maxTraceHeaders > 0) {
            maxTraceHeaders = bytesToInt(bytes, shift+240+156, 2);
        }

        int numberHeaders = 1 + maxTraceHeaders;

        if (!fixedTraces) {
            traceLength = bytesToInt(bytes, shift+114, 2);
        }

        int order = bytesToInt(bytes, shift+70, 2) - (1 << 16); // coordinates factor
        int format = bytesToInt(bytes, shift+88, 2); //meters or feet

        for (int nHeader=0; nHeader<numberHeaders; nHeader++) {
            int *coord;
            int size;

            if (nHeader == 0) {
                coord = MAIN_COORD;
                size = MAIN_COORD_LENGTH;
            }
            else {
                coord = ADD_COORD;
                size = ADD_COORD_LENGTH;
            }

            for (int j=0; j<6; j+=2) {
                int coordX = bytesToInt(bytes, shift+coord[j], size);
                int coordY = bytesToInt(bytes, shift+coord[j+1], size);

                std::vector<int> result{coordX, coordY};
                result = transformCoord(coordX, coordY, distance, azimut, format, order, measSystem);

                putBlock(bytes, intToBytes(result[0], size), shift+coord[j], size);
                putBlock(bytes, intToBytes(result[1], size), shift+coord[j+1], size);
            }
            shift += 240;
        }
        shift += traceLength*bytesPerRecord;
    }
    writeBytes(bytes, file_length, filename);
    return 0;
}
