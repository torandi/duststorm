#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "movable_object.hpp"

#define HALF_LIGHT_DISTANCE 1.5f

class Light : public MovableObject {
public:
   struct shader_light_t {
      float attenuation;
      int type; //light_type_t
      float padding[1];
      glm::vec3 intensity;
      glm::vec3 position;
   };

   enum light_type_t {
      DIRECTIONAL_LIGHT = 0, //No attenuation
      POINT_LIGHT = 1
   } light_type;

private:

   shader_light_t attributes_;
public:

	Light(light_type_t type, glm::vec3 _intensity);
	Light(light_type_t type, glm::vec3 _intensity, glm::vec3 position);
	virtual ~Light() {};

	void set_half_light_distance(float hld);

   const float &attenuation() const;
   const glm::vec3 &intensity() const;

   const shader_light_t &shader_light();

   const light_type_t type() const;

   void set_type(light_type_t type);

   virtual void set_position(const glm::vec3 &pos);
};
#endif
