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
#include <iostream>                                       
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic> 
#include <deque>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <windows.h>
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>
#include <time.h>

using namespace std;
USING_NAMESPACE_BENTLEY_DGNPLATFORM;
USING_NAMESPACE_BENTLEY_MSTNPLATFORM;
USING_NAMESPACE_BENTLEY_MSTNPLATFORM_ELEMENT;

#define pri(str) mdlDialog_dmsgsPrint(str);

//std::queue<EditElementHandle> myHandls;
std::map<ElementId, DgnModelRefP> mapHandls;
std::mutex handls_mutex;

namespace myThreadPool
{
	template<typename T>
	class thread_safe_queue
	{
	private:
		mutable std::mutex mut;
		std::queue<T> data_queue;
		std::condition_variable data_cond;

	public:
		thread_safe_queue() {}

		void push(T new_value)
		{
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(std::move(new_value));
			data_cond.notify_one();
		}

		void wait_and_pop(T& value)
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty(); });
			value = std::move(data_queue.front());
			data_queue.pop();
		}

		std::shared_ptr<T> wait_and_pop()
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty(); });
			std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
			data_queue.pop();
			return res;
		}

		bool try_pop(T& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty())
				return false;
			value = std::move(data_queue.front());
			data_queue.pop();
			return true;
		}

		std::shared_ptr<T> try_pop()
		{
			std::lock_guard<std::mutex> lk(muk);
			if (data_queue.empty())
				return std::shared_ptr<T>();
			std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
			data_queue.pop();
			return res;
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lk(mut);
			return data_queue.empty();
		}
	};

	class join_threads
	{
		std::vector<std::thread>& threads;
	public:
		explicit join_threads(std::vector<std::thread>& threads_) : threads(threads_) {}
		~join_threads()
		{
			for (unsigned long i = 0; i < threads.size(); ++i)
			{
				if (threads[i].joinable())
					threads[i].join();
			}
		}
	};

	class thread_pool
	{
		std::atomic_bool done;
		thread_safe_queue<std::function<void()>> work_queue;
		std::vector<std::thread> threads;
		join_threads joiner;

		void worker_thread()
		{
			while (!done)
			{
				std::function<void()> task;
				if (work_queue.try_pop(task))
					task();
				else
					std::this_thread::yield();
			}
		}
	public:
		thread_pool() :done(false), joiner(threads)
		{
			unsigned const thread_count = std::thread::hardware_concurrency();

			try
			{
				for (unsigned i = 0; i < thread_count; ++i)
				{
					threads.push_back(std::thread(&thread_pool::worker_thread, this));
				}
			}
			catch (...)
			{
				done = true;
				throw;
			}
		}
		~thread_pool()
		{
			done = true;
		}

		template<typename FunctionType>
		void submit(FunctionType f)
		{
			work_queue.push(std::function<void()>(f));
		}
	};

}

bool getHandl(ElementId& id, DgnModelRefP& modelRef)
{
	bool rtn = false;
	std::lock_guard<std::mutex> guard(handls_mutex);
	if (mapHandls.size() > 0)
	{
		std::map<ElementId, DgnModelRefP>::iterator it = mapHandls.begin();
		WString msg;
		id = it->first;
		modelRef = it->second;
		mapHandls.erase(it);
		rtn = true;
	}
	else
		rtn = false;

	return rtn;
}

void pushHandl(EditElementHandleCR sourceEh)
{
	std::lock_guard<std::mutex> guard(handls_mutex);
	//myHandls.push(sourceEh);
	mapHandls[sourceEh.GetElementId()] = sourceEh.GetModelRef();
}

WString getType(ElementHandle eh)
{
	WString rtn = L"\0";
	if (nullptr != dynamic_cast<ComplexHeaderDisplayHandler*>(&(eh.GetHandler())))
	{
		rtn = L"ComplexHeaderDisplayHandler";
	}
	else if (nullptr != dynamic_cast<ConeHandler*>(&(eh.GetHandler())))
		rtn = L"ConeHandler";
	else if (nullptr != dynamic_cast<CurveHandler*>(&(eh.GetHandler())))
		rtn = L"CurveHandler";
	else if (nullptr != dynamic_cast<DimensionHandler*>(&(eh.GetHandler())))
		rtn = L"DimensionHandler";
	else if (nullptr != dynamic_cast<EllipticArcBaseHandler*>(&(eh.GetHandler())))
		rtn = L"EllipticArcBaseHandler";
	else if (nullptr != dynamic_cast<ExtendedElementHandler*>(&(eh.GetHandler())))
		rtn = L"ExtendedElementHandler";
	else if (nullptr != dynamic_cast<LineHandler*>(&(eh.GetHandler())))
		rtn = L"LineHandler";
	else if (nullptr != dynamic_cast<LineStringBaseHandler*>(&(eh.GetHandler())))
		rtn = L"LineStringBaseHandler";
	else if (nullptr != dynamic_cast<MeshHeaderHandler*>(&(eh.GetHandler())))
	{
		rtn = L"MeshHeaderHandler\n";
	}
	else if (nullptr != dynamic_cast<MultilineHandler*>(&(eh.GetHandler())))
		rtn = L"MultilineHandler";
	else if (nullptr != dynamic_cast<PointCloudHandler*>(&(eh.GetHandler())))
		rtn = L"PointCloudHandler";
	else if (nullptr != dynamic_cast<PointStringHandler*>(&(eh.GetHandler())))
		rtn = L"PointStringHandler";
	else if (nullptr != dynamic_cast<RasterFrameHandler*>(&(eh.GetHandler())))
		rtn = L"RasterFrameHandler";
	else if (nullptr != dynamic_cast<RasterHdrHandler*>(&(eh.GetHandler())))
		rtn = L"RasterHdrHandler";
	else if (nullptr != dynamic_cast<SectionClipElementHandler*>(&(eh.GetHandler())))
		rtn = L"SectionClipElementHandler";
	else if (nullptr != dynamic_cast<SharedCellHandler*>(&(eh.GetHandler())))
		rtn = L"SharedCellHandler";
	else if (nullptr != dynamic_cast<TagElementHandler*>(&(eh.GetHandler())))
		rtn = L"TagElementHandler";
	else if (nullptr != dynamic_cast<TextHandlerBase*>(&(eh.GetHandler())))
		rtn = L"TextHandlerBase";
	else if (nullptr != dynamic_cast<IMeshQuery*>(&(eh.GetHandler())))
		rtn = L"IMeshQuery";
	else
		rtn = L"NULL";

	pri(rtn.GetWCharCP());
	return rtn;
}


struct MeshProcessor : public IElementGraphicsProcessor
{
	bvector <PolyfaceHeaderPtr> &m_output;
	IFacetOptionsP               m_options;
	Transform                    m_currentTransform;

	MeshProcessor(bvector<PolyfaceHeaderPtr> &output, IFacetOptionsP options)
		: m_output(output), m_options(options)
	{
	}

	virtual IFacetOptionsP _GetFacetOptionsP()
	{
		return m_options;
	}

	virtual bool _ProcessAsFacets(bool isPolyface) const override
	{
		return true;
	}

	virtual bool _ProcessAsBody(bool isCurved) const
	{
		return false;
	}

	virtual void _AnnounceTransform(TransformCP trans) override
	{
		//pri(L"_AnnounceTransform");
		if (trans)
			m_currentTransform = *trans;
		else
			m_currentTransform.InitIdentity();
	}


	virtual BentleyStatus _ProcessFacets(PolyfaceQueryCR facets, bool isFilled = false) override
	{
		//pri(L"_ProcessFacets");
		PolyfaceHeaderPtr header = PolyfaceHeader::CreateTriangleGrid(3);
		header->CopyFrom(facets);
		header->Transform(m_currentTransform);
		m_output.push_back(header);
		return SUCCESS;
	}
};

bool ElementToApproximateFacets(ElementHandleCR source, bvector<PolyfaceHeaderPtr> &output, IFacetOptionsP options)
{
	//pri(L"ElementToApproximateFacets");
	output.clear();
	MeshProcessor dest(output, options);

	ElementGraphicsOutput::Process(source, dest);
	return output.size() > 0 ? true : false;
}


int get(ElementHandle eh, int id)
{
	string str;
	char buf[256] = "\0";
	WString msg;

	if (nullptr != dynamic_cast<MeshHeaderHandler*>(&(eh.GetHandler())))
	{
		msg.Sprintf(L"ID=%d ,模型开始提取\n", id);
		mdlDialog_dmsgsPrint(msg.GetWCharCP());

		MeshHeaderHandler* handler = dynamic_cast<MeshHeaderHandler*>(&(eh.GetHandler()));
		PolyfaceHeaderPtr meshData;
		handler->GetMeshData(eh, meshData);

		DPoint3dCP point;
		point = meshData->GetPointCP();
		size_t count = meshData->GetPointCount();

		str = "Rows(mm)\n";
		for (size_t i = 0; i < count; i++)
		{
			sprintf(buf, "%d: (%.2f,%.2f,%.2f)\n", (int)i, point->x, point->y, point->z);
			str += buf;
			memset(buf, '\0', 256);
			point++;
		}
		size_t pic = meshData->GetPointIndexCount();

		sprintf(buf, "DataCount:%d\n  Rows:\n  0: ", (int)(meshData->GetPointIndexCount()));
		str += buf;
		memset(buf, '\0', 256);

		const int32_t* pi = meshData->GetPointIndexCP();
		int j = 1;
		for (size_t i = 0; i < pic; i++)
		{
			sprintf(buf, "%d ", (int)(abs(*pi)));
			str += buf;
			memset(buf, '\0', 256);
			if (*pi == 0 && i < pic - 1)
			{
				sprintf(buf, "\n%d: ", (int)j);
				str += buf;
				memset(buf, '\0', 256);
				j++;
			}
			pi++;
		}
		str += "\n";

		msg.Sprintf(L"ID=%d ,模型提取结束\n", id);
		mdlDialog_dmsgsPrint(msg.GetWCharCP());
	}

	char file[256] = "\0";
	sprintf(file, "d:/testData/%d.txt", id);

	ofstream out(file);
	if (out.is_open())
	{
		out << str.c_str() << endl;
		out.close();
	}
	msg.Sprintf(L"ID=%d ,循环完成\n", id);
	mdlDialog_dmsgsPrint(msg.GetWCharCP());

	return 0;
}

bool myCreateElement(EditElementHandleR sourceEh)
{
	bool rtn = true;

	//pri(L"CreateElement");
	IFacetOptionsPtr facetOptions = IFacetOptions::New();
	//Set different parameters for facet.
	facetOptions->SetIgnoreFaceMaterialAttachments(true); // Don't separate multi-symbology BReps by face symbology...
	facetOptions->SetChordTolerance(1);                 //many different parameters to control the final result mesh
	facetOptions->SetAngleTolerance(0.0);
	facetOptions->SetMaxEdgeLength(0.0);
	facetOptions->SetMaxFacetWidth(0.0);
	facetOptions->SetNormalsRequired(false);
	facetOptions->SetParamsRequired(false);
	facetOptions->SetMaxPerFace(3);
	facetOptions->SetCurvedSurfaceMaxPerFace(3);
	facetOptions->SetEdgeHiding(true);
	facetOptions->SetSmoothTriangleFlowRequired(true);

	int id = (int)(sourceEh.GetElementId());
	bvector<PolyfaceHeaderPtr> meshes;
	if (true == ElementToApproximateFacets(sourceEh, meshes, facetOptions.get()))
	{
		EditElementHandle tmpEeh;
		size_t j = meshes.size();

		WString myString;
		myString.Sprintf(L"ID=%d ,转换模型个数=%d\n", id, j);
		mdlDialog_dmsgsPrint(myString.GetWCharCP());

		for (size_t i = 0; i < j; i++)
		{
			if (SUCCESS == MeshHeaderHandler::CreateMeshElement(tmpEeh, NULL, *meshes[i], true, *sourceEh.GetModelRef()))
			{
				myString.Sprintf(L"ID=%d ,创建第%d个模型\n", id, i + 1);
				mdlDialog_dmsgsPrint(myString.GetWCharCP());
				ElementHandle source1(tmpEeh);

				get(source1, id);
			}
			else
				rtn = false;
		}
	}
	else
		rtn = false;

	return rtn;
}

void findAllActive()
{
DgnModelP pActiveModel = ISessionMgr::GetActiveDgnModelP();
DgnModel::ElementsCollection elemColl = pActiveModel->GetElementsCollection();

for (PersistentElementRefP elemRef : elemColl)
{
	if (elemRef->IsGraphics())
	{
		ElementPropertiesGetterPtr pPropsGetter = ElementPropertiesGetter::Create((ElementHandle)elemRef);
		UInt32 clr = pPropsGetter->GetColor();
		LevelId lvl = pPropsGetter->GetLevel();
		WString myString;
		myString.Sprintf(L"颜色=%d  图层=%d\n", clr, lvl);
		mdlDialog_dmsgsPrint(myString.GetWCharCP());

		EditElementHandle sourceEh(elemRef);
		myCreateElement(sourceEh);
	}
}
}

void thisFileReferenceModel(DgnModelRefP modelRef, WString defaultModelName)
{
	WString myString;
	DgnFileP file = mdlModelRef_getDgnFile(modelRef);
	myString.Sprintf(L"被参考 FileName=%ls  ModelName=%ls",
		file->GetFileName(), modelRef->GetDgnModelP()->GetModelName());
	mdlDialog_dmsgsPrint(myString.GetWCharCP());

	StatusInt* openForWriteStatus = nullptr;
	if (SUCCESS == file->LoadDgnFile(openForWriteStatus))
	{
		for (auto m : file->GetLoadedModelsCollection())
		{
			if (defaultModelName != WString(m->GetModelName()))
			{
				DgnModel::ElementsCollection mElemColl = m->GetElementsCollection();
				for (PersistentElementRefP const& mElemRef : mElemColl)
				{
					EditElementHandle mSourceEh(mElemRef);
					myCreateElement(mSourceEh);
				}
			}
		}
	}
	else
	{
		pri(L"下载参考文件失败");
	}
}

int otherFileReferenceModel(DgnModelRefP modelRef,const string filePath)
{
	if ((_access(filePath.c_str(), 0)) != -1)
	{
		//if (remove(filePath.c_str()) != 0)
		//{
		//	pri(L"删除文件失败");
		//	return -1;
		//}
	}

	WString fileName = modelRef->GetDgnFileP()->GetFileName();
	WString modelName = modelRef->GetDgnModelP()->GetModelName();

	char str[256] = "\0";
	sprintf(str, "%ls<->%ls", fileName.GetWCharCP(), modelName.GetWCharCP());

	ofstream out(filePath, ios::app);
	if (out.is_open())
	{
		out << str << endl;
		out.close();
	}
	return 0;
}

string deleteLine(string FileName)
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

void openFileAndModel(string filePath)
{
	string str = deleteLine(filePath);
	if (str.find("<->") > 0)
	{
		string fileName = str.substr(0, str.find("<->")).c_str();
		string modelName = str.substr(str.find("<->") + 3, str.rfind("<->"));

		WString name(modelName.c_str());
		mdlDialog_dmsgsPrint(name.GetWCharCP());
	}
}

string getTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d--%H-%M-%S", localtime(&timep));
	return tmp;
}

void myReferenceModel()
{
	DgnModelP pActiveModel = ISessionMgr::GetActiveDgnModelP();
	DgnModel::ElementsCollection elemColl = pActiveModel->GetElementsCollection();
	WString myString, elDescr;

	for (PersistentElementRefP const& elemRef : elemColl)//循环一次即可，每次循环里面得到内容相同
	{		
		ModelRefIteratorP   iterator;
		DgnModelRefP       modelRef;
		string timeStr = getTime();
		string filePath = "d:\\file" + timeStr + ".txt";
		int lineNum = 0;
		mdlModelRefIterator_create(&iterator, elemRef->GetDgnModelP(), MRITERATE_PrimaryChildRefs, 0);

		ofstream out(filePath, ios::app);
		if (out.is_open())
		{
			while (NULL != (modelRef = mdlModelRefIterator_getNext(iterator)))
			{
				WString fileName = modelRef->GetDgnFileP()->GetFileName();
				WString modelName = modelRef->GetDgnModelP()->GetModelName();
				char str[256] = "\0";
				sprintf(str, "%ls<->%ls",fileName.GetWCharCP(),modelName.GetWCharCP());

				out << str << endl;
				lineNum++;
			}
			out.close();
		}

		while (lineNum > 0)
		{
			openFileAndModel(filePath);
			lineNum--;
		} 

		if ((_access(filePath.c_str(), 0)) != -1)
		{
			if (remove(filePath.c_str()) != 0)
			{
				pri(L"删除文件失败");
			}
		}

		mdlModelRefIterator_free(&iterator);
		break;
	}
}


Public void startPolyfaceConvertTool(WCharCP unparsed)
{
	//findAllActive();  //本文件本Model模型
	myReferenceModel(); //参考Model
}



