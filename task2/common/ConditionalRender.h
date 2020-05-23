#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

#include <string>
#include <vector>
#include <memory>
#include <queue>
#include "Common.h"

class QueryObject;

class ConditionalRender {
public:
	enum WaitMode {
		WM_QUERY_WAIT = GL_QUERY_WAIT,
		WM_QUERY_NO_WAIT = GL_QUERY_NO_WAIT,
		WM_QUERY_BY_REGION_WAIT = GL_QUERY_BY_REGION_WAIT,
		WM_QUERY_BY_REGION_NO_WAIT = GL_QUERY_BY_REGION_NO_WAIT,

		// GL_ARB_conditional_render_inverted:
		WM_QUERY_WAIT_INVERTED = GL_QUERY_WAIT_INVERTED,
		WM_QUERY_NO_WAIT_INVERTED = GL_QUERY_NO_WAIT_INVERTED,
		WM_QUERY_BY_REGION_WAIT_INVERTED = GL_QUERY_BY_REGION_WAIT_INVERTED,
		WM_QUERY_BY_REGION_NO_WAIT_INVERTED = GL_QUERY_BY_REGION_NO_WAIT_INVERTED
	};

	void beginConditionalRender(const QueryObject &_query, WaitMode _mode);
	void endConditionalRender();

protected:
	bool renderBegan = false;
};
