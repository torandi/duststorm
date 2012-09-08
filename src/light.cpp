#include "light.hpp"

Light::Light() :
		constant_attenuation(1.0f),
		linear_attenuation(0.01f),
		quadratic_attenuation(0.002f),
		is_directional(1) {

}
