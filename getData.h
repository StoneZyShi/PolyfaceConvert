#pragma once
#include "utility.h"
#include "sqlliteRW.h"


class getData
{
public:
	getData(sqlliteRW* sqlRW, WString referencePath);
	getData();
	~getData();

	WString findAllActive();
	bool creatReference(EditElementHandleR sourceEh, WString modelName);
	WString get(PolyfaceHeaderPtr meshData);
	bool ElementToApproximateFacets(ElementHandleCR source, bvector<PolyfaceHeaderPtr> &output, IFacetOptionsP options);
	WString relativePath(WString absolutePath);

private:
	sqlliteRW* sqlRW;
	WString referencePath;

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
