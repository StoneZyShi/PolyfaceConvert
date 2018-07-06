
#include "findAllReference.h"
#include "fileControl.h"
#include "utility.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include  <direct.h>  


Public void startPolyfaceConvertTool(WCharCP unparsed)
{
	bool flag = true;

	WString fileDB = getRootPath().substr(0, getRootPath().rfind(L".")) + L".db";
#ifdef MY_DEBUG
	pri(fileDB.GetWCharCP());
#endif

	std::string fileSQL = ws2s(fileDB.GetWCharCP());
	if ((_access(fileSQL.c_str(), 0)) != -1)
	{
		if (remove(fileSQL.c_str()) != 0)
		{
			pri(L"删除文件失败");
			flag = false;
		}
	}
	
	if (flag) 
	{
		sqlliteRW sqlRW(fileDB);
		sqlRW.creatObjTable();

		WString referencePath = L"";

		fileControl fileC(&sqlRW, referencePath, getRootPath());
		fileC.start();
#ifdef MY_DEBUG
		pri(fileC.getRootFile().GetWCharCP());
		pri(fileC.getRootModel().GetWCharCP());
#endif // MY_DEBUG

		mdlSystem_newDesignFileAndModel(fileC.getRootFile().GetWCharCP(), fileC.getRootModel().GetWCharCP());

		pri(L"提取完成！！！");
	}
}



