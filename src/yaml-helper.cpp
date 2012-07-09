#include "yaml-helper.hpp"

std::string YAML::to_string(const YAML::Node &node) {
	if(node.Type() == YAML::NodeType::Undefined) {
		return "[empty]";
	}

	return YAML::Dump(node);
}
