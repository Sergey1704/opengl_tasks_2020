#include "QueryObject.h"

#include "ConditionalRender.h"

void QueryObject::beginQuery(Target _target, GLuint _index) {
	assert(!queryBegan);
	glBeginQueryIndexed(_target, _index, queryId);
	queryBegan = true;
	target = _target;
	index = _index;
}

void QueryObject::endQuery() {
	assert(queryBegan);
	glEndQueryIndexed(target, index);
	queryBegan = false;
}

ConditionalRender* QueryObject::beginConditionalRender(ConditionalRender::WaitMode waitMode) {
	assert(!queryBegan);
	if (!conditionalRender) {
		conditionalRender = std::unique_ptr<ConditionalRender>(new ConditionalRender());
	}
	conditionalRender->beginConditionalRender(*this, waitMode);
	return conditionalRender.get();
}

bool QueryObject::isResultAvailable() const {
	GLuint64 ready;
	glGetQueryObjectui64v(queryId, GL_QUERY_RESULT_AVAILABLE, &ready);
	return ready;
}

GLuint64 QueryObject::getResultSync() const {
	// If result is not available, synchronization will occur.
	GLuint64 result;
	glGetQueryObjectui64v(queryId, GL_QUERY_RESULT, &result);
	return result;
}

GLuint64 QueryObject::getResultAsync(GLuint64 defaultValue) const {
	GLuint64 result = defaultValue;
	glGetQueryObjectui64v(queryId, GL_QUERY_RESULT_NO_WAIT, &result);
	return result;
}

QueryObjectPtr QueryManager::beginQuery(GLuint index) {
	QueryObjectPtr query;

	if (unissuedQueries.empty()) {
		if (pendingQueries.size() < maxPendingQueries) {
			query = std::make_shared<QueryObject>();
		}
		else {
			stats.queriesLost++;
			return nullptr;
		}
	}
	else {
		query = unissuedQueries.front();
		unissuedQueries.pop();
	}

	query->beginQuery(target, index);

	pendingQueries.push(query);
	stats.queriesBegan++;
	return query;
}

void QueryManager::processFinishedQueries() {
	// Well, generally queries are async and their order of finishing is not specified.
	// But for simplicity we just check the oldest query in the queue.
	while (!pendingQueries.empty() && pendingQueries.front()->isResultAvailable()) {
		auto query = pendingQueries.front();
		pendingQueries.pop();
		// If we had not ensured the query result is available sync wait could be activated when fetching results!
		if (handler)
			handler(query);
		unissuedQueries.push(query);
		stats.queriesHandled++;
	}
}
