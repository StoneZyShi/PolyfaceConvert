#include "findAllReference.h"
#include "getData.h"
#include "utility.h"
#include "fileControl.h"

using namespace std;

findAllReference::findAllReference(sqlliteRW* sqlRW, WString referencePath, WString rootPath)
{
	this->sqlRW = sqlRW;
	this->referencePath = referencePath;
	this->rootPath = rootPath;
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

		str = fileName + "(" + modelName + ")";
		//referencePath += WString(str.c_str());

		WString file(fileName.c_str());
		WString mode(modelName.c_str());

		mdlSystem_setBatchProcessingState(PROCESSSTATE_Processing, NULL);
		if (SUCCESS == mdlSystem_newDesignFileAndModel(file.GetWCharCP(), mode.GetWCharCP()))
		{
			Sleep(5000);

			fileControl fileC(this->sqlRW, referencePath,this->rootPath);
			fileC.start();
		}
	}
}


void findAllReference::scanFeference(std::string& referenceFile, int& lineNum)
{
	DgnModelP pActiveModel = ISessionMgr::GetActiveDgnModelP();
	DgnModel::ElementsCollection elemColl = pActiveModel->GetElementsCollection();
	WString myString, elDescr;
	WString thisFileName(pActiveModel->GetDgnFileP()->GetFileName()), thisModelName(pActiveModel->GetModelName());

	referenceFile = ws2s(getRootPath().GetWCharCP()) +"/reference_" + getTime() + ".txt";
	
	for (PersistentElementRefP const& elemRef : elemColl)//循环一次即可，每次循环里面得到内容相同
	{
		ModelRefIteratorP   iterator;
		DgnModelRefP       modelRef;
		mdlModelRefIterator_create(&iterator, elemRef->GetDgnModelP(), MRITERATE_PrimaryChildRefs, 0);
		
		ofstream out(referenceFile, ios::app);
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

		mdlModelRefIterator_free(&iterator);

		break;
	}
}

void findAllReference::setReferencePath(WString path)
{
	referencePath = path;
}




