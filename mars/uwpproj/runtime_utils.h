#ifndef RUNTIME_UTILS
#define RUNTIME_UTILS

#include <string>

#define MAX_PATH          260
using namespace std;
using namespace Platform;

char* toMbs(const wchar_t * src, int len);

wchar_t* toWcs(const char* src, int len);

char* getIsoPath(char root_path[MAX_PATH]);

string String2stdstring(String^ inString);


void String2stdstring(string & outString, String^ inString);//reduce memory op

String^ stdstring2String(const string & instring);


extern int gMMProtectNeedExit;

#endif