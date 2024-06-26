#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Bone.h"
#include <functional>
#include "Animdata.h" 
#include "Model.h"

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation
{
public:
	Animation() = default;
	std::vector<std::string> animationNames;

	Animation(const std::string& animationPath, Model* model,int animate=0)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		int numAnimations = scene->mNumAnimations;
		if (animate >= numAnimations) {
			animate = 0; // Default to the first animation
		}
		auto animation = scene->mAnimations[animate];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadHierarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, *model);

		for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex) {
			auto animations = scene->mAnimations[animIndex];
			animationNames.push_back(animations->mName.C_Str());
		}
		//printNodeData(GetRootNode());
	}

	~Animation()
	{
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	std::vector<std::string> GetAnimationNames() {
		
		return animationNames;
	}
	
std::vector<Bone> m_Bones;
void printNodeData(const AssimpNodeData& node, int level = 0)
{
	Logger logger("printNodeData.log");
    std::stringstream ss;

    // Print node name and transformation
    ss << "Name: " << node.name;
  //  ss << std::string(level * 4, ' ') << "Transformation: " << std::endl;
  

    // Print children recursively
    for (const auto& child : node.children) {
        printNodeData(child,  level + 1);
    }

    // Log the formatted message
    logger.debug("printNodeData",ss.str());
}
	
	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration;}
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
	inline const std::map<std::string,BoneInfo>& GetBoneIDMap() 
	{ 
		return m_BoneInfoMap;
	}
	void BonesToString(const std::vector<Bone>& bones) {
		Logger logger("Bones.Log");
		std::stringstream ss;
		for (const auto& bone : bones) {
			ss << "Bone Name: " << bone.GetBoneName() <<"\n" ;
			//ss << "Bone ID: " << bone.GetBoneID() << "\n";
			// You can add more information if needed
		}
		logger.debug("Bones", ss.str());
	}

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model)
	{
		int size = animation->mNumChannels;

		auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}



	float m_Duration;
	int m_TicksPerSecond;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};
