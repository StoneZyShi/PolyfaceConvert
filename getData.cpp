#include "getData.h"
#include "utility.h"

getData::getData(sqlliteRW* sqlRW)
{
	this->sqlRW = sqlRW;
}

getData::getData()
{
}


getData::~getData()
{
}

void getData::findAllActive()
{
	DgnModelP pActiveModel = ISessionMgr::GetActiveDgnModelP();
	DgnModel::ElementsCollection elemColl = pActiveModel->GetElementsCollection();

	for (PersistentElementRefP elemRef : elemColl)
	{
		if (elemRef->IsGraphics())
		{
			EditElementHandle sourceEh(elemRef);
			creatReference(sourceEh);
		}
	}
}

bool getData::creatReference(EditElementHandleR sourceEh)
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
	facetOptions->SetNormalsRequired(true);
	facetOptions->SetParamsRequired(true);
	facetOptions->SetVertexColorsRequired(true);
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

		for (size_t i = 0; i < j; i++)
		{
			//get(meshes[i]);
			sqlRW->set_id();
			sqlRW->set_hostfile_name(ws2s(sourceEh.GetDgnFileP()->GetFileName().GetWCharCP()));
			sqlRW->set_id_infile((int)id);

			std::string str = get(meshes[i]);

			sqlRW->addData(str);
		}
	}
	else
		rtn = false;

	return rtn;
}

std::string getData::get(PolyfaceHeaderPtr meshData)
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
	
	return str;
}

bool getData::ElementToApproximateFacets(ElementHandleCR source, bvector<PolyfaceHeaderPtr>& output, IFacetOptionsP options)
{
	output.clear();
	MeshProcessor dest(output, options);

	ElementGraphicsOutput::Process(source, dest);
	return output.size() > 0 ? true : false;
}
