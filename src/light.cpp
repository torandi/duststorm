#include "light.hpp"

Light::Light() :
		constant_attenuation(0.0f),
		linear_attenuation(0.0f),
		quadratic_attenuation(0.0002f),
		type(DIRECTIONAL_LIGHT) {

}
