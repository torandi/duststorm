#ifndef YAML_HELPER_HPP
#define YAML_HELPER_HPP

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

#include "color.hpp"

namespace YAML {

	std::string to_string(const YAML::Node &node);

	template<>
	struct convert<glm::vec2> {
		static Node encode(const glm::vec2 &vec) {
			Node node;
			node.push_back(vec.x);
			node.push_back(vec.y);
			return node;
		}

		static bool decode(const Node &node, glm::vec2 &vec) {
			if(!node.IsSequence()) {
				return false;
			} else if(!node.size() == 2) {
				return false;
			}
			vec.x = node[0].as<float>();
			vec.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Color> {
		static Node encode(const Color &color) {
			Node node;
			node.push_back(color.r*255);
			node.push_back(color.g*255);
			node.push_back(color.b*255);
			node.push_back(color.a*255);
			return node;
		}

		static bool decode(const Node &node, Color &color) {
			if(!node.IsSequence()) {
				return false;
			} else if(node.size() == 3) {
				color.r = node[0].as<float>()/255.0;
				color.g = node[1].as<float>()/255.0;
				color.b = node[2].as<float>()/255.0;
				return true;
			} else if(node.size() == 4) {
				color.r = node[0].as<float>()/255.0;
				color.g = node[1].as<float>()/255.0;
				color.b = node[2].as<float>()/255.0;
				color.a = node[3].as<float>()/255.0;
				return true;
			} else {
				return false;
			}
		}
	};
}

#endif
