#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include "shader.hpp"
#include "movable_object.hpp"

#include <string>
#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class RenderObject : public MovableObject {

	glm::mat4 normalization_matrix_;

	void get_bounding_box_for_node (const struct aiNode* nd,	struct aiVector3D* min, struct aiVector3D* max, struct aiMatrix4x4* trafo);
	void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);
	void color4_to_vec4(const struct aiColor4D *c, glm::vec4 &target);

	//Trims path and loads texture
	static Texture* load_texture(std::string path);

	void pre_render();
	void recursive_pre_render(const aiNode* node);

	void recursive_render(const aiNode* node, const Shader * shader, const glm::mat4 &matrix);

public:
	const aiScene* scene;
	glm::vec3 scene_min, scene_max, scene_center;
	std::string name;
	glm::vec3 scale;

	struct mesh_data_t {
		mesh_data_t() : num_indices(0) {};
		GLuint vb, ib; //vertex buffer, index buffer
		GLenum draw_mode;
		unsigned int num_indices;
		unsigned int mtl_index;
	};

	struct vertex_t {
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		vertex_t(const aiVector3D* pos_,const  aiVector3D* texCoord_,
					const aiVector3D* normal_, const aiVector3D * tangent_, const aiVector3D * bitangent_) {
			pos = glm::vec3(pos_->x,pos_->y, pos_->z);
			texCoord = glm::vec2(texCoord_->x,texCoord_->y);
			normal = glm::vec3(normal_->x, normal_->y, normal_->z);
			tangent = glm::vec3(tangent_->x, tangent_->y, tangent_->z);
			bitangent = glm::vec3(bitangent_->x, bitangent_->y, bitangent_->z);
		}
	};

	struct material_t {
		material_t()
			: texture(nullptr)
			, normal_map(nullptr)
			, two_sided(false){

		};

		Texture* texture;
		Texture* normal_map;
		Shader::material_t attr;
		bool two_sided;

		void activate();
		void deactivate();
	};

	//Set normalize_scale to false to not scale down to 1.0
	RenderObject(std::string model, bool normalize_scale=true, unsigned int aiOptions=0);
	~RenderObject();

	std::vector<material_t> materials;

	std::map<const aiMesh*, mesh_data_t > mesh_data;


	void render(const Shader  * shader);

	const glm::mat4 matrix() const;


};

#endif
