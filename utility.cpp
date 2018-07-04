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

