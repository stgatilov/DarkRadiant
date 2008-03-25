#ifndef UPDATENODEVISIBILITYWALKER_H_
#define UPDATENODEVISIBILITYWALKER_H_

#include <stack>
#include "ilayer.h"
#include "iscenegraph.h"
#include "scenelib.h"

namespace scene {

class UpdateNodeVisibilityWalker :
	public scene::NodeVisitor
{
	std::stack<bool> _visibilityStack;
public:
	bool pre(const INodePtr& node) {
		// Update the node visibility and store the result
		bool nodeIsVisible = GlobalLayerSystem().updateNodeVisibility(node);

		// Add a new element for this level
		_visibilityStack.push(nodeIsVisible);

		return true;
	}

	void post(const INodePtr& node) {
		// Is this child visible?
		bool childIsVisible = _visibilityStack.top();

		_visibilityStack.pop();

		if (childIsVisible) {
			// Show the node, regardless whether it was hidden before
			// otherwise the parent would hide the visible children as well
			node->disable(Node::eLayered);
		}

		if (!node->visible()) {
			// Node is hidden after update (and no children are visible), de-select
			Node_setSelected(node, false);
		}

		if (childIsVisible && !_visibilityStack.empty()) {
			// The child was visible, set this parent to true
			_visibilityStack.top() = true;
		}
	}
};

} // namespace scene

#endif /* UPDATENODEVISIBILITYWALKER_H_ */
