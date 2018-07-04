#include "findAllReference.h"
#include "getData.h"
#include <io.h>

using namespace std;

extern "C" void mdlSystem_setBatchProcessingState(BatchProcessing_States, const WChar*);

findAllReference::findAllReference()
{
}


findAllReference::~findAllReference()
{
}

void findAllReference::openFileAndModel(std::string filePath)
{
	string str = deleteLine(filePath);
	if (str.find("<->") > 0)
	{
		string fileName = str.substr(0, str.find("<->")).c_str();
		string modelName = str.substr(str.find("<->") + 3, str.rfind("<->"));

		WString file(fileName.c_str()), mode(modelName.c_str());

		mdlSystem_setBatchProcessingState(PROCESSSTATE_Processing, NULL);
		if (SUCCESS == mdlSystem_newDesignFileAndModel(file.GetWCharCP(), mode.GetWCharCP()))
		{
			Sleep(5000);

			getData data;
			data.findAllActive();  //本文件本Model模型
			scanFeference(); //参考Model
		}
	}
}

void findAllReference::scanFeference()
{
	DgnModelP pActiveModel = ISessionMgr::GetActiveDgnModelP();
	DgnModel::ElementsCollection elemColl = pActiveModel->GetElementsCollection();
	WString myString, elDescr;
	WString thisFileName(pActiveModel->GetDgnFileP()->GetFileName()), thisModelName(pActiveModel->GetModelName());

	string timeStr = getTime();
	string filePath = "d:\\file" + timeStr + ".txt";

	for (PersistentElementRefP const& elemRef : elemColl)//循环一次即可，每次循环里面得到内容相同
	{
		ModelRefIteratorP   iterator;
		DgnModelRefP       modelRef;
		int lineNum = 0;
		mdlModelRefIterator_create(&iterator, elemRef->GetDgnModelP(), MRITERATE_PrimaryChildRefs, 0);

		ofstream out(filePath, ios::app);
		if (out.is_open())
		{
			while (nullptr != (modelRef = mdlModelRefIterator_getNext(iterator)))
			{
				if (nullptr != modelRef->GetDgnFileP())
				{
					WString fileName = modelRef->GetDgnFileP()->GetFileName();
					WString dgn = L"dgn";
					if (fileName.substr(fileName.size() - 3, 3) == dgn)
					{
						WString modelName = modelRef->GetDgnModelP()->GetModelName();
						char str[256] = "\0";
						sprintf(str, "%ls<->%ls", fileName.GetWCharCP(), modelName.GetWCharCP());

						out << str << endl;
						lineNum++;
					}
				}
			}
			out.close();
		}

		while (lineNum > 0)
		{
			openFileAndModel(filePath);
			lineNum--;
		}

		mdlModelRefIterator_free(&iterator);

		break;
	}
	if ((_access(filePath.c_str(), 0)) != -1)
	{
		if (remove(filePath.c_str()) != 0)
		{
			pri(L"删除文件失败");
		}
		mdlSystem_newDesignFileAndModel(thisFileName.GetWCharCP(), thisModelName.GetWCharCP());
	}
}




