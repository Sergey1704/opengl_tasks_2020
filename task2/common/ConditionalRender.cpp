#include "ConditionalRender.h"

#include "QueryObject.h"

void ConditionalRender::beginConditionalRender(const QueryObject &_query, WaitMode _mode) {
	assert(!renderBegan);
	glBeginConditionalRender(_query.getId(), _mode);
	renderBegan = true;
}

void ConditionalRender::endConditionalRender() {
	assert(renderBegan);
	glEndConditionalRender();
	renderBegan = false;
}
