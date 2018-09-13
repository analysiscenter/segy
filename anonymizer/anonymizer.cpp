#include <iostream>
#include <fstream>

using namespace std;

char* readFileBytes(const char *name)  
{  
    ifstream fl(name);  
    fl.seekg( 0, ios::end );  
    size_t len = fl.tellg();  
    char *ret = new char[len];  
    fl.seekg(0, ios::beg);   
    fl.read(ret, len);  
    fl.close();  
    return ret;  
}

char* readTextHeader(char *bytes, int start, int length)
{
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

	char *a2e = new char[256];

	for (int i=0; i<256; i++)
	{
		a2e[i] = 0;
	}

	for (int i=0; i<256; i++)
	{
		for (int j=0; j<256; j++)
		{
		  	if (e2a[j] == i)
		  	{
				a2e[i] = j;
				break;
			}
		}
	}
	
	char *header = new char[length];

	for (int i=start; i<start+length; i++)
	{
		unsigned int code = (unsigned int)(unsigned char)bytes[i];
		header[i] = e2a[code];
	}
	
	return header;
}

int bytesToInt(char *bytes, int start, int length)
{
	//cout << "bytes:" << (int)bytes[start+0] << ' ' << (int)bytes[start+1] << ' '<< (int)bytes[start+2] << ' ' << (int)bytes[start+3] << '\n';
	cout << "Bytes " << start-3600 << '-' << start+4-3600 << '\n';
	
	
	int a = (unsigned char)(bytes[start]) << (8 * (length-1));
	for (int i=0; i<length-1; i++)
	{
		a = a | (unsigned char)(bytes[start+(length-1-i)]) << (8 * i);
	}

    cout << "int: " << a << '\n';
    return a;
}

int* getBinaryHeader(char *bytes, int start, int length)
{
	int* numbers = new int[10];
	for (int i=start; i<start+length; i+=4)
	{
		int number = bytesToInt(bytes, i, 4);		
	}
	numbers[0] = 0;
	return numbers;
}

int main () {
	
	char *empty_header[256];
	string str = "C01 ****** \n C02 ****** \n";
	char *ret = readFileBytes("2010_012_FA-l10f1.segy");
    
    cout << "Read Line Text Header\n";
  	char *textLineHeader = readTextHeader(ret, 0, 3200);
	
	cout << bytesToInt(ret, 3600+72, 4) << '\n';
	cout << bytesToInt(ret, 3600+76, 4) << '\n';
	cout << bytesToInt(ret, 3600+80, 4) << '\n';
	cout << bytesToInt(ret, 3600+84, 4) << '\n';
	
	
  return 0;
}
