#ifndef ECLASSMODELNODE_H_
#define ECLASSMODELNODE_H_

#include "nameable.h"
#include "inamespace.h"
#include "modelskin.h"
#include "ientity.h"

#include "scenelib.h"
#include "scene/TraversableNodeSet.h"
#include "transformlib.h"
#include "selectionlib.h"
#include "../target/TargetableNode.h"
#include "../EntityNode.h"

#include "EclassModel.h"

namespace entity {

class EclassModelNode :
	public EntityNode,
	public scene::Cloneable,
	public Nameable,
	public Snappable,
	public TransformNode,
	public Transformable
{
	friend class EclassModel;

	EclassModel m_contained;

	mutable bool _updateSkin;

public:
	// Constructor
	EclassModelNode(const IEntityClassConstPtr& eclass);
	// Copy Constructor
	EclassModelNode(const EclassModelNode& other);

	virtual ~EclassModelNode();

	// Snappable implementation
	virtual void snapto(float snap);

	// TransformNode implementation
	virtual const Matrix4& localToParent() const;

	// EntityNode implementation
	virtual Entity& getEntity();
	virtual void refreshModel();

	// Namespaced implementation
	//virtual void setNamespace(INamespace& space);

	scene::INodePtr clone() const;

	// scene::Instantiable implementation
	virtual void instantiate(const scene::Path& path);
	virtual void uninstantiate(const scene::Path& path);

	// Renderable implementation
	void renderSolid(RenderableCollector& collector, const VolumeTest& volume) const;
	void renderWireframe(RenderableCollector& collector, const VolumeTest& volume) const;

	// Nameable implementation
	virtual std::string name() const;

	void skinChanged(const std::string& value);
	typedef MemberCaller1<EclassModelNode, const std::string&, &EclassModelNode::skinChanged> SkinChangedCaller;

protected:
	// Gets called by the Transformable implementation whenever
	// scale, rotation or translation is changed.
	void _onTransformationChanged();

	// Called by the Transformable implementation before freezing
	// or when reverting transformations.
	void _applyTransformation();

private:
	void construct();
	void destroy();
};

} // namespace entity

#endif /*ECLASSMODELNODE_H_*/
