#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdlib>
#include <cstdio>
#include <list>

#include "config.hpp"
#include "globals.hpp"
#include "color.hpp"
#include "data.hpp"

static std::string trim(std::string s) {
	size_t begin_str = s.find_first_not_of(" \t\n");
	if(begin_str != std::string::npos) {
		size_t last = s.find_last_not_of(" \t\n");
		if(last >= begin_str) {
			std::string trimmed = s.substr(begin_str, last - begin_str + 1);
			return trimmed;
		} else {
			return "";
		}
	} else {
		return "";
	}
}

/*
 * Split on any of the chars in search
 * set keep to true to keep splited char
 */
static std::vector<std::string> split(std::string str, std::string search, bool keep) {
	size_t pos = 0;
	std::vector<std::string> res;
	size_t p;
	while((p = str.find_first_of(search, pos)) != std::string::npos) {
		if(p != pos) {
			std::string s = trim(str.substr(pos, (p - pos) + (keep ? 1 : 0)));
			if(!s.empty()) res.push_back(s);
		}
		pos = p + 1;
	}
	if(pos < str.length() - 1) {
		std::string s = trim(str.substr(pos));
		if(!s.empty()) res.push_back(s);
	}
	return res;
}

static void print_error(const char * error, int linenr, const std::string &line) {
	printf("[ConfigEntry] Parse error in line %d (%s): %s\n", linenr, line.c_str(), error);
	abort();
}

ConfigEntry::ConfigEntry(ConfigEntry::entry_type_t type_) : type(type_) {};

ConfigEntry::~ConfigEntry() {
	switch(type) {
		case ENTRY_MAP:
			for(auto it: entry_map) {
				delete it.second;
		}
			break;
		case ENTRY_LIST:
			for(auto it : entry_list) {
				delete it;
			}
			break;
		case ENTRY_STRING:
			break;
	}
}

Config Config::parse(std::string file) {
	Data * data = Data::open(file);
	std::string str = std::string((const char*)data->data(), data->size());
	std::vector<std::string> lines = split(str, ";[]{}", true);

	std::list<ConfigEntry*> config_stack;
	ConfigEntry * current = new ConfigEntry(ConfigEntry::ENTRY_MAP);
	int linenr = 0;
	for(std::string line : lines) {
		++linenr;
		//Remove semicolons:
		line = line.substr(0, line.find_last_not_of(";") + 1);
		switch(current->type) {
			case ConfigEntry::ENTRY_MAP:
				{
					if(line.find('}') != std::string::npos) {
						if(config_stack.empty()) print_error("unmatched '}'", linenr, line);
						current = config_stack.back();
						config_stack.pop_back();
						continue;
					}
					std::vector<std::string> data = split(line, "=", false);
					if(data.size() != 2) print_error("Data-value pair must be of size 2", linenr, line);
					if(current->entry_map.find(data[0]) != current->entry_map.end()) print_error("Duplicate map entry", linenr, line);
					ConfigEntry * new_cfg;
					switch(data[1][0]) {
						case '{':
							new_cfg = new ConfigEntry(ConfigEntry::ENTRY_MAP);
							break;
						case '[':
							new_cfg = new ConfigEntry(ConfigEntry::ENTRY_LIST);
							break;
						default:
							new_cfg = new ConfigEntry(ConfigEntry::ENTRY_STRING);
							new_cfg->entry_string = data[1];
					}
					current->entry_map[data[0]] = new_cfg;
					if(new_cfg->type != ConfigEntry::ENTRY_STRING) {
						config_stack.push_back(current);
						current = new_cfg;
					}
				}
				break;
			case ConfigEntry::ENTRY_LIST:
				{
					if(line.find(']') != std::string::npos) {
						if(config_stack.empty()) print_error("unmatched ']'", linenr, line);
						current = config_stack.back();
						config_stack.pop_back();
						continue;
					}
					ConfigEntry * new_cfg;
					switch(line[0]) {
						case '{':
							new_cfg = new ConfigEntry(ConfigEntry::ENTRY_MAP);
							break;
						case '[':
							new_cfg = new ConfigEntry(ConfigEntry::ENTRY_LIST);
							break;
						default:
							new_cfg = new ConfigEntry(ConfigEntry::ENTRY_STRING);
							new_cfg->entry_string = line;
					}
					current->entry_list.push_back(new_cfg);
					if(new_cfg->type != ConfigEntry::ENTRY_STRING) {
						config_stack.push_back(current);
						current = new_cfg;
					}
				}
				break;
			case ConfigEntry::ENTRY_STRING:
				print_error("parsing line in string context", linenr, line);
				break;
		}
	}

	if(!config_stack.empty()) {
		printf("[ConfigEntry] Parse error: End of file reached with unmatch } or ]\n");
		abort();
	}
	return Config(current);
}

void ConfigEntry::print(std::string indent) const {
	std::string indent_next = indent + "\t";
	switch(type) {
		case ENTRY_MAP:
			printf("%s{\n", indent.c_str());
			for(auto it : entry_map) {
				printf("%s%s = ", indent_next.c_str(), it.first.c_str());
				it.second->print(it.second->type == ENTRY_STRING ? "" : indent_next);
			}
			printf("%s}\n", indent.c_str());
			break;
		case ENTRY_LIST:
			printf("%s[\n", indent.c_str());
			for(auto it : entry_list) {
				it->print(indent_next);
			}
			printf("%s]\n", indent.c_str());
			break;
		case ENTRY_STRING:
			printf("%s%s;\n", indent.c_str(), entry_string.c_str());
			break;
	}
}

const std::string &ConfigEntry::as_string() const {
	if(type != ENTRY_STRING) {
		printf("[ConfigEntry] Trying to read a non-string entry as string\n");
		abort();
	}
	return entry_string;
}

int ConfigEntry::as_int() const {
	if(type != ENTRY_STRING) {
		printf("[ConfigEntry] Trying to read a non-string entry as int\n");
		abort();
	}
	return atoi(entry_string.c_str());
}

float ConfigEntry::as_float() const {
	if(type != ENTRY_STRING) {
		printf("[ConfigEntry] Trying to read a non-string entry as float\n");
		abort();
	}
	return atof(entry_string.c_str());
}

glm::vec2 ConfigEntry::as_vec2() const {
	if(type != ENTRY_STRING) {
		printf("[ConfigEntry] Trying to read a non-string entry as vec2\n");
		abort();
	}
	if(entry_string[0] != '(' || entry_string[entry_string.size() - 1] != ')') {
		printf("[ConfigEntry] A vec2 must start with ( and end with ): %s\n", entry_string.c_str());
		abort();
	}
	std::vector<std::string> data = split(entry_string.substr(1, entry_string.size() - 1), ",", false);
	if(data.size() != 2) {
		printf("[ConfigEntry] A vec2 must contain exactly one comma (,): %s\n", entry_string.c_str());
		abort();
	}
	return glm::vec2(atof(data[0].c_str()), atof(data[1].c_str()));
}

glm::vec3 ConfigEntry::as_vec3() const {
	if(type != ENTRY_STRING) {
		printf("[ConfigEntry] Trying to read a non-string entry as vec3\n");
		abort();
	}
	if(entry_string[0] != '(' || entry_string[entry_string.size() - 1] != ')') {
		printf("[ConfigEntry] A vec3 must start with ( and end with ): %s\n", entry_string.c_str());
		abort();
	}
	std::vector<std::string> data = split(entry_string.substr(1, entry_string.size() - 1),"," ,false);
	if(data.size() != 3) {
		printf("[ConfigEntry] A vec3 must contain exactly two commas (,): %s\n", entry_string.c_str());
		abort();
	}
	return glm::vec3(atof(data[0].c_str()), atof(data[1].c_str()), atof(data[2].c_str()));
}

glm::vec4 ConfigEntry::as_vec4() const {
	if(type != ENTRY_STRING) {
		printf("[ConfigEntry] Trying to read a non-string entry as vec4\n");
		abort();
	}
	if(entry_string[0] != '(' || entry_string[entry_string.size() - 1] != ')') {
		printf("[ConfigEntry] A vec4 must start with ( and end with ): %s\n", entry_string.c_str());
		abort();
	}
	std::vector<std::string> data = split(entry_string.substr(1, entry_string.size() - 1),"," ,false);
	if(data.size() != 4) {
		printf("[ConfigEntry] A vec4 must contain exactly three commas (,): %s\n", entry_string.c_str());
		abort();
	}
	return glm::vec4(atof(data[0].c_str()), atof(data[1].c_str()), atof(data[2].c_str()), atof(data[3].c_str()));
}

Color ConfigEntry::as_color() const {
	if(type != ENTRY_STRING) {
		printf("[ConfigEntry] Trying to read a non-string entry as color\n");
		abort();
	}
	size_t len = split(entry_string, ",", false).size();
	if(len == 3) {
		return Color(as_vec3());
	} else if(len == 4) {
		return Color(as_vec4());
	} else {
		printf("A color must have 3 or 4 components: %s\n", entry_string.c_str());
		abort();
	}
}

const std::vector<ConfigEntry*> ConfigEntry::as_list() const {
	if(type != ENTRY_LIST) {
		printf("[ConfigEntry] Trying to read a non-list entry as list\n");
		abort();
	}
	return entry_list;
}

const ConfigEntry * ConfigEntry::find(const std::string &path, bool fail_on_not_found) const {
	if(type != ENTRY_MAP) {
		printf("[ConfigEntry] Can't search non-map config entry\n");
		abort();
	}

	std::vector<std::string> data = split(path, "/", false);
	const ConfigEntry * current = this;
	std::string prev = "/";
	for(std::string &s : data) {
		if(current->type != ENTRY_MAP) {
			fprintf(verbose, "[ConfigEntry] Entry %s is of non-map type, can't search\n", prev.c_str());
			current = nullptr;
			break;
		}
		auto f = current->entry_map.find(s);
		if(f != current->entry_map.end()) {
			current = f->second;
		} else {
			printf("%s not found\n", s.c_str());
			current = nullptr;
			break;
		}
		prev = s;
	}
	if(current == nullptr && fail_on_not_found) {
		printf("[ConfigEntry] Entry not found: %s\n", path.c_str());
		abort();
	}
	return current;
}

const ConfigEntry * Config::find(const std::string &path, bool fail_on_not_found) const {
	return root->find(path, true);
}
const ConfigEntry * Config::operator[](const std::string &path) const {
	return find(path, true);
}

Config::Config(ConfigEntry * entry) : root(entry) { }

Config::~Config() {
	delete root;
}

void Config::print() const {
	root->print();
}
