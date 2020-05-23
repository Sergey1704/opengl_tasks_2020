#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <queue>

#include "Common.h"
#include "ConditionalRender.h"

class QueryObject {
public:
	// Enumeration of occlusion query targets that can be used in glBeginQuery function.
	enum Target {
		QOT_SAMPLES_PASSED = GL_SAMPLES_PASSED,
		QOT_ANY_SAMPLES_PASSED = GL_ANY_SAMPLES_PASSED,
		QOT_ANY_SAMPLES_PASSED_CONSERVATIVE = GL_ANY_SAMPLES_PASSED_CONSERVATIVE,
		QOT_PRIMITIVES_GENERATED = GL_PRIMITIVES_GENERATED,
		QOT_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN = GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
		QOT_TIME_ELAPSED = GL_TIME_ELAPSED
	};

	QueryObject() {
		glGenQueries(1, &queryId);
	}
	~QueryObject() {
		glDeleteQueries(1, &queryId);
	}

	void beginQuery(Target _target, GLuint _index = 0);
	void endQuery();

	ConditionalRender* beginConditionalRender(ConditionalRender::WaitMode waitMode);

	bool isResultAvailable() const;
	GLuint64 getResultSync() const;
	GLuint64 getResultAsync(GLuint64 defaultValue) const;

	GLuint getId() const { return queryId; }
protected:
	bool queryBegan = false;
	GLuint queryId;
	GLuint index;
	GLenum target;

	std::unique_ptr<ConditionalRender> conditionalRender;
};

typedef std::shared_ptr<QueryObject> QueryObjectPtr;

class QueryManager {
public:
	struct Stats {
		size_t queriesBegan = 0;
		size_t queriesHandled = 0;
		size_t queriesLost = 0;
	};

	typedef std::function<void (QueryObjectPtr)> QueryResultHandler;

	QueryManager(QueryObject::Target _target) : target(_target), maxPendingQueries(1) { }

	void processFinishedQueries();

	void setQueryResultHandler(const QueryResultHandler &_handler) { handler = _handler; }

	void setMaxPendingQueries(GLuint _maxPending) {
		assert(unissuedQueries.size() + pendingQueries.size() < _maxPending); // Decrease just is not handled here :)
		maxPendingQueries = _maxPending;
	}

	/**
	 * Takes a free query object or generates a new one if available according to limitations and begins an occlusion query.
	 * @param index non-zero only for indexed targets.
	 * @return pointer to occlusion query or null if limit reached.
	 */
	QueryObjectPtr beginQuery(GLuint index = 0);

	const Stats &getStats() const { return stats; }
	void clearStats() { stats = Stats(); }
protected:
	QueryObject::Target target;

	GLuint maxPendingQueries;

	std::queue<QueryObjectPtr> unissuedQueries;
	std::queue<QueryObjectPtr> pendingQueries;

	QueryResultHandler handler;

	Stats stats;
};

