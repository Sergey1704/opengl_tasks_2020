#include "SkinnedMesh.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <algorithm>
#include <iostream>

#include "Utils.h"

void Skeleton::registerMeshBones(const aiMesh &assimpMesh) {
	for (unsigned int i = 0; i < assimpMesh.mNumBones; i++) {
		const aiBone &bone = *assimpMesh.mBones[i];

		std::string boneName(bone.mName.data);
		auto iter = boneNameToIndex.find(boneName);
		if (iter == boneNameToIndex.cend()) {
			// New bone.
			int index = bones.size();
			boneNameToIndex.insert({boneName, index});

			glm::mat4 boneOffset = Utils::aiMatrixToGLM(bone.mOffsetMatrix);

			bones.emplace_back(boneOffset);
		}
	}

	if (boneTransforms.size() < bones.size()) {
		boneTransforms.resize(bones.size());
	}
}

template <typename Frame>
unsigned int findAnimationRightKey(unsigned int framesCount, const Frame *array, float animTime) {
	if (array[framesCount-1].mTime <= animTime)
		return framesCount-1;

	for (unsigned int i = 0; i < framesCount; i++) {
		if (array[i].mTime > animTime)
			return i;
	}
	// Should not reach here.
	return framesCount-1;
}

float getLerpCoeff(float animTime, float leftTime, float rightTime) {
	return glm::clamp((animTime - leftTime) / (rightTime - leftTime), 0.0f, 1.0f);
}

glm::mat4 countNodeLocalTransform(const aiNodeAnim &animation, float animationTime) {
	unsigned int posKey = findAnimationRightKey(animation.mNumPositionKeys, animation.mPositionKeys, animationTime);
	unsigned int rotKey = findAnimationRightKey(animation.mNumRotationKeys, animation.mRotationKeys, animationTime);
	unsigned int sclKey = findAnimationRightKey(animation.mNumScalingKeys, animation.mScalingKeys, animationTime);

	// Interpolate position.
	glm::vec3 pos;
	if (posKey == 0) {
		pos = Utils::aiVecToGLM(animation.mPositionKeys[0].mValue);
	}
	else {
		glm::vec3 left = Utils::aiVecToGLM(animation.mPositionKeys[posKey - 1].mValue);
		glm::vec3 right = Utils::aiVecToGLM(animation.mPositionKeys[posKey].mValue);
		float time = getLerpCoeff(animationTime, animation.mPositionKeys[posKey - 1].mTime, animation.mPositionKeys[posKey].mTime);
		pos = glm::mix(left, right, time);
	}

	// Interpolate rotation.
	glm::quat rot;
	if (rotKey == 0) {
		rot = Utils::aiQuatToGLM(animation.mRotationKeys[0].mValue);
	}
	else {
		glm::quat left = Utils::aiQuatToGLM(animation.mRotationKeys[rotKey - 1].mValue);
		glm::quat right = Utils::aiQuatToGLM(animation.mRotationKeys[rotKey].mValue);
		float time = getLerpCoeff(animationTime, animation.mRotationKeys[rotKey-1].mTime, animation.mRotationKeys[rotKey].mTime);
		rot = glm::slerp(left, right, time);
	}

	// Interpolate scale. Well, actually hierarchical scaling can lead to something looking really strange but we do it :)
	glm::vec3 scale;
	if (sclKey == 0) {
		scale = Utils::aiVecToGLM(animation.mScalingKeys[0].mValue);
	}
	else {
		glm::vec3 left = Utils::aiVecToGLM(animation.mScalingKeys[sclKey - 1].mValue);
		glm::vec3 right = Utils::aiVecToGLM(animation.mScalingKeys[sclKey].mValue);
		float time = getLerpCoeff(animationTime, animation.mScalingKeys[sclKey - 1].mTime, animation.mScalingKeys[sclKey].mTime);
		scale = glm::mix(left, right, time);
	}

	return glm::translate(glm::mat4(1), pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1), scale);
}

void Skeleton::animateLocalTransforms(const aiNodeAnim &animation, float animationTime) {
	glm::mat4 nodeLocalTransform = countNodeLocalTransform(animation, animationTime);
	animatedNodeLocalTransforms[animation.mNodeName.data] = nodeLocalTransform;
}

void Skeleton::recountGlobalTransforms(const aiNode &node, glm::mat4 parentMatrix) {
	glm::mat4 nodeLocalMatrix;
	auto iter = animatedNodeLocalTransforms.find(node.mName.data);
	if (iter == animatedNodeLocalTransforms.cend()) {
		// Node is not animated.
		nodeLocalMatrix = Utils::aiMatrixToGLM(node.mTransformation);
	}
	else {
		nodeLocalMatrix = iter->second;
	}

	glm::mat4 nodeGlobalMatrix = parentMatrix * nodeLocalMatrix;

	// If associated bone is present, record transform for it.
	int bone = findBone(node.mName.data);
	if (bone >= 0) {
		boneTransforms[bone] = nodeGlobalMatrix * getBoneInfo(bone).offsetMatrix;
	}

	for (unsigned int child = 0; child < node.mNumChildren; child++)
		recountGlobalTransforms(*node.mChildren[child], nodeGlobalMatrix);
}

void Skeleton::recountGlobalTransforms(const aiNode &rootNode) {
	recountGlobalTransforms(rootNode, glm::mat4(1));
}

int Skeleton::findBone(const std::string &name) {
	auto iter = boneNameToIndex.find(name);
	if (iter == boneNameToIndex.cend())
		return -1;
	return iter->second;
}

void SkinnedMesh::setupSkeleton(const aiMesh &assimpMesh) {
	verticesBones.clear();
	verticesBones.reserve(assimpMesh.mNumVertices);
	skeleton->registerMeshBones(assimpMesh);

	for (unsigned int ibone = 0; ibone < assimpMesh.mNumBones; ibone++) {
		const aiBone &bone = *assimpMesh.mBones[ibone];
		int boneIndex = skeleton->findBone(std::string(bone.mName.data));
		assert(boneIndex >= 0);

		for (unsigned int iweight = 0; iweight < bone.mNumWeights; iweight++) {
			aiVertexWeight weightInfo = bone.mWeights[iweight];

			if (weightInfo.mVertexId >= verticesBones.size())
				verticesBones.resize(weightInfo.mVertexId + 1);
			verticesBones[weightInfo.mVertexId].emplace_back(boneIndex, weightInfo.mWeight);
		}
	}
}

void SkinnedMesh::setBoneAttributes4(unsigned int idsLocation, unsigned int weightLocation) {
	// Prepare reduced data.
	std::vector<Vertex4BoneData> vbonesData(verticesBones.size());

	// Just for fun: number of vertices having #index bones influencing them.
	int stats[5] = { 0, 0, 0, 0, 0 };

	for (unsigned int i = 0; i < verticesBones.size(); i++) {
		std::sort(verticesBones[i].begin(), verticesBones[i].end(), [&](const VertexSingleBoneInfo &a, const VertexSingleBoneInfo &b) {
			return a.boneWeight > b.boneWeight;
		});
		vbonesData[i].ids = glm::uvec4(0);
		vbonesData[i].weights = glm::vec4(0);
		float sumWeight = 0.0f;
		for (size_t j = 0; j < std::min(static_cast<size_t>(4), verticesBones[i].size()); j++) {
			vbonesData[i].ids[j] = verticesBones[i][j].boneId;
			vbonesData[i].weights[j] = verticesBones[i][j].boneWeight;
			sumWeight += verticesBones[i][j].boneWeight;
		}
		assert(sumWeight > 0);
		assert(vbonesData[i].weights.x >= vbonesData[i].weights.y);
		vbonesData[i].weights /= sumWeight;
		stats[std::min(static_cast<size_t>(4), verticesBones[i].size())]++;
	}

	std::cout << "Bone number distribution: " << "0 - " << stats[0] << ", 1 - " << stats[1] << ", 2 - " << stats[2] << ", 3 - " << stats[3] << ", 4 - " << stats[4] << std::endl;

	DataBufferPtr buffer = std::make_shared<DataBuffer>();
	buffer->setData(vbonesData.size() * sizeof(Vertex4BoneData), vbonesData.data());

	mesh->setAttributeI(idsLocation, 4, GL_UNSIGNED_INT, sizeof(glm::vec4) + sizeof(glm::uvec4), 0, buffer);
	mesh->setAttribute(weightLocation, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) + sizeof(glm::uvec4), sizeof(glm::uvec4), buffer);
}
