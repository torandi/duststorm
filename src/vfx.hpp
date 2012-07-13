#ifndef VFX_HPP
#define VFX_HPP

#include <glm/glm.hpp>
#include <string>
#include <map>

#include "render_object.hpp"
#include "yaml-helper.hpp"
#include "particle_system.hpp"

/*
 * The VFX class has a state variable that identifies each single instance of the vfx.
 * To create a new state (and thus a new use instance) use create_state()
 *
 * The state could contain animation state etc
 */

class VFX {
	public:
		virtual void render(const glm::mat4 &matrix, const void * state) const = 0;
		virtual void * update(float dt, void * state) = 0;

		virtual void * create_state() = 0;

		static VFX * get_vfx(const std::string &name);
		static std::map<std::string, VFX*> loaded_vfx;
		//TODO: Animations etc
		
};

class ModelVFX : public VFX {
	public:
		ModelVFX(const YAML::Node &node);
		~ModelVFX();
		virtual void render(const glm::mat4 &matrix, const void * state) const;
		virtual void * update(float dt, void * state);
		virtual void * create_state();

	private:
		RenderObject *  render_object_;
};

class ParticlesVFX : public VFX {
	public:
		ParticlesVFX(const YAML::Node &node);

		virtual void render(const glm::mat4 &matrix, const void * state) const;
		virtual void * update(float dt, void * state);
		virtual void * create_state();

		void set_spawn_rate(void * state, float spawn_rate, float spawn_rate_var) const;
	private:
		ParticleSystem::config_t config;
		float avg_spawn_rate, spawn_rate_var;
		int count;
		TextureArray * textures;

};

#endif
