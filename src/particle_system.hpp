#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "movable_object.hpp"

#include "cl.hpp" 
#include <glm/glm.hpp>

class ParticleSystem : public MovableObject {

   const int max_num_particles_;

   //Texture * texture_;

   // Buffer 0: position buffer 1: color.
   // Both are set in the opencl-kernel
   GLuint gl_buffer_; 
   std::vector<cl::Memory> cl_gl_buffers_;
   cl::Buffer particles_, config_, random_;

   cl::Program program_;
   cl::Kernel kernel_;
   
   struct particle_t {
      glm::vec4 direction;
      float ttl;
      float speed;
      float acc;
			float org_ttl;
   };

	struct vertex_t {
		glm::vec4 position;
		glm::vec4 color;
	};

   std::vector<cl::Event> update_blocking_events_;
   std::vector<cl::Event> render_blocking_events_;

   public:

   ParticleSystem(const int max_num_particles);
   ~ParticleSystem();

   void update(float dt);
   void render();

   //Limit the spawing of particles
   void limit_particles(int limit);
   void update_config();

   //Change values in this struct and call update_config() to update
   struct {

      glm::vec4 birth_color; 

      glm::vec4 death_color;

      glm::vec4 motion_rand; 

      glm::vec4 spawn_direction;
      glm::vec4 direction_var;

      glm::vec4 spawn_position;
      glm::vec4 spawn_area;

      //Time to live
      float avg_ttl;
      float ttl_var;
      //Spawn speed
      float avg_spawn_speed; 
      float spawn_speed_var;

      //Acceleration 
      float avg_acc;
      float acc_var;
      //Scale
      float avg_scale;
      float scale_var;

		int max_num_particles;      
   } config;
};

#endif
