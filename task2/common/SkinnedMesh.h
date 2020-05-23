#pragma once

#include "Mesh.hpp"

#include <unordered_map>
#include <vector>

class aiMesh;
class aiNode;
class aiNodeAnim;

/**
 * A collection of bones generally separated from bone hierarchy.
 * Describes bone transformations.
 */
class Skeleton {
public:
	struct BoneInfo {
		// Offset matrix generally is inverse node global matrix (in modelspace) for static pose.
		glm::mat4 offsetMatrix;

		BoneInfo(glm::mat4 _offsetMatrix) : offsetMatrix(_offsetMatrix) { }
	};

	/**
	 * Each mesh in assimp scene has array of pointers to bones influencing them.
	 * Actually these bones are nodes in the scene hierarchy.
	 * We just expand our array of bones with those going with the given mesh.
	 */
	void registerMeshBones(const aiMesh &assimpMesh);

	/**
	 * Get bone ID by node name.
	 */
	int findBone(const std::string &name);

	BoneInfo getBoneInfo(unsigned int index) const { return bones[index]; }

	glm::mat4 &operator[](unsigned int index) { return boneTransforms[index]; }
	const glm::mat4 &operator[](unsigned int index) const { return boneTransforms[index]; }

	/**
	 * Interpolate bone local transformation based on animation and animation time.
	 */
	void animateLocalTransforms(const aiNodeAnim &animation, float animationTime);

	/**
	 * Use hierarchy to deduce nodes global transforms (in modelspace).
	 */
	void recountGlobalTransforms(const aiNode &rootNode);

	// Bone transformations ready to be sent to shader.
	std::vector<glm::mat4> boneTransforms;

protected:
	void recountGlobalTransforms(const aiNode &node, glm::mat4 parentMatrix);

	// Node name -> local transformation.
	// Not only bone nodes can be animated. Actually a bone is a node that influences at least one vertex :)
	std::unordered_map<std::string, glm::mat4> animatedNodeLocalTransforms;
	// Bone offset matrices.
	std::vector<BoneInfo> bones;
	// Mapping between node names and bone indices.
	std::unordered_map<std::string, int> boneNameToIndex;
};

typedef std::shared_ptr<Skeleton> SkeletonPtr;

/**
 * Skinned mesh is a combination of usual mesh with vertex-bone mappings and a skeleton.
 */
class SkinnedMesh {
public:
	/**
	 * In shader we limit animation quality to 4 bones per vertex.
	 */
	struct Vertex4BoneData {
		glm::uvec4 ids;
		glm::vec4 weights;
	};

	/**
	 * For accumulating per-vertex bone influence.
	 */
	struct VertexSingleBoneInfo {
		unsigned int boneId;
		float boneWeight;

		VertexSingleBoneInfo(unsigned int _boneId, float _boneWeight) : boneId(_boneId), boneWeight(_boneWeight) { }
	};

	MeshPtr getMesh() const { return mesh; }
	SkeletonPtr getSkeleton() const { return skeleton; }

	void setMesh(const MeshPtr &_mesh) { mesh = _mesh; }
	void setSkeleton(const SkeletonPtr &_skeleton) { skeleton = _skeleton; verticesBones.clear(); }

	/**
	 * Deduce verticesBones array.
	 */
	void setupSkeleton(const aiMesh &assimpMesh);

	/**
	 * Chooses most influensive bones for each vertex, upload them to GPU and register attributes for mesh.
	 * @param idsLocation location in shader for uvec4 attribute VERTEX BONE ID
	 * @param weightLocation location in shader for vec4 attribute VERTEX BONE WEIGHT
	 */
	void setBoneAttributes4(unsigned int idsLocation, unsigned int weightLocation);

protected:
	MeshPtr mesh;
	SkeletonPtr skeleton = std::make_shared<Skeleton>();
	std::vector<std::vector<VertexSingleBoneInfo>> verticesBones;
};

typedef std::shared_ptr<SkinnedMesh> SkinnedMeshPtr;
