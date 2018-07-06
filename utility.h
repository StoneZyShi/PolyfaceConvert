#pragma once
#include "polyfacetool.h"
#include "..\include\dgnplatform\MeshHeaderHandler.h"
#include "..\include\DgnPlatform\ElementGraphics.h"
#include "..\include\DgnView\LocateSubEntityTool.h"
#include <Mstn\ISessionMgr.h> 
#include <Mstn\MdlApi\MdlApi.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform\Handler.h>
#include <PSolid\PSolidCoreAPI.h>
#include <DgnPlatform\SectionClip.h>

#include "utility.h"
#include <Windows.h>
#include <iostream>                                       
#include <fstream>
#include <string>
#include "sqlliteRW.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM;
USING_NAMESPACE_BENTLEY_MSTNPLATFORM;
USING_NAMESPACE_BENTLEY_MSTNPLATFORM_ELEMENT;


#define pri(str) mdlDialog_dmsgsPrint(str);

#define MY_DEBUG

std::string getTime();

std::string deleteLine(std::string FileName);

std::string WChar2Ansi(LPCWSTR pwszSrc);

std::string ws2s(const std::wstring& ws);

std::wstring Ansi2WChar(LPCSTR pszSrc, int nLen);

std::wstring s2ws(const std::string& s);

//int compress(const char* str1);

WString getRootPath();
