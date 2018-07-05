
#include "findAllReference.h"
#include "getData.h"
#include "sqlliteRW.h"

Public void startPolyfaceConvertTool(WCharCP unparsed)
{
	char file[256] = "\0";
	sprintf(file, "d:/testData/test.db");
	sqlliteRW sqlRW(file);
	sqlRW.creatObjTable();

	getData data(&sqlRW);
	data.findAllActive();
	findAllReference references(&sqlRW);
	references.scanFeference();

	pri(L"提取完成！！！");
}



