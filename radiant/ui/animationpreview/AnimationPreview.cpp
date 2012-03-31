#include "AnimationPreview.h"

#include "i18n.h"
#include "imodel.h"
#include "ientity.h"
#include "ieclass.h"
#include "imd5anim.h"
#include "itextstream.h"
#include "math/AABB.h"

#include "gtkutil/GLWidget.h"

namespace ui
{

namespace
{
	const char* const FUNC_STATIC_CLASS = "func_static";
}

AnimationPreview::AnimationPreview() :
	gtkutil::RenderPreview()
{}

// Set the model, this also resets the camera
void AnimationPreview::setModelNode(const scene::INodePtr& node)
{
	// Ensure that this is an MD5 model node
	model::ModelNodePtr model = Node_getModel(node);
	
	if (!model)
	{
		globalErrorStream() << "AnimationPreview::setModelNode: node is not a model." << std::endl;
		_model.reset();
		return;
	}

	// Set up the scene
	if (!_entity)
	{
		setupSceneGraph();
	}

	try
	{
		dynamic_cast<const md5::IMD5Model&>(model->getIModel());
	}
	catch (std::bad_cast&)
	{
		globalErrorStream() << "AnimationPreview::setModelNode: modelnode doesn't contain an MD5 model." << std::endl;
		_model.reset();
		return;
	}

	if (_model)
	{
		_entity->removeChildNode(_model);
	}

	_model = node;

	// Set the animation to play
	dynamic_cast<md5::IMD5Model&>(model->getIModel()).setAnim(_anim);

	// AddChildNode also tells the model which renderentity it is attached to
	_entity->addChildNode(_model);

	if (_model != NULL)
	{
		// Reset preview time
		stopPlayback();

		// Reset the rotation to the default one
		_rotation = Matrix4::getRotation(Vector3(0,-1,0), Vector3(0,-0.3f,1));
		_rotation.multiplyBy(Matrix4::getRotation(Vector3(0,1,0), Vector3(1,-1,0)));
		
		// Use AABB to adjust camera distance
		const AABB& bounds = _model->localAABB();

		if (bounds.isValid())
		{
			_camDist = -5.0f * static_cast<float>(bounds.getRadius());
		}
		else
		{
			// Bounds not valid, fall back to default
			_camDist = -40.0f;
		}

		// Start playback when switching particles
		startPlayback();
	}
	else
	{
		stopPlayback();
	}

	// Redraw
	queueDraw();
}

AABB AnimationPreview::getSceneBounds()
{
	if (!_model) return RenderPreview::getSceneBounds();

	return _model->localAABB();
}

bool AnimationPreview::onPreRender()
{
	if (!_model) return false;

	// Set the animation to play
	model::ModelNodePtr model = Node_getModel(_model);
	dynamic_cast<md5::IMD5Model&>(model->getIModel()).updateAnim(_renderSystem->getTime());

	return true;
}

RenderStateFlags AnimationPreview::getRenderFlagsFill()
{
	return RenderPreview::getRenderFlagsFill() | RENDER_DEPTHWRITE | RENDER_DEPTHTEST;
}

void AnimationPreview::setAnim(const md5::IMD5AnimPtr& anim)
{
	_anim = anim;

	if (!_model)
	{
		return;
	}

	// Set the animation to play
	model::ModelNodePtr model = Node_getModel(_model);
	dynamic_cast<md5::IMD5Model&>(model->getIModel()).setAnim(_anim);
}

void AnimationPreview::setupSceneGraph()
{
	RenderPreview::setupSceneGraph();

	_entity = GlobalEntityCreator().createEntity(
		GlobalEntityClassManager().findClass(FUNC_STATIC_CLASS)
    );

	// This entity is acting as our root node in the scene
	getScene()->setRoot(_entity);
}

} // namespace
