#pragma once
#include <string>
#include "sqlliteRW.h"

extern "C" void mdlSystem_setBatchProcessingState(BatchProcessing_States, const WChar*);

class findAllReference
{
public:
	findAllReference(sqlliteRW* sqlRW, WString referencePath, WString rootPath);
	~findAllReference();

	void openFileAndModel(std::string filePath);
	void scanFeference(std::string& referenceFile, int& lineNum);
	void setReferencePath(WString path);
private:
	sqlliteRW* sqlRW;
	WString referencePath;
	WString rootPath;
};
