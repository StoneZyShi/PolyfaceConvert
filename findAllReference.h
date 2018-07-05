#pragma once
#include <string>
#include "sqlliteRW.h"
class findAllReference
{
public:
	findAllReference(sqlliteRW* sqlRW);
	~findAllReference();

	void openFileAndModel(std::string filePath);
	void scanFeference();

private:
	sqlliteRW* sqlRW;
};
