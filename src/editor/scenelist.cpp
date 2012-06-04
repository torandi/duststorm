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

		{
			GtkTreeIter iter;
			gtk_list_store_clear(propstore);
			SceneFactory::Metadata* meta = SceneFactory::find(name)->second.meta;
			for ( std::pair<std::string, std::string> p: *meta ){
				const std::string& key   = p.first;
				const std::string& value = p.second;
				gtk_list_store_append(propstore, &iter);
				gtk_list_store_set(propstore, &iter,
				                   0, key.c_str(),
				                   1, value.c_str(),
				                   -1);

			}
		}
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
				std::string filename = "";
				GdkPixbuf* icon = nullptr;
				gint type = Editor::TYPE_UNKNOWN;

				char* data = strdup(value.c_str());
				char* t = strtok(data, ":");
				char* d = strtok(NULL, ":");

				if ( strcmp(t, "path") == 0 ){
					type = Editor::TYPE_PATH;
					icon = icon_path;
					filename = std::string(d?d:"<nil>");
				} else if ( strcmp(t, "model") == 0 ){
					type = Editor::TYPE_MODEL;
					icon = icon_model;
					filename = std::string(d?d:"<nil>");
				} else if ( strcmp(t, "light") == 0 ){
					type = Editor::TYPE_LIGHT;
					icon = icon_light;
					filename = std::string(d?d:"<nil>");
				}

				free(data);

				GtkTreeIter cur;
				gtk_tree_store_append(scenestore, &cur, &child);
				gtk_tree_store_set(scenestore, &cur,
				                   COL_ICON, icon,
				                   COL_TITLE, key.c_str(),
				                   COL_FILENAME, filename.c_str(),
				                   COL_TYPE, type,
				                   -1);
			}
		}

		gtk_tree_store_append(scenestore, &toplevel, NULL);
		gtk_tree_store_set(scenestore, &toplevel, COL_TITLE, "<b>Compositions</b>", -1);
	}

}
