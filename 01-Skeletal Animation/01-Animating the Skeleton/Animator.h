#pragma once
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "Animation.h"
#include "Bone.h"
class Animator
{
private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
Animation*	m_CurrentAnimation	{ nullptr };
	float m_CurrentTime;
	float m_DeltaTime;

public:
	Animator() = default;
	Animator(Animation* animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(200);

		for (int i = 0; i < 200; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
		m_CurrentAnimation->printNodeData(m_CurrentAnimation->GetRootNode());
		m_CurrentAnimation->BonesToString(m_CurrentAnimation->m_Bones);
	}

	 void printFinalMatrices()
    {
		Logger logger("printFinalMatrices.log");
        std::stringstream ss;
        for (size_t i = 0; i < m_FinalBoneMatrices.size(); ++i)
        {
            ss << "Bone " << i << " Matrix:" << std::endl;
            const glm::mat4& matrix = m_FinalBoneMatrices[i];
            for (size_t row = 0; row < 4; ++row)
            {
                for (size_t col = 0; col < 4; ++col)
                {
                    ss << matrix[col][row] << "\t";
                }
                ss << std::endl;
            }
            ss << std::endl;
        }

        // Log the formatted message
        logger.debug("FinalBoneMatrices", ss.str());
    }

	void UpdateAnimation(float dt,Animation* m_CurrentAnimation)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	/*void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}*/
void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    Logger logger("Calculatingbonetransform.log");

    if (!node) {
        // Error: Invalid node pointer
        logger.debug("Error: Invalid node pointer");
        return;
    }

    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;
    glm::mat4 globalTransformation; // Declare globalTransformation here

    Bone* bone = m_CurrentAnimation->FindBone(nodeName);

    if (!bone) {
        // Error: Bone not found for the given node
        logger.debug("Error: Bone not found for node ", nodeName);
        // Continue to the next node
    } else {
        // Bone found, proceed with calculation
        bone->Update(m_CurrentTime);
        nodeTransform = bone->GetLocalTransform();

        globalTransformation = parentTransform * nodeTransform; // Initialize globalTransformation

        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }
        else
        {
            // Error: Bone info not found in the bone ID map
            logger.debug("Error: Bone info not found in the bone ID map for node ", nodeName);
            // Continue to the next node
        }
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

    void Animator::BlendTwoAnimations(Animation* pBaseAnimation, Animation* pLayeredAnimation, float blendFactor, float deltaTime, float& currentTimeBase, float& currentTimeLayered)
    {
        // Speed multipliers to correctly transition from one animation to another
        float a = 1.0f;
        float b = pBaseAnimation->GetDuration() / pLayeredAnimation->GetDuration();
        const float animSpeedMultiplierUp = (1.0f - blendFactor) * a + b * blendFactor; // Lerp

        a = pLayeredAnimation->GetDuration() / pBaseAnimation->GetDuration();
        b = 1.0f;
        const float animSpeedMultiplierDown = (1.0f - blendFactor) * a + b * blendFactor; // Lerp

        // Update the accumulated times
        static float accumulatedTimeBase = 0.0f;
        accumulatedTimeBase += pBaseAnimation->GetTicksPerSecond() * deltaTime * animSpeedMultiplierUp;

        static float accumulatedTimeLayered = 0.0f;
        accumulatedTimeLayered += pLayeredAnimation->GetTicksPerSecond() * deltaTime * animSpeedMultiplierDown;

        // Calculate the current animation times
        currentTimeBase = fmod(accumulatedTimeBase, pBaseAnimation->GetDuration());
        currentTimeLayered = fmod(accumulatedTimeLayered, pLayeredAnimation->GetDuration());

        CalculateBlendedBoneTransform(pBaseAnimation, &pBaseAnimation->GetRootNode(), pLayeredAnimation, &pLayeredAnimation->GetRootNode(), currentTimeBase, currentTimeLayered, glm::mat4(1.0f), blendFactor);
    }


    // Recursive function that sets interpolated bone matrices in the 'm_FinalBoneMatrices' vector
    void Animator::CalculateBlendedBoneTransform(
        Animation* pAnimationBase, const AssimpNodeData* node,
        Animation* pAnimationLayer, const AssimpNodeData* nodeLayered,
        const float currentTimeBase, const float currentTimeLayered,
        const glm::mat4& parentTransform,
        const float blendFactor)
    {
        const std::string& nodeName = node->name;

        glm::mat4 nodeTransform = node->transformation;
        Bone* pBone = pAnimationBase->FindBone(nodeName);
        if (pBone)
        {
            pBone->Update(currentTimeBase);
            nodeTransform = pBone->GetLocalTransform();
        }

        glm::mat4 layeredNodeTransform = nodeLayered->transformation;
        pBone = pAnimationLayer->FindBone(nodeName);
        if (pBone)
        {
            pBone->Update(currentTimeLayered);
            layeredNodeTransform = pBone->GetLocalTransform();
        }

        // Blend two matrices
        const glm::quat rot0 = glm::quat_cast(nodeTransform);
        const glm::quat rot1 = glm::quat_cast(layeredNodeTransform);
        const glm::quat finalRot = glm::slerp(rot0, rot1, blendFactor);
        glm::mat4 blendedMat = glm::mat4_cast(finalRot);
        blendedMat[3] = (1.0f - blendFactor) * nodeTransform[3] + layeredNodeTransform[3] * blendFactor;

        glm::mat4 globalTransformation = parentTransform * blendedMat;

        const auto& boneInfoMap = pAnimationBase->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            const int index = boneInfoMap.at(nodeName).id;
            const glm::mat4& offset = boneInfoMap.at(nodeName).offset;

            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (size_t i = 0; i < node->children.size(); ++i)
            CalculateBlendedBoneTransform(pAnimationBase, &node->children[i], pAnimationLayer, &nodeLayered->children[i], currentTimeBase, currentTimeLayered, globalTransformation, blendFactor);
    }



};