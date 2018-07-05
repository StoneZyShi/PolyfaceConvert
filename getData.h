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

using namespace std;
USING_NAMESPACE_BENTLEY_DGNPLATFORM;
USING_NAMESPACE_BENTLEY_MSTNPLATFORM;
USING_NAMESPACE_BENTLEY_MSTNPLATFORM_ELEMENT;


#define pri(str) mdlDialog_dmsgsPrint(str);

class getData
{
public:
	getData(sqlliteRW* sqlRW);
	getData();
	~getData();

	void findAllActive();
	bool creatReference(EditElementHandleR sourceEh);
	std::string get(PolyfaceHeaderPtr meshData);
	bool ElementToApproximateFacets(ElementHandleCR source, bvector<PolyfaceHeaderPtr> &output, IFacetOptionsP options);
private:
	sqlliteRW* sqlRW;
};


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
		if (trans)
			m_currentTransform = *trans;
		else
			m_currentTransform.InitIdentity();
	}


	virtual BentleyStatus _ProcessFacets(PolyfaceQueryCR facets, bool isFilled = false) override
	{
		PolyfaceHeaderPtr header = PolyfaceHeader::CreateTriangleGrid(3);
		header->CopyFrom(facets);
		header->Transform(m_currentTransform);
		m_output.push_back(header);
		return SUCCESS;
	}
};
