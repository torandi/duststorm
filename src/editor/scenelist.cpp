#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor/editor.hpp"
#include "scene.hpp"
#include <cstring>
#include <gtk/gtk.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sstream>

enum COL {
	COL_ICON = 0,
	COL_TITLE,
	COL_FILENAME,
	COL_TYPE,
};

enum {
	PROP_KEY = 0,
	PROP_VALUE,
	PROP_MODE,
	PROP_EDITABLE,
	PROP_COLOR,
	PROP_POINTER,
	PROP_OFFSET,
	PROP_TYPE,
};

enum {
	PROP_UNSET = 0,
	PROP_FLOAT,
};

static unsigned int add_property(GtkTreeIter* parent, const glm::vec3& v, const std::string& name, Meta* variable, unsigned int offset){
	GtkTreeIter cur, child;
	std::stringstream s; s << "<" << v.x << ", " << v.y << ", " << v.z << ">";
	std::stringstream x; x << v.x;
	std::stringstream y; y << v.y;
	std::stringstream z; z << v.z;

	gtk_tree_store_append(propstore, &cur, parent);
	gtk_tree_store_set(propstore, &cur, PROP_KEY, name.c_str(), PROP_VALUE, s.str().c_str(), PROP_COLOR, "#ddd", -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "x", PROP_VALUE, x.str().c_str(), PROP_EDITABLE, TRUE, PROP_TYPE, PROP_FLOAT, PROP_POINTER, variable, PROP_OFFSET, offset + 1, -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "y", PROP_VALUE, y.str().c_str(), PROP_EDITABLE, TRUE, PROP_TYPE, PROP_FLOAT, PROP_POINTER, variable, PROP_OFFSET, offset + 2, -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "z", PROP_VALUE, z.str().c_str(), PROP_EDITABLE, TRUE, PROP_TYPE, PROP_FLOAT, PROP_POINTER, variable, PROP_OFFSET, offset + 3, -1);

	return offset + 3;
}

static unsigned int add_property(GtkTreeIter* parent, const Color& c, unsigned int components, const std::string& name, Meta* variable, unsigned int offset){
	GtkTreeIter cur, child;
	std::stringstream r; r << c.r;
	std::stringstream g; g << c.g;
	std::stringstream b; b << c.b;
	std::stringstream a; a << c.a;

	gtk_tree_store_append(propstore, &cur, parent);
	gtk_tree_store_set(propstore, &cur, PROP_KEY, name.c_str(), PROP_VALUE, "Color", PROP_COLOR, "#ddd", -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "r", PROP_VALUE, r.str().c_str(), PROP_EDITABLE, TRUE, PROP_TYPE, PROP_FLOAT, PROP_POINTER, variable, PROP_OFFSET, offset + 1, -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "g", PROP_VALUE, g.str().c_str(), PROP_EDITABLE, TRUE, PROP_TYPE, PROP_FLOAT, PROP_POINTER, variable, PROP_OFFSET, offset + 2, -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "b", PROP_VALUE, b.str().c_str(), PROP_EDITABLE, TRUE, PROP_TYPE, PROP_FLOAT, PROP_POINTER, variable, PROP_OFFSET, offset + 3, -1);

	if ( components == 4 ){
		gtk_tree_store_append(propstore, &child, &cur);
		gtk_tree_store_set(propstore, &child, PROP_KEY, "a", PROP_VALUE, a.str().c_str(), PROP_EDITABLE, TRUE, PROP_TYPE, PROP_FLOAT, PROP_POINTER, variable, PROP_OFFSET, offset + 4, -1);
	}

	return offset + components;
}

static void add_light_property(GtkTreeIter* parent, const MovableLight& light, const std::string& name, Meta* variable, unsigned int offset){
	GtkTreeIter cur, child;

	std::stringstream t; t << light.type;
	std::stringstream ac; ac << light.constant_attenuation;

	gtk_tree_store_append(propstore, &cur, parent);
	gtk_tree_store_set(propstore, &cur, PROP_KEY, name.c_str(), PROP_VALUE, "Light", PROP_POINTER, variable, PROP_COLOR, "#ddd", -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "Type", PROP_VALUE, t.str().c_str(), PROP_EDITABLE, TRUE, PROP_OFFSET, offset + 1, -1);

	gtk_tree_store_append(propstore, &child, &cur);
	gtk_tree_store_set(propstore, &child, PROP_KEY, "Attenuation", PROP_VALUE, ac.str().c_str(), PROP_EDITABLE, TRUE, PROP_OFFSET, offset + 2, -1);

	offset += 2;
	offset = add_property(&cur, Color(light.intensity), 3, "Color", NULL, offset);
	offset = add_property(&cur, light.position(), "Position", NULL, offset);
}

static void notify_update(GtkTreeIter* iter, const char* new_text, unsigned int offset){
	/* locate meta variable by searching parents */
	GtkTreeIter cur = *iter;
	GtkTreeIter prev = cur;
	gpointer pointer;
	Meta* variable = nullptr;
	do {
		gtk_tree_model_get(GTK_TREE_MODEL(propstore), &cur, PROP_POINTER, &pointer, -1);

		variable = static_cast<Meta*>(pointer);
		if ( variable ) break;

		prev = cur;
	} while ( gtk_tree_model_iter_parent(GTK_TREE_MODEL(propstore), &cur, &prev) == TRUE );

	if ( !variable ){
		fprintf(stderr, "Cell does not have an associated metadata variable assigned, edit ignored.\n");
		return;
	}

	variable->set_string(scene, std::string(new_text), offset);
	gtk_tree_store_set(propstore, iter, PROP_VALUE, variable->get_string(scene, offset).c_str(), -1);
}

namespace Editor { void sceneprops_populate(Scene* scene){
	GtkTreeIter parent;
	gtk_tree_store_clear(propstore);
	for ( std::pair<std::string, Meta*> p: scene->metadata() ){
		const std::string& key   = p.first;
		Meta* variable = p.second;

		switch ( variable->type ){
		case Meta::TYPE_MODEL:
		case Meta::TYPE_PATH:
			/* don't show these in property editor */
			break;

		case Meta::TYPE_LIGHT:
		{
			const MetaLightBase* r = dynamic_cast<const MetaLightBase*>(variable);
			add_light_property(NULL, r->get(scene), key, variable, 0);
			break;
		}

		case Meta::TYPE_VEC3:
		{
			const MetaVariableBase<glm::vec3>* r = dynamic_cast<const MetaVariableBase<glm::vec3>*>(variable);
			add_property(NULL, r->get(scene), key, variable, 0);
			break;
		}

		case Meta::TYPE_COLOR:
		{
			const MetaVariableBase<Color>* r = dynamic_cast<const MetaVariableBase<Color>*>(variable);
			add_property(NULL, r->get(scene), 4, key, variable, 0);
			break;
		}

		default:
			gtk_tree_store_append(propstore, &parent, NULL);
			gtk_tree_store_set(propstore, &parent, PROP_KEY, key.c_str(), PROP_VALUE, variable->get_string(scene, 0).c_str(), PROP_EDITABLE, TRUE, PROP_POINTER, variable, -1);
		}
	}
}}

extern "C" G_MODULE_EXPORT void property_edited_cb (GtkCellRendererText* cell, gchar* path, gchar* new_text, gpointer user_data){
	GtkTreeIter iter;
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(propstore), &iter, path);

	guint offset;
	gtk_tree_model_get(GTK_TREE_MODEL(propstore), &iter, PROP_OFFSET, &offset, -1);

	notify_update(&iter, new_text, offset);
}

extern "C" G_MODULE_EXPORT void scenelist_row_activated_cb(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data){
	//gint depth;
	//gint* tree = gtk_tree_path_get_indices_with_depth(path, &depth);

	GtkTreeIter iter;
	GtkTreeModel* treemodel = gtk_tree_view_get_model(tree_view);
	if ( !gtk_tree_model_get_iter(treemodel, &iter, path) ){
		return;
	}

	gchar* name;
	gchar* filename;
	Editor::TYPE type;
	gtk_tree_model_get(treemodel, &iter, COL_TITLE, &name, COL_TYPE, &type, COL_FILENAME, &filename, -1);

	switch ( type ){
	case Editor::TYPE_CAT:
		if ( gtk_tree_view_row_expanded(tree_view, path) ){
			gtk_tree_view_collapse_row(tree_view, path);
		} else {
			gtk_tree_view_expand_row(tree_view, path, FALSE);
		}
		break;

	case Editor::TYPE_CAT_SCENE:
		Editor::load_scene(std::string(name));
		break;

	case Editor::TYPE_PATH:
		break;

	case Editor::TYPE_MODEL:
		Editor::mode = Editor::MODE_MODEL;
		Editor::reset();
		Editor::load_model(filename);
		break;

	default:
		printf("type: %d\n", type);
		break;
	}

	g_free(name);
	g_free(filename);
}

extern "C" G_MODULE_EXPORT void properties_scroll_event_cb(GtkWidget* widget, GdkEventScroll* event, gpointer data){
	GtkTreeIter iter;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if ( !gtk_tree_selection_get_selected(selection, nullptr, &iter) ) return;

	guint type, offset;
	gchar* value;
	gtk_tree_model_get(GTK_TREE_MODEL(propstore), &iter, PROP_VALUE, &value, PROP_TYPE, &type, PROP_OFFSET, &offset, -1);

	static char buf[64];

	switch ( type ){
	case PROP_FLOAT:
		snprintf(buf, 64, "%f", atof(value) + (event->direction == GDK_SCROLL_UP ? 0.1 : -0.1));
		notify_update(&iter, buf, offset);
		break;
	}

	g_free(value);
}

namespace Editor {

	void scenelist_populate(){
		static GdkPixbuf* type_icon[Editor::TYPE_MAX] = {
			NULL,           /* TYPE_CAT */
			icon_cat_scene, /* TYPE_CAT_SCENE, */
			NULL,           /* TYPE_CAT_COMPOSITION, */
			NULL,           /* TYPE_CAT_MODELS */
			icon_path,      /* TYPE_PATH, */
			icon_model,     /* TYPE_MODEL, */
			icon_light,     /* TYPE_LIGHT, */
			NULL,           /* TYPE_INT, */
			NULL,           /* TYPE_FLOAT, */
			NULL,           /* TYPE_VEC2, */
			NULL,           /* TYPE_VEC3, */
			NULL,           /* TYPE_VEC4, */
			NULL,           /* TYPE_STRING, */
		};

		GtkTreeIter toplevel;
		gtk_tree_store_append(scenestore, &toplevel, NULL);
		gtk_tree_store_set(scenestore, &toplevel,
		                   COL_ICON, icon_cat_scene,
		                   COL_TITLE, "<b>Scenes</b>",
		                   COL_TYPE, Editor::TYPE_CAT,
		                   -1);

		for ( auto it = SceneFactory::begin(); it != SceneFactory::end(); ++it ){
			GtkTreeIter child;
			gtk_tree_store_append(scenestore, &child, &toplevel);
			gtk_tree_store_set(scenestore, &child,
			                   COL_ICON, icon_scene,
			                   COL_TITLE, it->first.c_str(),
			                   COL_TYPE, Editor::TYPE_CAT_SCENE,
			                   -1);

			SceneFactory::Metadata* meta = it->second.meta;
			for ( std::pair<std::string, Meta*> p: *meta ){
				const std::string& key   = p.first;
				const Meta* value = p.second;
				GtkTreeIter cur;
				std::string data;
				//Editor::TYPE type = Editor::classify_name(value, data);

				static Editor::TYPE type_lut[] = {
					Editor::TYPE_MODEL,
					Editor::TYPE_PATH,
					Editor::TYPE_LIGHT,
					Editor::TYPE_UNKNOWN,
				};
				Editor::TYPE type = type_lut[value->type];

				switch ( value->type ){
				case Meta::TYPE_MODEL:
				case Meta::TYPE_PATH:
				case Meta::TYPE_LIGHT:
					gtk_tree_store_append(scenestore, &cur, &child);
					gtk_tree_store_set(scenestore, &cur,
					                   COL_ICON, type_icon[type],
					                   COL_TITLE, key.c_str(),
					                   COL_FILENAME, data.c_str(),
					                   COL_TYPE, type,
					                   -1);
					break;

				default:
					break;
				}
			}
		}

		gtk_tree_store_append(scenestore, &toplevel, NULL);
		gtk_tree_store_set(scenestore, &toplevel, COL_TITLE, "<b>Compositions</b>", -1);

		gtk_tree_store_append(scenestore, &toplevel, NULL);
		gtk_tree_store_set(scenestore, &toplevel, COL_ICON, icon_cat_model, COL_TITLE, "<b>Models</b>", -1);
		{
			GtkTreeIter child;
			struct dirent **namelist;
			int n = scandir(PATH_MODELS, &namelist,
			                [](const struct dirent* d) -> int { return fnmatch("*.obj", d->d_name, 0) == 0; },
			                alphasort);
			while ( n --> 0 ) {
				gtk_tree_store_append(scenestore, &child, &toplevel);
				gtk_tree_store_set(scenestore, &child,
				                   COL_ICON, icon_model,
				                   COL_TITLE, namelist[n]->d_name,
				                   COL_FILENAME, namelist[n]->d_name,
				                   COL_TYPE, TYPE_MODEL,
				                   -1);
				free(namelist[n]);
			}
			free(namelist);
		}
	}

}
