#ifndef META_H
#define META_H

#include "movable_light.hpp"
#include "lights_data.hpp"
#include <string>

class Meta {
public:
	enum Type {
		TYPE_MODEL,
		TYPE_PATH,
		TYPE_LIGHT,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_VEC2,
		TYPE_VEC3,
		TYPE_VEC4,
		TYPE_STRING,
		TYPE_COLOR,
	};

	Meta(Type type): type(type){}
	const Type type;

	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset) = 0;
	virtual std::string get_string(Scene* instance, unsigned int offset) const = 0;
};

class MetaFile: public Meta {
protected:
	MetaFile(const std::string& filename, Type type): Meta(type){}

public:
	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset){ return str; };
	virtual std::string get_string(Scene* instance, unsigned int offset) const { return ""; }
};

class MetaModel: public MetaFile {
public:
	MetaModel(const std::string& filename): MetaFile(filename, TYPE_MODEL){}
};

class MetaPath: public MetaFile {
public:
	MetaPath(const std::string& filename): MetaFile(filename, TYPE_PATH){}
};

class MetaLightBase: public MetaFile {
public:
	MetaLightBase(const std::string& filename)
		: MetaFile(filename, TYPE_LIGHT){
	}

	virtual MovableLight& get(Scene* instance) const = 0;
	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset);
	virtual std::string get_string(Scene* instance, unsigned int offset) const;
};

template <class cls>
class MetaLight: public MetaLightBase {
public:
	MetaLight(LightsData cls::*light, unsigned int index, const std::string& filename)
		: MetaLightBase(filename)
		, light(light)
		, index(index) {
	}

	virtual MovableLight& get(Scene* instance) const { return (dynamic_cast<cls*>(instance)->*light).lights[index]; }

private:
	LightsData cls::*light;
	unsigned int index;
};

template <class T>
class MetaVariableBase: public Meta {
public:
	MetaVariableBase();
	virtual void set(Scene* instance, const T& value) = 0;
	virtual T get(Scene* instance) const = 0;

	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset);
	virtual std::string get_string(Scene* instance, unsigned int offset) const;
};

template <class T, class cls>
class MetaVariable: public MetaVariableBase<T> {
public:
	MetaVariable(T cls::*ptr): ptr(ptr){}
	virtual void set(Scene* instance, const T& value){ dynamic_cast<cls*>(instance)->*ptr = value; }
	virtual T get(Scene* instance) const {
		const cls* tmp = dynamic_cast<cls*>(instance);
		return tmp->*ptr;
	}
private:
	T cls::*ptr;
};

#endif /* META_H */
