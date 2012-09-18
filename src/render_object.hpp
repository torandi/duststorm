#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include "movable_object.hpp"

#include <string>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class RenderObject : public MovableObject {

	glm::mat4 normalization_matrix_;
	Assimp::Importer importer;

	void get_bounding_box_for_node (const aiNode* nd,	aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
	void get_bounding_box (aiVector3D* min, aiVector3D* max);
	void color4_to_vec4(const aiColor4D *c, glm::vec4 &target);

	//Trims path and loads texture
	TextureBase* load_texture(const std::string& path);

	void pre_render();
	void recursive_pre_render(const aiNode* node);

	void recursive_render(const aiNode* node, const glm::mat4 &matrix);

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

	//Set normalize_scale to false to not scale down to 1.0
	RenderObject(std::string model, bool normalize_scale=false, unsigned int aiOptions=0);
	~RenderObject();

	std::vector<Material> materials;

	std::map<const aiMesh*, mesh_data_t > mesh_data;


	void render(const glm::mat4& m = glm::mat4());

	const glm::mat4 matrix() const;


};

#endif
