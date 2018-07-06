#include "utility.h"
#include <iostream>
#include <time.h>
#include <vector>
#include <fstream>
#include <string.h>

using namespace std;

std::string getTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d--%H-%M-%S", localtime(&timep));
	return tmp;
}

std::string deleteLine(std::string FileName)
{
	vector<string> vecContent;
	string strLine, str;
	ifstream inFile(FileName);

	getline(inFile, str);
	vecContent.push_back(str);
	while (inFile)
	{
		getline(inFile, strLine);
		vecContent.push_back(strLine);
	}
	inFile.close();

	vecContent.erase(vecContent.begin());
	ofstream outFile(FileName);
	vector<string>::const_iterator iter = vecContent.begin();
	for (; vecContent.end() != iter; ++iter)
	{
		outFile.write((*iter).c_str(), (*iter).size());
		outFile << '\n';
	}

	outFile.close();

	return str;
}


std::string WChar2Ansi(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
	if (nLen <= 0) return std::string("");
	char* pszDst = new char[nLen]();
	if (NULL == pszDst) return std::string("");
	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;
	std::string strTemp(pszDst);
	delete[] pszDst;
	return strTemp;
}

std::string ws2s(const std::wstring& ws)
{
	return WChar2Ansi(ws.c_str());
}

std::wstring Ansi2WChar(LPCSTR pszSrc, int nLen)
{
	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
	if (nSize <= 0) return NULL;

	WCHAR *pwszDst = new WCHAR[nSize + 1]();
	if (NULL == pwszDst) return NULL;

	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, pwszDst, nSize);
	pwszDst[nSize] = 0;

	if (pwszDst[0] == 0xFEFF) // skip Oxfeff  
		for (int i = 0; i < nSize; i++)
			pwszDst[i] = pwszDst[i + 1];

	std::wstring wcharString(pwszDst);
	delete pwszDst;


	return wcharString;
}

std::wstring s2ws(const std::string& s)
{
	return Ansi2WChar(s.c_str(), (int)(s.size()));
}


WString getRootPath()
{
	WString filePath = ISessionMgr::GetActiveDgnModelP()->GetDgnFileP()->GetFileName();
	filePath = filePath.substr(0, filePath.rfind(L"\\"));
	return filePath;
}

//int compress(const char * str1)
//{
	//char* str = new char[strlen(str1)];
	//strcpy(str,str1);
	//
	//while (strlen(str) > 4)
	//{
	//	int len = strlen(str) % 2 > 0 ? strlen(str) / 2 : strlen(str) / 2 + 1;
	//	for (int i = 0; i < strlen(str) / 2; i++)
	//	{
	//		str[i] ^= str[i + len];
	//		str[i + len] = '\0';
	//	}
	//}

	//unsigned int a = 0;
	//for (int i = 0; i < strlen(str); i++)
	//{
	//	a += (unsigned int)str[i] << 4 * i;
	//}

	//return a;
//}
