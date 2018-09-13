#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string> 

using namespace std;

int FORMATS[6] = {4, 4, 2, 4, 4, 1};

char* readFileBytes(const char *name)  
{  
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

char* writeBytes(char* bytes, int length, const char *name)
{
	ofstream fs(name, ios::out | ios::binary | ios::app);
    fs.write(bytes, length);
    fs.close();
}

char* getBlock(char *bytes, int start, int length)
{
	char *block = new char[length];
	for (int i=0; i<length; i++)
	{
		block[i] = bytes[start+i];
	}
	return block;
}


void putBlock(char *bytes, char*block, int start, int length)
{
	for (int i=0; i<length; i++)
	{
		bytes[start+i] = block[i];
	}
}

char* ebcdicToAscii(char *bytes, int length)
{
	char *header = new char[length];
    static const unsigned char e2a[256] = {
	    0,  1,  2,  3,156,  9,134,127,151,141,142, 11, 12, 13, 14, 15,
	    16, 17, 18, 19,157,133,  8,135, 24, 25,146,143, 28, 29, 30, 31,
	    128,129,130,131,132, 10, 23, 27,136,137,138,139,140,  5,  6,  7,
	    144,145, 22,147,148,149,150,  4,152,153,154,155, 20, 21,158, 26,
	    32,160,161,162,163,164,165,166,167,168, 91, 46, 60, 40, 43, 33,
	    38,169,170,171,172,173,174,175,176,177, 93, 36, 42, 41, 59, 94,
        45, 47,178,179,180,181,182,183,184,185,124, 44, 37, 95, 62, 63,
        186,187,188,189,190,191,192,193,194, 96, 58, 35, 64, 39, 61, 34,
        195, 97, 98, 99,100,101,102,103,104,105,196,197,198,199,200,201,
        202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,208,
        209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,215,
        216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
        123, 65, 66, 67, 68, 69, 70, 71, 72, 73,232,233,234,235,236,237,
        125, 74, 75, 76, 77, 78, 79, 80, 81, 82,238,239,240,241,242,243,
        92,159, 83, 84, 85, 86, 87, 88, 89, 90,244,245,246,247,248,249,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57,250,251,252,253,254,255
	};
	for (int i=0; i<length; i++)
	{
		unsigned int code = (unsigned int)(unsigned char)bytes[i];
		header[i] = e2a[code];
	}
	return header;
}

char* asciiToEbcdic(char *bytes, int length)
{
	char *header = new char[length];
    static const unsigned char a2e[256] = {
	    0,  1,  2,  3,  55, 45, 46, 47, 22, 5,  37, 11, 12, 13, 14, 15,
	    16, 17, 18, 19, 60, 61, 50, 38, 24, 25, 63, 39, 28, 29, 30, 31,
	    64, 79, 127,123,91, 108,80, 125,77, 93, 92, 78, 107,96, 75, 97,
	    240,241,242,243,244,245,246,247,248,249,122,94, 76, 126,110,111,
	    124,193,194,195,196,197,198,199,200,201,209,210,211,212,213,214,
	    215,216,217,226,227,228,229,230,231,232,233,74, 224,90, 95, 109,
	    121,129,130,131,132,133,134,135,136,137,145,146,147,148,149,150,
	    151,152,153,162,163,164,165,166,167,168,169,192,106,208,161,7,
	    32, 33, 34, 35, 36, 21, 6,  23, 40, 41, 42, 43, 44, 9,  10, 27,
	    48, 49, 26, 51, 52, 53, 54, 8,  56, 57, 58, 59, 4,  20, 62, 225,
	    65, 66, 67, 68, 69, 70, 71, 72, 73, 81, 82, 83, 84, 85, 86, 87,
	    88, 89, 98, 99, 100,101,102,103,104,105,112,113,114,115,116,117,
	    118,119,120,128,138,139,140,141,142,143,144,154,155,156,157,158,
	    159,160,170,171,172,173,174,175,176,177,178,179,180,181,182,183,
	    184,185,186,187,188,189,190,191,202,203,204,205,206,207,218,219,
	    220,221,222,223,234,235,236,237,238,239,250,251,252,253,254,255
	};
	for (int i=0; i<length; i++)
	{
		header[i] = a2e[bytes[i]];
	}
	return header;
}

char* clearHeader(char *bytes, int start)
{
	for (int line=0; line < 39; line++)
	{
		for (int symb=3; symb<80; symb++)
		{
			bytes[line*80+symb] = '*';
		}
	}
}

int bytesToInt(char *bytes, int start, int length)
{
	int a = (unsigned char)(bytes[start]) << (8 * (length-1));
	for (int i=0; i<length-1; i++)
	{
		a = a | (unsigned char)(bytes[start+(length-1-i)]) << (8 * i);
	}
    return a;
}

char* intToBytes(int a, int length)
{
	char *bytes = new char[length];
	for (int i=0; i<length; i++)
	{
		bytes[length-1-i] = (unsigned char) (a % 256);
		a = a / 256;
	}
	return bytes;
}

int main () {
	srand(time(0));
	
    char *ret = readFileBytes("OGA.2016.SWA.SH812DL006.SW81-724_724.1.GEOKINET.RAWPSTM.FULLSTK.sgy");
  	
    char *textLineHeader = getBlock(ret, 0, 3200);
  	textLineHeader = ebcdicToAscii(textLineHeader, 3200);
  	clearHeader(textLineHeader, 0);
  	textLineHeader = asciiToEbcdic(textLineHeader, 3200);
  	putBlock(ret, textLineHeader, 0, 3200);

	int numberTraces = bytesToInt(ret, 3212, 2);
	int traceLength = bytesToInt(ret, 3220, 2);
	int bytesPerRecord = FORMATS[bytesToInt(ret, 3224, 2) - 1];
    
    int randomX = 100000; //rand();
    int randomY = 100000; //rand();
	
	int file_length = 18155376;
	
	for (int i=0; i < numberTraces; i++)
	{
		int shift = 3600+(240+traceLength*bytesPerRecord)*i;
		
		int sourceX = bytesToInt(ret, shift+72, 4)+randomX;
		int sourceY = bytesToInt(ret, shift+76, 4)+randomY;
		
		int groupX = bytesToInt(ret, shift+80, 4)+randomX;
		int groupY = bytesToInt(ret, shift+84, 4)+randomY;
		
		int cdpX = bytesToInt(ret, shift+180, 4)+randomX;
		int cdpY = bytesToInt(ret, shift+184, 4)+randomY;
		
		putBlock(ret, intToBytes(sourceX, 4), shift+72, 4);
		putBlock(ret, intToBytes(sourceY, 4), shift+76, 4);
	
	    putBlock(ret, intToBytes(groupX, 4), shift+80, 4);
		putBlock(ret, intToBytes(groupY, 4), shift+84, 4);
		
		putBlock(ret, intToBytes(cdpX, 4), shift+180, 4);
		putBlock(ret, intToBytes(cdpY, 4), shift+184, 4);
	}
	writeBytes(ret, file_length, "t.sgy");
    return 0;
}
