#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor/editor.hpp"
#include "scene.hpp"
#include <cstring>
#include <gtk/gtk.h>

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
};

static Editor::TYPE classify_name(const std::string&name, std::string& data){
	const size_t offset = name.find_first_of(":");
	std::string prefix = name;
	data = "";

	if ( offset != std::string::npos ){
		prefix = name.substr(0, offset);
		data = name.substr(offset+1);
	}

	if ( prefix == "path"     ) return Editor::TYPE_PATH;
	if ( prefix == "model"    ) return Editor::TYPE_MODEL;
	if ( prefix == "light"    ) return Editor::TYPE_LIGHT;
	if ( prefix == "int"      ) return Editor::TYPE_INT;
	if ( prefix == "float"    ) return Editor::TYPE_FLOAT;
	if ( prefix == "vec2"     ) return Editor::TYPE_VEC2;
	if ( prefix == "vec3"     ) return Editor::TYPE_VEC3;
	if ( prefix == "vec4"     ) return Editor::TYPE_VEC4;
	if ( prefix == "string"   ) return Editor::TYPE_STRING;

	data = name;
	return Editor::TYPE_UNKNOWN;
}

static void populate_sceneprops(const std::string& name){
	GtkTreeIter parent, child;
	gtk_tree_store_clear(propstore);
	SceneFactory::Metadata* meta = SceneFactory::find(name)->second.meta;
	for ( std::pair<std::string, std::string> p: *meta ){
		const std::string& key   = p.first;
		const std::string& value = p.second;

		std::string data;
		Editor::TYPE type = classify_name(value, data);

		switch ( type ){
		case Editor::TYPE_PATH:
		case Editor::TYPE_MODEL:
			/* don't show these in property editor */
			break;

		case Editor::TYPE_LIGHT:
			gtk_tree_store_append(propstore, &parent, NULL);
			gtk_tree_store_set(propstore, &parent, PROP_KEY, key.c_str(), PROP_VALUE, "Light", PROP_COLOR, "#ddd", -1);

			gtk_tree_store_append(propstore, &child, &parent);
			gtk_tree_store_set(propstore, &child, PROP_KEY, "Type", PROP_VALUE, "derp", PROP_EDITABLE, TRUE, -1);

			gtk_tree_store_append(propstore, &child, &parent);
			gtk_tree_store_set(propstore, &child, PROP_KEY, "Attenuation", PROP_VALUE, "derp", PROP_EDITABLE, TRUE, PROP_MODE, gtk_cell_renderer_progress_new(), -1);

			gtk_tree_store_append(propstore, &child, &parent);
			gtk_tree_store_set(propstore, &child, PROP_KEY, "Color", PROP_VALUE, "derp", PROP_EDITABLE, TRUE, -1);

			gtk_tree_store_append(propstore, &child, &parent);
			gtk_tree_store_set(propstore, &child, PROP_KEY, "Position", PROP_VALUE, "derp", PROP_EDITABLE, TRUE, -1);
			break;

		default:
			gtk_tree_store_append(propstore, &parent, NULL);
			gtk_tree_store_set(propstore, &parent, PROP_KEY, key.c_str(), PROP_VALUE, value.c_str(), PROP_EDITABLE, TRUE, -1);
		}
	}
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

	case Editor::TYPE_SCENE:
		Editor::mode = Editor::MODE_SCENE;
		delete scene;
		Editor::scene_name = name;
		scene = SceneFactory::create(std::string(name), frame->texture_size());
		assert(scene);
		scene->add_time(0,60);
		global_time.reset();
		populate_sceneprops(name);
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

namespace Editor {

	void scenelist_populate(){
		static GdkPixbuf* type_icon[Editor::TYPE_MAX] = {
			NULL,           /* TYPE_CAT */
			icon_cat_scene, /* TYPE_SCENE, */
			NULL,           /* TYPE_COMPOSITION, */
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
			                   COL_TYPE, Editor::TYPE_SCENE,
			                   -1);

			SceneFactory::Metadata* meta = it->second.meta;
			for ( std::pair<std::string, std::string> p: *meta ){
				const std::string& key   = p.first;
				const std::string& value = p.second;
				GtkTreeIter cur;
				std::string data;
				Editor::TYPE type = classify_name(value, data);

				switch ( type ){
				case Editor::TYPE_PATH:
				case Editor::TYPE_MODEL:
				case Editor::TYPE_LIGHT:
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
	}

}
