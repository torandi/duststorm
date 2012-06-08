#ifndef MOVABLE_LIGHT_H
#define MOVABLE_LIGHT_H

#include "movable_object.hpp"
#include "light.hpp"

class MovableLight : public MovableObject {
	public:
		MovableLight(Light * light);

		void update(); //Must be called to update position in light

		Light * data;
};

#endif
