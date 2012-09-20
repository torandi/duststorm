#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "platform.h"
#include "render_object.hpp"
#include "engine.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "data.hpp"
#include "material.hpp"

#include <string>
#include <cstdio>

#include <assimp/postprocess.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* Classes for imports with Data class */

class AssimpDataStream : public Assimp::IOStream {
	private:
		Data * data_;

	public:
		AssimpDataStream(Data * data) : data_(data) {};

		virtual 	~AssimpDataStream () {
			delete data_;
		}

		virtual size_t FileSize () const  {
			return data_->size();
		}

		virtual void Flush () {
			//Writing not implemented
		}

		virtual size_t Read (void *pvBuffer, size_t pSize, size_t pCount) {
			return data_->read(pvBuffer, pSize, pCount);
		}

		virtual aiReturn Seek (size_t pOffset, aiOrigin pOrigin) {
			return (aiReturn) data_->seek( pOffset, (int)pOrigin);
		}

		virtual size_t Tell () const  {
			return data_->tell();
		}

		virtual size_t Write (const void *pvBuffer, size_t pSize, size_t pCount) {
			fprintf(stderr, "Writing of models is not implemented\n");
			abort();
		}
};

class AssimpDataImport : public Assimp::IOSystem {
	public:
		virtual void Close (Assimp::IOStream *pFile) {
			delete pFile;
		}

		virtual bool Exists (const char *pFile) const {
			return file_exists(PATH_BASE "/data/models/" + std::string(pFile));
		}

		virtual char getOsSeparator () const {
			return __PATH_SEPARATOR_;
		}

		virtual Assimp::IOStream * Open (const char *pFile, const char *pMode="rb") {
			Data * data = Data::open(PATH_BASE "/data/models/" + std::string(pFile));
			if(data) {
				return new AssimpDataStream(data);
			} else {
				fprintf(verbose, "Failed to open file %s\n", pFile);
				return NULL;
			}
		}

		virtual ~AssimpDataImport() {};
};

void RenderObject::color4_to_vec4(const aiColor4D *c, glm::vec4 &target) {
	target.x = c->r;
	target.y = c->g;
	target.z = c->b;
	target.w = c->a;
}

RenderObject::~RenderObject() { }

RenderObject::RenderObject(std::string model, bool normalize_scale, unsigned int aiOptions)
	: MovableObject()
	, normalization_matrix_(1.0f)
	, scene(nullptr)
	, name(model)
	, scale(1.0f) {

	std::string real_path = model;

	importer.SetIOHandler(new AssimpDataImport());

	scene = importer.ReadFile(real_path,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph  |
		aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords |
		aiProcess_ValidateDataStructure | aiProcess_FixInfacingNormals |
		aiProcess_SortByPType |
		aiProcess_CalcTangentSpace | aiOptions
		);

	if ( !scene ) {
		printf("Failed to load model `%s': %s\n", real_path.c_str(), importer.GetErrorString());
		return;
	}

	fprintf(verbose, "Loaded model %s:\n"
	        "  Meshes: %d\n"
	        "  Textures: %d\n"
	        "  Materials: %d\n",
	        model.c_str(), scene->mNumMeshes, scene->mNumTextures, scene->mNumMaterials);

	//Get bounds:
	aiVector3D s_min, s_max;
	get_bounding_box(&s_min, &s_max);
	scene_min = glm::make_vec3((float*)&s_min);
	scene_max = glm::make_vec3((float*)&s_max);
	scene_center  = (scene_min+scene_max)/2.0f;

	//Calculate normalization matrix
	if(normalize_scale) {
		const glm::vec3 size = scene_max - scene_min;
		float tmp = std::max(size.x, size.y);
		tmp = std::max(tmp, size.z);
		normalization_matrix_ = glm::scale(normalization_matrix_, glm::vec3(1.f/tmp));
	}

	pre_render();
}

TextureBase* RenderObject::load_texture(const std::string& path) {
	return Texture2D::from_filename(PATH_BASE "/data/textures/" + path);
}

void RenderObject::pre_render() {

	recursive_pre_render(scene->mRootNode);

	//Init materials:
	for(unsigned int i= 0; i < scene->mNumMaterials; ++i) {
		const aiMaterial * mtl = scene->mMaterials[i];
		Material mtl_data;

		aiString path;
		if(mtl->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
			mtl->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string p(path.data);
			mtl_data.texture = load_texture(p);
		} else if(mtl->GetTextureCount(aiTextureType_AMBIENT) > 0 &&
			mtl->GetTexture(aiTextureType_AMBIENT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string p(path.data);
			mtl_data.texture = load_texture(p);
		}

		if ( !mtl_data.texture ){
			fprintf(stderr, "RenderObject `%s' texture failed to load.\n", name.c_str());
			abort();
		}

		//Check for normalmap:
		if(mtl->GetTextureCount(aiTextureType_HEIGHT) > 0 &&
			mtl->GetTexture(aiTextureType_HEIGHT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			const std::string p(path.data);
			mtl_data.normal_map = load_texture(p);
		}

		if(mtl->GetTextureCount(aiTextureType_SHININESS) > 0 &&
		   mtl->GetTexture(aiTextureType_SHININESS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			const std::string p(path.data);
			mtl_data.specular_map = load_texture(p);
		}

		if(mtl->GetTextureCount(aiTextureType_OPACITY) > 0 &&
		   mtl->GetTexture(aiTextureType_OPACITY, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			const std::string p(path.data);
			mtl_data.alpha_map = load_texture(p);
		}

		aiString name;
		if(AI_SUCCESS == mtl->Get(AI_MATKEY_NAME, name))
			fprintf(verbose, "Loaded material %d %s\n", i, name.data);

		aiColor4D value;
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &value))
			color4_to_vec4(&value, mtl_data.diffuse);

		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &value))
			color4_to_vec4(&value, mtl_data.specular);

		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &value))
			color4_to_vec4(&value, mtl_data.ambient);

		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &value))
			color4_to_vec4(&value, mtl_data.emission);

		unsigned int max = 1;
		float strength;
		int ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &mtl_data.shininess, &max);
		if(ret1 == AI_SUCCESS) {
			max = 1;
			int ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
			if(ret2 == AI_SUCCESS)
				mtl_data.shininess *= strength;
		} else {
			mtl_data.shininess = 0.0f;
		}

		if ( mtl_data.shininess < 0.001f ){ /* arbitrary small value */
			mtl_data.shininess = 0.001f; /* in glsl pow(x,0) is undefined */
			mtl_data.specular = glm::vec4(0.f, 0.f, 0.f, 0.f);
		}

		max = 1;

		materials.push_back(mtl_data);
	}

}

void RenderObject::recursive_pre_render(const aiNode* node) {
	const aiVector3D zero_3d(0.0f,0.0f,0.0f);

	for(unsigned int i=0; i<node->mNumMeshes; ++i) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		mesh_data_t md;

		md.mtl_index = mesh->mMaterialIndex;

		std::vector<Shader::vertex_t> vertexData;
		std::vector<unsigned int> indexData;

		for(unsigned int n = 0; n<mesh->mNumVertices; ++n) {
			const aiVector3D* pos = &(mesh->mVertices[n]);
			const aiVector3D* texCoord = &zero_3d;
			if(mesh->HasTextureCoords(0) && mesh->mTextureCoords[0] != NULL) {
				texCoord = &(mesh->mTextureCoords[0][n]);
			}
			const aiVector3D* normal = &(mesh->mNormals[n]);
			const aiVector3D* tangent, *bitangent;
			if(mesh->HasTangentsAndBitangents()) {
				tangent = &(mesh->mTangents[n]);
				bitangent= &(mesh->mBitangents[n]);
			} else {
				tangent = &zero_3d;
				bitangent = &zero_3d;
			}
			if(!mesh->HasNormals())
				normal = &zero_3d;

			/* still hate c++ for not using designated initializes */
			const Shader::vertex_t v = {
				/* .pos       = */ glm::vec3(pos->x, pos->y, pos->z),
				/* .uv        = */ glm::vec2(texCoord->x, texCoord->y),
				/* .normal    = */ glm::vec3(normal->x, normal->y, normal->z),
				/* .tangent   = */ glm::vec3(tangent->x, tangent->y, tangent->z),
				/* .bitangent = */ glm::vec3(bitangent->x, bitangent->y, bitangent->z),
				/* .color     = */ glm::vec4(0.0f)};
			vertexData.push_back(v);
		}

		for(unsigned int n = 0 ; n<mesh->mNumFaces; ++n) {
			const aiFace* face = &mesh->mFaces[n];
			assert(face->mNumIndices <= 3);
			if(face->mNumIndices == 3) { //Ignore points and lines
				md.num_indices+=3;

				for(unsigned int j = 0; j< face->mNumIndices; ++j) {
					int index = face->mIndices[j];
					indexData.push_back(index);
				}
			} else {
				fprintf(verbose, "Derp, ignoring mesh with %d indices\n", face->mNumIndices);
			}
		}

		glGenBuffers(1, &md.vb);

		glBindBuffer(GL_ARRAY_BUFFER, md.vb);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Shader::vertex_t)*vertexData.size(), &vertexData.front(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &md.ib);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, md.ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indexData.size(), &indexData.front(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mesh_data[mesh] = md;
	}

	for(unsigned int i=0; i<node->mNumChildren; ++i) {
		recursive_pre_render(node->mChildren[i]);
	}
}

void RenderObject::recursive_render(const aiNode* node,
		const glm::mat4 &parent_matrix) {


	aiMatrix4x4 m = node->mTransformation;
	m.Transpose();

	glm::mat4 matrix(parent_matrix);

	matrix *= glm::make_mat4((float*)&m);

	Shader::upload_model_matrix(matrix);

	for(unsigned int i=0; i<node->mNumMeshes; ++i) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		if(mesh->mNumFaces > 0) {
			mesh_data_t *md = &mesh_data[mesh];

			glBindBuffer(GL_ARRAY_BUFFER, md->vb);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, md->ib);

			glVertexAttribPointer(Shader::ATTR_POSITION,  3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, pos));
			glVertexAttribPointer(Shader::ATTR_TEXCOORD,  2, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, uv));
			glVertexAttribPointer(Shader::ATTR_NORMAL,    3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, normal));
			glVertexAttribPointer(Shader::ATTR_TANGENT,   3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, tangent));
			glVertexAttribPointer(Shader::ATTR_BITANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, bitangent));
			glVertexAttribPointer(Shader::ATTR_COLOR,     4, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, color));

			checkForGLErrors("set attrib pointers");

			materials[md->mtl_index].bind();
			checkForGLErrors("Activte material");

			glDrawElements(GL_TRIANGLES, md->num_indices, GL_UNSIGNED_INT,0 );
			checkForGLErrors("Draw material");

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			checkForGLErrors("Model post");
		}
	}

	for(unsigned int i=0; i<node->mNumChildren; ++i) {
		recursive_render(node->mChildren[i], matrix);
	}

}

void RenderObject::render(const glm::mat4& m) {
	if ( !scene ) return;
	recursive_render(scene->mRootNode, m * matrix());
}

const glm::mat4 RenderObject::matrix() const {
	//Apply scale and normalization matrix
	return MovableObject::matrix() * glm::scale(normalization_matrix_, scale);
}

void RenderObject::get_bounding_box_for_node (const aiNode* nd,
	aiVector3D* min,
	aiVector3D* max,
	aiMatrix4x4* trafo){

	aiMatrix4x4 prev;
	unsigned int n = 0, t;
	prev = *trafo;
	*trafo *= nd->mTransformation;

	for (; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {
			aiVector3D tmp = mesh->mVertices[t];
			tmp *= *trafo;

			min->x = std::min(min->x,tmp.x);
			min->y = std::min(min->y,tmp.y);
			min->z = std::min(min->z,tmp.z);

			max->x = std::max(max->x,tmp.x);
			max->y = std::max(max->y,tmp.y);
			max->z = std::max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}



void RenderObject::get_bounding_box (aiVector3D* min, aiVector3D* max) {
	aiMatrix4x4 trafo; //Set to identity

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}
