#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>

class Utils {
public:
	Utils() = delete;

	static_assert(sizeof(glm::mat4) == sizeof(aiMatrix4x4), "glm::mat4 has different size from aiMatrix4x4");

	static glm::mat4 aiMatrixToGLM(const aiMatrix4x4 &m) {
		return glm::transpose(*reinterpret_cast<const glm::mat4*>(&m));
	}

	static glm::quat aiQuatToGLM(aiQuaternion q) {
		// ASSIMP quat stores wxyz, GLM quat stores xyzw, but glm quat constructor receives wxyz.
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	static glm::vec3 aiVecToGLM(aiVector3D v) {
		return glm::vec3(v.x, v.y, v.z);
	}
};



