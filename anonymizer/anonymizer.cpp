#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>
#include <sstream>

using namespace std;

const int FORMATS[6] = {4, 4, 2, 4, 4, 1};              // data sample format
int MAIN_COORD[6] = {72, 76, 80, 84, 180, 184};   // position of coordinates
                                                        // in trace header
const int MAIN_COORD_LENGTH = 4;                        // format of coordinates
int ADD_COORD[6] = {96, 104, 112, 120, 160, 168}; // position of coordinates
                                                        // in trace header extension
const int ADD_COORD_LENGTH = 8;                         // format of coordinates

const double EARTH_RADIUS = 6378137.0;
const double PI = 3.14159265358979323846;

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
    cout << "Length of file: " << len << '\n';
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

int transformCoordX(int coord, int format, int order, int shift, int measSystem)
{
    switch (format)
    {
        case 1:
            coord = coord + shift * 1000 * (-order);
            break;
        case 2:
            int degrees, minutes, seconds, subseconds;
            switch (order)
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
            break;
        default:
            cout << "I can't work with anything except meters :( ";
            break;
    }
    return coord;
}

int transformCoordY(int coord, int format, int order, int shift, int measSystem)
{
    switch (format)
    {
        case 1:
            coord = coord + shift * 1000 * (-order);
            break;
        case 2:
            int degrees, minutes, seconds, subseconds;
            switch (order)
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
            double newcoord = degrees + minutes / 60.0 + (seconds + subseconds / 100.0) / 3600.0;
            newcoord = newcoord + (180.0 * shift) / (EARTH_RADIUS * PI);

            degrees = (int) newcoord;
            newcoord = (newcoord - degrees) * 60;

            minutes = (int) newcoord;
            newcoord = (newcoord - minutes) * 60;

            seconds = (int) newcoord;
            newcoord = (newcoord - seconds) * 100;

            subseconds = (int) newcoord;

            coord = degrees * 10000 + minutes * 100 + seconds;
            if (order == -100)
            {
                coord = coord * 100 + subseconds;
            }

            break;
        default:
            cout << "I can't work with anything except meters :( ";
            break;
    }
    return coord;
}

int shiftCoordinateX(int coord, int shift)
{
    return coord+shift;
}

int shiftCoordinateY(int coord, int shift)
{
    return coord+shift;
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
        bytes[length-1-i] = (unsigned char) (a % 256);
        a = a / 256;
    }
    return bytes;
}

int anonimize(char* filename, int shiftX, int shiftY)
{
    char *ret = readFileBytes(filename);
      
    char *textLineHeader = getBlock(ret, 0, 3200);
    clearHeader(textLineHeader, 0);
    putBlock(ret, textLineHeader, 0, 3200);

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
    
    cout << "Format Revision Number: " << (int)majorRevision << '.' << (int)minorRevision << '\n';
    cout << "Additional Trace Headers: " << maxTraceHeaders << '\n';
    cout << "Fixed length: " << fixedTraces << '\n';
    cout << "Number of traces: " << numberTraces << '\n';
    cout << "Trace length: " << traceLength << '\n';
    cout << "Extended Headers: " << numberExtendedHeaders << '\n';
    
    for (int extHeader=0; extHeader<numberExtendedHeaders; extHeader++)
    {
        char *textLineHeader = getBlock(ret, 3600+extHeader*3200, 3200);
        clearHeader(textLineHeader, 0);
        putBlock(ret, textLineHeader, 3600+extHeader*3200, 3200);
    }

    int actualNumber = (file_length - 3600) / (240 + traceLength*bytesPerRecord);
    
    if ((file_length - 3600) % (240 + traceLength*bytesPerRecord) != 0)
    {
        cout << "ERROR\n";
        return -1;
    }
    
    
    if (actualNumber != numberTraces)
    {
        cout << "The number of traces in header (" << numberTraces << ") is not equal to the actual number of traces in file (" << actualNumber << ")\n";
        numberTraces = actualNumber;
    }

    int shift = 3600;

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

        int order = bytesToInt(ret, shift+70, 2) - (1 << 16);
        int format = bytesToInt(ret, shift+88, 2); //meters or feet

        // cout << "Coordinates format: " << format << '\n';
        // cout << "Coordinates order: " << order << '\n';

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

                coordX = transformCoordX(coordX, format, order, shiftX, measSystem);
                coordY = transformCoordY(coordY, format, order, shiftY, measSystem);

                putBlock(ret, intToBytes(coordX, size), shift+coord[j], size);
                putBlock(ret, intToBytes(coordY, size), shift+coord[j+1], size);
            }
            shift += 240;
        }
        shift += traceLength*bytesPerRecord;
    }

    filename = (char *)"anonimized.sgy";
    writeBytes(ret, file_length, filename);
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << "Parameters error";
    }
    else
    {
        char* filename = argv[1];
        int shiftX = stoi(argv[2]);
        int shiftY = stoi(argv[3]);
        anonimize(filename, shiftX, shiftY);
    }
    return 0;
}

