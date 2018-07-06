#pragma once
#include "getData.h"
#include "sqlliteRW.h"
#include "findAllReference.h"


class fileControl
{
public:
	fileControl(sqlliteRW* sqlite, WString referencePath, WString rootPath);
	~fileControl();

	WString start();
	WString getRootFile();
	WString getRootModel();

private:
	getData* data;
	sqlliteRW* sqlite;
	findAllReference* reference;
	WString referencePath;
	WString rootPath;
};

