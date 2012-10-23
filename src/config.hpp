#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

class Config;

class ConfigEntry {
	public:
		const enum entry_type_t {
			ENTRY_MAP,
			ENTRY_LIST,
			ENTRY_STRING
		} type;

		~ConfigEntry();

		const std::string &as_string() const;
		int as_int() const;
		float as_float() const;

		glm::vec2 as_vec2() const; //Require format (x, y)
		glm::vec3 as_vec3() const; //Require format (x, y, z)
		glm::vec4 as_vec4() const; //Require format (x, y, z, w)
		Color as_color() const;

		const std::vector<ConfigEntry*> as_list() const;

		const ConfigEntry * find(const std::string &path, bool fail_on_not_found = false) const;

		void print(std::string indent = "") const;
	private:
		ConfigEntry(entry_type_t type_);

		//One of these are in use:
		std::map<std::string, ConfigEntry*> entry_map;
		std::vector<ConfigEntry*> entry_list;
		std::string entry_string;

		friend class Config;
};

class Config {
	Config(ConfigEntry * entry);
	std::shared_ptr<ConfigEntry> root;
	public:
		static Config parse(std::string file);
		~Config();

		const ConfigEntry * find(const std::string &path, bool fail_on_not_found = false) const;
		const ConfigEntry * operator[](const std::string &path) const;
		void print() const;
};


#endif
