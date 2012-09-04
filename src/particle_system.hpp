#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "platform.h"
#include "movable_object.hpp"
#include "cl.hpp"
#include <glm/glm.hpp>
#include <list>
#include <utility>

class ParticleSystem : public MovableObject {
	public:

		ParticleSystem(const int max_num_particles, TextureArray* texture, bool _auto_spawn = true);
		~ParticleSystem();

		void update(float dt);
		void render(const glm::mat4 * m = nullptr);

		void update_config();

		//Change values in this struct and call update_config() to update
		__ALIGNED__(struct config_t {

				glm::vec4 birth_color;

				glm::vec4 death_color;

				glm::vec4 motion_rand;

				glm::vec4 spawn_direction;
				glm::vec4 direction_var;

				glm::vec4 spawn_position;
				glm::vec4 spawn_area;

				glm::vec4 directional_speed;
				glm::vec4 directional_speed_var;

				//Time to live
				cl_float avg_ttl;
				cl_float ttl_var;
				//Spawn speed
				cl_float avg_spawn_speed;
				cl_float spawn_speed_var;

				//Acceleration
				cl_float avg_acc;
				cl_float acc_var;
				//Scale
				cl_float avg_scale;
				cl_float scale_var;

				cl_float avg_scale_change;
				cl_float scale_change_var;

				//Rotation
				cl_float avg_rotation_speed;
				cl_float rotation_speed_var;

				//These two should not be manually changed!
				cl_int num_textures;
				cl_int max_num_particles;

		} config, 16);

		float avg_spawn_rate; //Number of particles to spawn per second
		float spawn_rate_var;
		bool auto_spawn;

		__ALIGNED__(struct vertex_t {
				glm::vec4 position;
				glm::vec4 color;
				cl_float scale;
				cl_int texture_index;
				},16);

		virtual void callback_position(const glm::vec3 &position);

		void push_config();
		void pop_config();

		/*
		 * Spawn count elements with current config
		 */
		void spawn(int count);
	private:

		/**
		 * Internal function for spawning count particles now
		 * event is set to the event for the execution
		 */
		void spawn_particles(cl_int count,cl::Event * event);

		const int max_num_particles_;


		//Texture * texture_;

		// Buffer 0: position buffer 1: color.
		// Both are set in the opencl-kernel
		GLuint gl_buffer_;
		std::vector<cl::BufferGL> cl_gl_buffers_;
		cl::Buffer particles_, config_, random_, spawn_rate_;

		cl::Program program_;
		cl::Kernel run_kernel_, spawn_kernel_;

		__ALIGNED__(
				struct particle_t {
				glm::vec4 direction;

				cl_float ttl;
				cl_float speed;
				cl_float acc;
				cl_float rotation_speed;

				cl_float initial_scale;
				cl_float final_scale;
				cl_float org_ttl;
				cl_int dead;

				glm::vec4 birth_color;
				glm::vec4 death_color;
				},16);

		TextureArray* texture_;


		typedef std::pair<config_t, int> spawn_data;

		std::list<spawn_data> spawn_list_;
		std::list<config_t> config_stack_;
};


#endif
