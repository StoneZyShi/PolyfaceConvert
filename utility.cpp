#include "utility.h"
#include <iostream>
#include <time.h>
#include <vector>
#include <fstream>

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
