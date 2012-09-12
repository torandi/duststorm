#include "light.hpp"

Light::Light() :
		constant_attenuation(1.0f),
		linear_attenuation(0.001f),
		quadratic_attenuation(0.04f),
		is_directional(1),
		shadow_bias(0.0001f) {

}
