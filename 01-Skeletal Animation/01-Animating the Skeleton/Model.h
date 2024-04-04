
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "logger.h"
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "Animdata.h"
#include "AssimpGLMHelpers.h"
using namespace glm;

class Model{
public:


vector<Texture> textures_loaded;	
vector<Mesh>    meshes = {};
string directory;
const aiScene* scene;
aiMesh* mesh;

std::map<string, BoneInfo> m_BoneInfoMap;
int m_BoneCounter = 0;

auto& GetBoneInfoMap() { return m_BoneInfoMap; }
int& GetBoneCount() { return m_BoneCounter; }

void SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}

void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
	{
        Logger logger("SetVertexBoneData.log");
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			if (vertex.m_BoneIDs[i] < 0)
			{
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
          
				break;
			}
		}
        logger.debug("weight",weight);
        logger.debug("boneID",boneID);
	}

void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
        Logger logger("ExtractBoneWeightForVertices.log");

		auto& boneInfoMap = m_BoneInfoMap;
		int& boneCount = m_BoneCounter;

		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = boneCount;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				boneInfoMap[boneName] = newBoneInfo;
				boneID = boneCount;
				boneCount++;
			}
			else
			{
				boneID = boneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
                if (weight == 0.0f)
                    continue;
				assert(vertexId <= vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}

// Function to load a texture image and create a texture object
GLuint loadTextureEmbedded(void* pData, int bufferSize) {
    Logger logger("loadTextureEmbedded.Log");

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load texture image and generate mipmaps
    int width, height, channels;
    unsigned char* data = stbi_load_from_memory((const stbi_uc*)pData, bufferSize,  &width, &height, &channels, 0);
    if (data) {
        GLenum format = channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
		 logger.debug("Failed to load texture");
    }
    logger.debug("width",width);
    logger.debug("height",height);
    logger.debug("channels",channels);
    logger.debug("textureID",textureID);
    stbi_image_free(data);

    return textureID;
}

unsigned int TextureFromFile(const char *path, const string &directory)
{
    Logger logger("Loading texture.Log");
    string filename = string(path); 
    filename = directory + '/' + filename;
    logger.debug("Filename: ",filename);
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

 vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        Logger logger("loadMaterialTextures.Log");
        vector<Texture> textures;
      
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
                if (embeddedTexture) {
                aiTexture* texturea = scene->mTextures[mesh->mMaterialIndex];
                // Print some information about the embedded texture
                texture.id  = loadTextureEmbedded(embeddedTexture->pcData, embeddedTexture->mWidth);
                logger.debug("Embedded texture info:",str.C_Str() );
                logger.debug("Width",texturea->mWidth );
                logger.debug("Height",texturea->mHeight  );
                logger.debug("Format",texturea->achFormatHint );
                }
                else{
                    logger.debug("TextureFromFile",str.C_Str() );
                    texture.id = TextureFromFile(str.C_Str(), directory);
                }
                
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }

Mesh processMesh(aiMesh *mesh, const aiScene *scene)
     {
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			SetVertexBoneDataToDefault(vertex);
			vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
			vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
			
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		ExtractBoneWeightForVertices(vertices,mesh,scene);

		return Mesh(vertices, indices, textures);
	}


 // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void processNode(aiNode *node, const aiScene *scene)
    {
		Logger logger("processNode.log");
		logger.debug("processNode");
		logger.debug("node->mNumMeshes",node->mNumMeshes);
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }


 void loadModel(string const &path)
    {
		Logger logger("LoadModel.log");
		logger.debug("logger");
        // read file via ASSIMP
		logger.debug("path",path);
        Assimp::Importer importer;
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            return;
        }

        size_t lastSeparator = path.find_last_of("\\/");
    
        // Extract the directory path (excluding the separator)
        directory = path.substr(0, lastSeparator);
        // retrieve the directory path of the filepath
       // directory = path.substr(0, path.find_last_of('/'));
		logger.debug("directory",directory);


        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }


void draw()
{
    	for(unsigned int i = 0; i < meshes.size(); i++)
	{
		glBindVertexArray(meshes[i].VAO);
		
			glDrawElements(GL_TRIANGLES, (meshes[i].indices.size()), GL_UNSIGNED_INT, 0);
		
		glBindVertexArray(0);

	}
}

void printMeshDetails(const std::vector<Mesh>& meshes) {
    std::stringstream ss;
    Logger logger("printMeshDetails.log");
    for (size_t i = 0; i < meshes.size(); ++i) {
        ss << "Mesh " << i + 1 << ":" << std::endl;
        ss << "Vertices:" << std::endl;
        for (const auto& vertex : meshes[i].vertices) {
            ss << "Position: (" << vertex.Position[0] << ", " << vertex.Position[1] << ", " << vertex.Position[2]<< ")" << std::endl;
            ss << "Normal: (" << vertex.Normal[0] << ", " << vertex.Normal[1] << ", " << vertex.Normal[2] << ")" << std::endl;
            ss << "TexCoords: (" << vertex.TexCoords[0] << ", " << vertex.TexCoords[1] << ")" << std::endl;
            ss << "Tangent: (" << vertex.Tangent[0] << ", " << vertex.Tangent[1] << ", " << vertex.Tangent[2] << ")" << std::endl;
            ss << "Bitangent: (" << vertex.Bitangent[0] << ", " << vertex.Bitangent[1] << ", " << vertex.Bitangent[2] << ")" << std::endl;
            ss << "Bone IDs: ";
            for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
                ss << vertex.m_BoneIDs[j] << " ";
            }
            ss << std::endl;
            ss << "Weights: ";
            for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
                ss << vertex.m_Weights[j] << " ";
            }
            ss << std::endl << std::endl;
        }

        ss << "Indices:" << std::endl;
        for (const auto& index : meshes[i].indices) {
            ss << index << " ";
        }
        ss << std::endl << std::endl;

        ss << "Textures:" << std::endl;
        for (const auto& texture : meshes[i].textures) {
            ss << "ID: " << texture.id << ", Type: " << texture.type << ", Path: " << texture.path << std::endl;
        }
        ss << std::endl;
    }

    logger.debug("Log:",ss.str());
}
};