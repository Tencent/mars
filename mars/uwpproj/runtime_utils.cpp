
#include <collection.h>

#include "runtime_utils.h"

using namespace std;

using namespace Platform;

char* toMbs(const wchar_t * src, int len)
{
	char * szRet;
	int size = WideCharToMultiByte(CP_ACP, NULL, src, len, NULL, 0, NULL, FALSE);
	if (size <= 0)
		return NULL;
	szRet = new char[size + 1];
	memset(szRet, 0, size + 1);

	WideCharToMultiByte(CP_ACP, NULL, src, len, szRet, size, NULL, FALSE);
	return szRet;
}

wchar_t* toWcs(const char* src, int len)
{
	wchar_t* wszRet;
	int size = MultiByteToWideChar(CP_ACP, 0, (char*)src, len, NULL, 0);
	if (size <= 0)
		return NULL;
	wszRet = new wchar_t[size + 1];
	memset(wszRet, 0, (size + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, (char*)src, len, wszRet, size);
	return wszRet;
}


char* getIsoPath(char root_path[MAX_PATH])
{
	Windows::Storage::StorageFolder^ local = Windows::Storage::ApplicationData::Current->LocalFolder;

	char* path = toMbs(local->Path->Data(), local->Path->Length());
	if (path == NULL)
	{
		return root_path;
	}

	//int srclen = strlen(path);
	strcpy_s(root_path, MAX_PATH, path);
	delete[] path;
	path = NULL;

	strcat_s(root_path, MAX_PATH, "\\");//???andrewu不需要加\了

	return root_path;
}

string String2stdstring(String^ inString)
{
	if (inString == nullptr)
		return "";

	const wchar_t *src = inString->Data();
	int len = inString->Length();
	char* dst = toMbs(src, len);

	//char *dst = new char[len + 1];

	//for (int i = 0; i < len; i++)
	//{
	//	dst[i] = (char)src[i];
	//}

	//dst[len] = 0;

	string out = dst;
	delete[] dst;
	dst = NULL;
	return out;
}

void String2stdstring(string & outString, String ^ inString)
{
	if (inString == nullptr)
		return outString.clear();

	const wchar_t *src = inString->Data();
	int len = inString->Length();
	char* dst = toMbs(src, len);

	//char *dst = new char[len + 1];

	//for (int i = 0; i < len; i++)
	//{
	//	dst[i] = (char)src[i];
	//}

	//dst[len] = 0;

	outString = dst;
	delete[] dst;
	dst = NULL;
}

String^ stdstring2String(const string & instring)
{
	const char* src = instring.c_str();
	int len = instring.length();
	wchar_t *dst = toWcs(src, len);

	//wchar_t *dst = new wchar_t[len + 1];

	//for (int i = 0; i < len; i++)
	//{
	//	dst[i] = src[i];
	//}
	//dst[len] = 0;

	String^ out = ref new String(dst);
	delete[] dst;
	dst = NULL;

	return out;
}

int gMMProtectNeedExit = 0;