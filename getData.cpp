#include "getData.h"
#include "findAllReference.h"

getData::getData(sqlliteRW* sqlRW, WString referencePath)
{
	this->sqlRW = sqlRW;
	if (referencePath == L"")
		this->referencePath = referencePath;
	else
		this->referencePath = referencePath + L"->";
}

getData::getData()
{
}


getData::~getData()
{
}

WString getData::findAllActive()
{
	DgnModelP pActiveModel = ISessionMgr::GetActiveDgnModelP();
	DgnModel::ElementsCollection elemColl = pActiveModel->GetElementsCollection();

	for (PersistentElementRefP elemRef : elemColl)
	{
		if (elemRef->IsGraphics())
		{
			EditElementHandle sourceEh(elemRef);
			WString name = sourceEh.GetDgnModelP()->GetModelName();
#ifdef  MY_DEBUG
			pri(name.GetWCharCP());
#endif //  MY_DEBUG
			creatReference(sourceEh,name);
		}
	}

	return referencePath;
}

bool getData::creatReference(EditElementHandleR sourceEh, WString modelName)
{
	bool rtn = true;

	IFacetOptionsPtr facetOptions = IFacetOptions::New();
	//Set different parameters for facet.
	facetOptions->SetIgnoreFaceMaterialAttachments(true); // Don't separate multi-symbology BReps by face symbology...
	facetOptions->SetChordTolerance(1);                 //many different parameters to control the final result mesh
	facetOptions->SetAngleTolerance(0.0);
	facetOptions->SetMaxEdgeLength(0.0);
	facetOptions->SetMaxFacetWidth(0.0);
	facetOptions->SetNormalsRequired(true);
	facetOptions->SetParamsRequired(true);
	facetOptions->SetVertexColorsRequired(true);
	facetOptions->SetMaxPerFace(3);
	facetOptions->SetCurvedSurfaceMaxPerFace(3);
	facetOptions->SetEdgeHiding(true);
	facetOptions->SetSmoothTriangleFlowRequired(true);

	unsigned int id = (unsigned int)(sourceEh.GetElementId());
	bvector<PolyfaceHeaderPtr> meshes;
	if (true == ElementToApproximateFacets(sourceEh, meshes, facetOptions.get()))
	{
		EditElementHandle tmpEeh;
		size_t j = meshes.size();

		for (size_t i = 0; i < j; i++)
		{
			sqlRW->set_id();
			WString fileName = sourceEh.GetDgnFileP()->GetFileName();
			referencePath += fileName + L"(" + modelName + L")";
#ifdef MY_DEBUG
			pri(referencePath.GetWCharCP());
#endif // MY_DEBUG
			sqlRW->set_hostfile_name(referencePath);
			sqlRW->set_id_infile((unsigned int)id);

			WString str = get(meshes[i]);

			sqlRW->addData(str);
		}
	}
	else
		rtn = false;

	return rtn;
}

WString getData::get(PolyfaceHeaderPtr meshData)
{
	std::string str = "o\n";
	char buf[256] = "\0";
	WString msg;

	size_t pointCount = meshData->GetPointCount();
	size_t PointIndexCount = meshData->GetPointIndexCount();
	size_t normalCount = meshData->GetNormalCount();

	//点
	DPoint3dCP point = meshData->GetPointCP();
	for (size_t i = 0; i < pointCount; i++)
	{
		sprintf(buf, "v %.7f %.7f %.7f\n", (point->x) / 10000, (point->y) / 10000, (point->z) / 10000);
		str += buf;
		memset(buf, '\0', 256);
		point++;
	}

	//法线
	DVec3dCP normal = meshData->GetNormalCP();
	for (size_t i = 0; i < normalCount; i++)
	{
		sprintf(buf, "vn %.7f %.7f %.7f\n", normal->x, normal->y, normal->z);
		str += buf;
		memset(buf, '\0', 256);
		normal++;
	}

	int32_t const* normalIndex = meshData->GetNormalIndexCP(false);

	//面		
	const int32_t* pi = meshData->GetPointIndexCP();
	str += "f ";
	for (size_t i = 0; i < PointIndexCount; i++)
	{
		if (normalIndex != nullptr)
			sprintf(buf, "%d//%d ", (int)(abs(*pi)), *normalIndex);
		else
			sprintf(buf, "%d ", (int)(abs(*pi)));
		if (*pi != 0)
		{
			str += buf;
			memset(buf, '\0', 256);
		}
		else
		{
			if (i < PointIndexCount - 1)
				str += "\nf ";
		}
		pi++;
		if (normalIndex != nullptr)
			normalIndex++;
	}
	
	return WString(str.c_str());
}

bool getData::ElementToApproximateFacets(ElementHandleCR source, bvector<PolyfaceHeaderPtr>& output, IFacetOptionsP options)
{
	output.clear();
	MeshProcessor dest(output, options);

	ElementGraphicsOutput::Process(source, dest);
	return output.size() > 0 ? true : false;
}

WString getData::relativePath(WString absolutePath)
{
	if (this->referencePath == L"")
		return absolutePath.substr(absolutePath.rfind(L"\\"), absolutePath.length() - absolutePath.rfind(L"\\"));
	else
	{

	}

	return WString();
}
