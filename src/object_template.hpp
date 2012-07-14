#ifndef OBJECT_TEMPLATE_HPP
#define OBJECT_TEMPLATE_HPP

#include "object2d.hpp"
#include "yaml-helper.hpp"

class Game;


class ObjectTemplate {
	public:
		ObjectTemplate() : destroyed(false), highlighted(false), ttl(-1.f) {};
		virtual ~ObjectTemplate();
		Object2D * obj;
		bool destroyed;
		bool highlighted;
		virtual bool click() = 0;
		virtual bool collision() = 0;
		virtual bool hit() = 0;

		virtual void update(float dt);
		virtual void render() const;

		float ttl;
};

class Decoration : public ObjectTemplate {
	public:
		virtual ~Decoration() {};
		static ObjectTemplate * create(const YAML::Node &node, Game &game);
		static ObjectTemplate * create(const std::string &vfx, Game &game);

		virtual bool click() { return false; }
		virtual bool collision() { return false; }
		virtual bool hit() { return false; }
};

class Pickup : public ObjectTemplate {
	public:
		virtual ~Pickup() {};
		static ObjectTemplate * create(const YAML::Node &node, Game &game);
		static ObjectTemplate * create(const std::string &vfx, const std::string &attr, int effect, float radius, float height, Game &game);

		std::string attr;
		int effect;

		virtual bool click();
		virtual bool collision();
		virtual bool hit() { return false; }
};

class Door : public ObjectTemplate {
	public:
		virtual ~Door() {};
		static ObjectTemplate * create(const YAML::Node &node, Game &game);

		std::string area, entry_point;
		virtual bool click();
		virtual bool collision();
		virtual bool hit();
};

typedef ObjectTemplate* (object_template_create) (const YAML::Node &node, Game &game);

#endif
