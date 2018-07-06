#include "fileControl.h"
#include <io.h>
#include <vector>



fileControl::fileControl(sqlliteRW* sqlite, WString referencePath, WString rootPath)
{
	this->sqlite = sqlite;
	this->referencePath = referencePath;
	this->rootPath = rootPath;

	data = new getData(sqlite,referencePath);
	reference = new findAllReference(sqlite, referencePath,rootPath);
}


fileControl::~fileControl()
{
	if(data != nullptr)
		delete data;
	if (reference != nullptr)
		delete reference;
}

WString fileControl::start()
{
	std::string referenceFile;
	int lineNum = 0;

	reference->setReferencePath(data->findAllActive());
	reference->scanFeference(referenceFile,lineNum);

	while (lineNum > 0)
	{
		reference->openFileAndModel(referenceFile);
		lineNum--;
	}

	if ((_access(referenceFile.c_str(), 0)) != -1)
	{
		if (remove(referenceFile.c_str()) != 0)
		{
			pri(L"É¾³ýÎÄ¼þÊ§°Ü");
		}
	}

	return WString();
}

WString fileControl::getRootFile()
{
	return rootPath.substr(0,rootPath.rfind(L"("));
}

WString fileControl::getRootModel()
{
	return rootPath.substr(rootPath.rfind(L"(")+1, rootPath.rfind(L")")-2);
}
