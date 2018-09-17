#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <math.h>
#include "app.h"
#include "geo.h"

using namespace std;

const int FORMATS[6] = {4, 4, 2, 4, 4, 1};              // data sample format
int MAIN_COORD[6] = {72, 76, 80, 84, 180, 184};   // position of coordinates
                                                        // in trace header
const int MAIN_COORD_LENGTH = 4;                        // format of coordinates
int ADD_COORD[6] = {96, 104, 112, 120, 160, 168}; // position of coordinates
                                                        // in trace header extension
const int ADD_COORD_LENGTH = 8;                         // format of coordinates

const double PI = 3.141592653589793238463;
const int MAX_RANGE = 2147483647;

size_t fileLength(const char *name)
{
/**
    Get length of the file.

    @param name
    @return length
*/
    ifstream fl(name);
    fl.seekg( 0, ios::end );
    size_t len = fl.tellg();
    fl.close();
    return len;
}

char* readFileBytes(const char *name)
{
/**
    Read binary file to char array.

    @param name
    @return ret *char array
*/
    ifstream fl(name);
    fl.seekg( 0, ios::end );
    size_t len = fl.tellg();
    char *ret = new char[len];
    fl.seekg(0, ios::beg);
    fl.read(ret, len);
    fl.close();
    return ret;
}

void writeBytes(char* bytes, int length, const char *name)
{
/**
    Write *char array into binary file.

    @param bytes Array to write
    @param length
    @param name
*/
    ofstream fs(name, ios::out | ios::binary | ios::trunc);
    fs.seekp(0, ios::beg);
    fs.write(bytes, length);
    fs.close();
}

char* getBlock(char *bytes, int start, int length)
{
/**
    Get block of the elements from the array.

    @param bytes
    @param start
    @param length
    @return block
*/
    char *block = new char[length];
    for (int i=0; i<length; i++)
    {
        block[i] = bytes[start+i];
    }
    return block;
}


void putBlock(char *bytes, char*block, int start, int length)
{
/**
    Put block of the elements to the array.

    @param bytes
    @param block
    @param start
    @param length
*/
    for (int i=0; i<length; i++)
    {
        bytes[start+i] = block[i];
    }
}

char* clearHeader(char *bytes, int start)
{
/**
    Fill the first two lines of header by '*' in EBCDIC.

    @param bytes
    @param start
*/
    for (int line=0; line < 3; line++)
    {
        for (int symb=3; symb<80; symb++)
        {
            bytes[line*80+symb] = 92;
        }
    }
}

int bytesToInt(char *bytes, int start, int length)
{
/**
    Convert sequence of bytes into int.

    @param bytes
    @param start
    @param length
    @return a The resulting number.
*/
    int a = (unsigned char)(bytes[start]) << (8 * (length-1));
    for (int i=0; i<length-1; i++)
    {
        a = a | (unsigned char)(bytes[start+(length-1-i)]) << (8 * i);
    }
    return a;
}

double parseDMS(int coord, int order)
{
    int degrees, minutes, seconds, subseconds;
    switch (order)
    {
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
    }
    return degrees + minutes / 60.0 + (seconds + subseconds / 100.0) / 3600.0;
}

int getDMS(double coord, int order)
{   
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

vector<int> transformCoord(int coordX, int coordY, double distance, double azimut, int format, int order, int measSystem)
{
    double dOrder = (double) order;
    if (dOrder < 0)
    {
        dOrder = 1. / (-dOrder);
    }
    switch (format)
    {
        case 1: // meters or fixedTraces
        {
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
                throw invalid_argument("coordinate is too large"); 
            }
            coordX = (int) (_coordX / factor);
            coordY = (int) (_coordY / factor);
            break;
        }
        case 2: // arcseconds
        {
            double longitude = (double)(coordX * dOrder / 3600);
            double latitude = (double)(coordY * dOrder / 3600);

            vector<double> cartesian_res = shift_geo_coordinates(latitude, longitude, distance, azimut);
            vector<double> geo_res = cartesian2geo(cartesian_res);

            coordX = (int) (geo_res[1] * 3600 / dOrder);
            coordY = (int) (geo_res[0] * 3600 / dOrder);

            break;
        }
        case 3: // decimal degrees
        {
            double longitude = (double)coordX * dOrder;
            double latitude = (double)coordY * dOrder;

            vector<double> cartesian_res = shift_geo_coordinates(latitude, longitude, distance, azimut);
            vector<double> geo_res = cartesian2geo(cartesian_res);
            coordX = (int) (geo_res[1] / dOrder);
            coordY = (int) (geo_res[0] / dOrder);
            break;
        }
        case 4: // DMS
        {
            double longitude = parseDMS(coordX, order);
            double latitude = parseDMS(coordY, order);

            vector<double> cartesian_res = shift_geo_coordinates(latitude, longitude, distance, azimut);
            vector<double> geo_res = cartesian2geo(cartesian_res);
            coordX = getDMS(geo_res[1], order);
            coordY = getDMS(geo_res[0], order);
            break;
        }
    }
    vector<int> result{coordX, coordY};
    result[0] = coordX;
    result[1] = coordY;
    return result;
}

char* intToBytes(int a, int length)
{
/**
    Convert int into sequence of bytes.

    @param a
    @param length
    @return bytes Array of bytes.
*/
    char *bytes = new char[length];
    for (int i=0; i<length; i++)
    {
        bytes[length-1-i] = a >> (i * 8);
    }
    return bytes;
}

int anonymize(char* filename, double distance, double azimut, ofstream& logfile)
{
    char *ret = readFileBytes(filename);

    // anonymize text line header
    char *textLineHeader = getBlock(ret, 0, 3200);
    clearHeader(textLineHeader, 0);
    putBlock(ret, textLineHeader, 0, 3200);

    // get information from binary line header
    int numberTraces = bytesToInt(ret, 3212, 2);
    int traceLength = bytesToInt(ret, 3220, 2);
    int bytesPerRecord = FORMATS[bytesToInt(ret, 3224, 2) - 1];
    int measSystem = bytesToInt(ret, 3254, 2); // meters or feet
    unsigned char majorRevision = bytesToInt(ret, 3500, 1);
    unsigned char minorRevision = bytesToInt(ret, 3501, 1);
    int fixedTraces = bytesToInt(ret, 3502, 2);
    int numberExtendedHeaders = bytesToInt(ret, 3504, 2);
    int maxTraceHeaders = bytesToInt(ret, 3506, 2);

    int file_length = fileLength(filename);

    logfile << "Format Revision Number: " << (int)majorRevision << '.' << (int)minorRevision << '\n';
    logfile << "Additional Trace Headers: " << maxTraceHeaders << '\n';
    logfile << "Fixed length: " << fixedTraces << '\n';
    logfile << "Number of traces: " << numberTraces << '\n';
    logfile << "Trace length: " << traceLength << '\n';
    logfile << "Extended Headers: " << numberExtendedHeaders << '\n';

    // read extended text headers
    for (int extHeader=0; extHeader<numberExtendedHeaders; extHeader++)
    {
        char *textLineHeader = getBlock(ret, 3600+extHeader*3200, 3200);
        clearHeader(textLineHeader, 0);
        putBlock(ret, textLineHeader, 3600+extHeader*3200, 3200);
    }
    int actualNumber = (file_length - 3600) / (240 + traceLength*bytesPerRecord);
    if ((file_length - 3600) % (240 + traceLength*bytesPerRecord) != 0)
    {
        logfile << "Error: incorrect file length" << endl;
        return -1;
    }
    if (actualNumber != numberTraces)
    {
        logfile << "Warning: the number of traces in header (" << numberTraces << ") is not equal to the actual number of traces in file (" << actualNumber << ")\n";
        numberTraces = actualNumber;
    }

    int shift = 3600;

    // anonymize binary trace headers
    for (int i=0; i < numberTraces; i++)
    {
        if (maxTraceHeaders > 0)
        {
            maxTraceHeaders = bytesToInt(ret, shift+240+156, 2);
        }

        int numberHeaders = 1 + maxTraceHeaders;

        if (!fixedTraces)
        {
            traceLength = bytesToInt(ret, shift+114, 2);
        }

        int order = bytesToInt(ret, shift+70, 2) - (1 << 16); // coordinates factor
        int format = bytesToInt(ret, shift+88, 2); //meters or feet

        for (int nHeader=0; nHeader<numberHeaders; nHeader++)
        {

            int *coord;
            int size;

            if (nHeader == 0)
            {
                coord = MAIN_COORD;
                size = MAIN_COORD_LENGTH;
            }
            else
            {
                coord = ADD_COORD;
                size = ADD_COORD_LENGTH;
            }

            for (int j=0; j<6; j+=2)
            {
                int coordX = bytesToInt(ret, shift+coord[j], size);
                int coordY = bytesToInt(ret, shift+coord[j+1], size);

                vector<int> result{coordX, coordY};
                result = transformCoord(coordX, coordY, distance, azimut, format, order, measSystem);

                putBlock(ret, intToBytes(result[0], size), shift+coord[j], size);
                putBlock(ret, intToBytes(result[1], size), shift+coord[j+1], size);
            }
            shift += 240;
        }
        shift += traceLength*bytesPerRecord;
    }
    writeBytes(ret, file_length, filename);
    return 0;
}
