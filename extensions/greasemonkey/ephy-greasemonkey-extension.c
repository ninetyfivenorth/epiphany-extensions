/*
 *  Copyright (C) 2005 Adam Hooper
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "config.h"

#include "ephy-greasemonkey-extension.h"
#include "greasemonkey-script.h"
#include "mozilla-helpers.h"
#include "ephy-debug.h"
#include "ephy-file-helpers.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-embed-factory.h>
#include <epiphany/ephy-embed-event.h>
#include <epiphany/ephy-embed-persist.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-window.h>

#include <gtk/gtkuimanager.h>

#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define EPHY_GREASEMONKEY_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GREASEMONKEY_EXTENSION, EphyGreasemonkeyExtensionPrivate))

#define WINDOW_DATA_KEY "EphyGreasemonkeyExtensionWindowData"
#define ACTION_NAME "EphyGreasemonkeyInstallScript"
#define ACTION_LINK_KEY "EphyGreasemonkeyScriptUrl"

struct _EphyGreasemonkeyExtensionPrivate
{
	GHashTable *scripts;
	GnomeVFSMonitorHandle *monitor;
};

typedef struct
{
	GtkActionGroup *action_group;
	guint ui_id;
	char *last_clicked_url;
} WindowData;

typedef struct
{
	const char *location;
	gpointer event;
} ApplyScriptCBData;

static void ephy_greasemonkey_extension_install_cb (GtkAction *action,
						    EphyWindow *window);

static const GtkActionEntry action_entries [] =
{
	{ ACTION_NAME,
	  NULL,
	  N_("Install User _Script"),
	  NULL, /* shortcut key */
	  N_("Install Greasemonkey script"),
	  G_CALLBACK (ephy_greasemonkey_extension_install_cb) }
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

static const char *
get_script_dir (void)
{
	/* XXX: This is hardly error-proof */
	static char *script_dir = NULL;

	if (script_dir == NULL)
	{
		char *path;
		char *t;
		int ret;

		path = g_build_filename (ephy_dot_dir (), "extensions", NULL);
		if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE)
		{
			ret = mkdir (path, 0755);
			g_return_val_if_fail (ret == 0, NULL);
		}

		t = path;
		path = g_build_filename (path, "data", NULL);
		g_free (t);
		if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE)
		{
			ret = mkdir (path, 0755);
			g_return_val_if_fail (ret == 0, NULL);
		}

		t = path;
		path = g_build_filename (path, "greasemonkey", NULL);
		g_free (t);
		if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE)
		{
			ret = mkdir (path, 0755);
			g_return_val_if_fail (ret == 0, NULL);
		}

		script_dir = path;
	}

	return script_dir;
}

static GHashTable *
load_scripts (const char *path)
{
	DIR *d;
	struct dirent *e;
	char *file_path;
	GreasemonkeyScript *script;
	GHashTable *scripts;

	scripts = g_hash_table_new_full (g_str_hash, g_str_equal,
					 (GDestroyNotify) g_free,
					 (GDestroyNotify) g_object_unref);

	d = opendir (path);
	if (d == NULL)
	{
		return scripts;
	}

	while ((e = readdir (d)) != NULL)
	{
		if (g_str_has_suffix (e->d_name, ".user.js"))
		{
			file_path = g_build_filename (path, e->d_name, NULL);

			script = greasemonkey_script_new (file_path);
			g_hash_table_replace (scripts,
					      g_strdup (e->d_name), script);

			g_free (file_path);
		}
	}
	closedir (d);

	return scripts;
}

static void
dir_changed_cb (GnomeVFSMonitorHandle *handle,
		const char *monitor_uri,
		const char *info_uri,
		GnomeVFSMonitorEventType event_type,
		EphyGreasemonkeyExtension *extension)
{
	char *path;
	char *basename;
	GreasemonkeyScript *script;

	if (g_str_has_suffix (info_uri, ".user.js") == FALSE) return;

	path = gnome_vfs_get_local_path_from_uri (info_uri);
	basename = g_path_get_basename (path);

	switch (event_type)
	{
		case GNOME_VFS_MONITOR_EVENT_CREATED:
		case GNOME_VFS_MONITOR_EVENT_CHANGED:
			script = greasemonkey_script_new (path);
			g_hash_table_replace (extension->priv->scripts,
					      g_strdup (basename), script);
			break;
		case GNOME_VFS_MONITOR_EVENT_DELETED:
			g_hash_table_remove (extension->priv->scripts,
					     basename);
			break;
		default:
			break;
	}

	g_free (basename);
	g_free (path);
}

static GnomeVFSMonitorHandle *
monitor_scripts (const char *path,
		 EphyGreasemonkeyExtension *extension)
{
	char *uri;
	GnomeVFSMonitorHandle *monitor;
	GnomeVFSResult res;

	uri = gnome_vfs_get_uri_from_local_path (path);
	res = gnome_vfs_monitor_add (&monitor, path,
				     GNOME_VFS_MONITOR_DIRECTORY,
				     (GnomeVFSMonitorCallback) dir_changed_cb,
				     extension);
	g_free (uri);

	if (res != GNOME_VFS_OK)
	{
		return NULL;
	}

	return monitor;
}

static void
ephy_greasemonkey_extension_init (EphyGreasemonkeyExtension *extension)
{
	const char *path;

	extension->priv = EPHY_GREASEMONKEY_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyGreasemonkeyExtension initialising");

	path = get_script_dir ();

	extension->priv->scripts = load_scripts (path);
	extension->priv->monitor = monitor_scripts (path, extension);
}

static void
ephy_greasemonkey_extension_finalize (GObject *object)
{
	EphyGreasemonkeyExtension *extension = EPHY_GREASEMONKEY_EXTENSION (object);

	LOG ("EphyGreasemonkeyExtension finalising");

	g_hash_table_destroy (extension->priv->scripts);

	if (extension->priv->monitor != NULL)
	{
		gnome_vfs_monitor_cancel (extension->priv->monitor);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static char *
script_name_build (const char *url)
{
	char *basename;
	char *path;
	const char *script_dir;

	script_dir = get_script_dir ();
	g_return_val_if_fail (script_dir != NULL, NULL);

	basename = g_filename_from_utf8 (url, -1, NULL, NULL, NULL);
	g_return_val_if_fail (basename != NULL, NULL);

	g_strdelimit (basename, "/", '_');

	path = g_build_filename (script_dir, basename, NULL);

	g_free (basename);

	return path;
}

static void
save_source_completed_cb (EphyEmbedPersist *persist,
			  gpointer dummy)
{
	g_print ("Download to %s complete.\n",
		 ephy_embed_persist_get_dest (persist));
}

static void
ephy_greasemonkey_extension_install_cb (GtkAction *action,
					EphyWindow *window)
{
	EphyEmbed *embed;
	EphyEmbedPersist *persist;
	WindowData *data;
	const char *url;
	const char *filename;

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	url = data->last_clicked_url;
	g_return_if_fail (url != NULL);

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (embed != NULL);

	LOG ("Installing script at '%s'", url);

	filename = script_name_build (url);
	g_return_if_fail (filename != NULL);

	persist = EPHY_EMBED_PERSIST
		(ephy_embed_factory_new_object (EPHY_TYPE_EMBED_PERSIST));

	ephy_embed_persist_set_source (persist, url);
	ephy_embed_persist_set_embed (persist, embed);
	ephy_embed_persist_set_dest (persist, filename);

	g_signal_connect (persist, "completed",
			  G_CALLBACK (save_source_completed_cb), NULL);

	ephy_embed_persist_save (persist);

	g_object_unref (G_OBJECT (persist));
}

static EphyWindow *
ephy_window_for_ephy_embed (EphyEmbed *embed)
{
	EphyTab *tab;
	EphyWindow *window;

	tab = ephy_tab_for_embed (embed);
	g_return_val_if_fail (EPHY_IS_TAB (tab), NULL);

	window = EPHY_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab)));
	g_return_val_if_fail (EPHY_IS_WINDOW (window), NULL);

	return window;
}

static gboolean
dom_mouse_down_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphyGreasemonkeyExtension *extension)
{
	/*
	 * Set whether or not the action is visible before we display the
	 * context popup menu
	 */
	WindowData *window_data;
	EphyWindow *window;
	EphyEmbedEventContext context;
	GtkAction *action;
	const GValue *value;
	const char *url;
	gboolean show_install;

	context = ephy_embed_event_get_context (event);
	if ((context & EPHY_EMBED_CONTEXT_LINK) == 0)
	{
		return FALSE;
	}

	window = ephy_window_for_ephy_embed (embed);
	g_return_val_if_fail (window != NULL, FALSE);

	ephy_embed_event_get_property (event, "link", &value);
	url = g_value_get_string (value);

	show_install = g_str_has_suffix (url, ".user.js");

	window_data = (WindowData *) g_object_get_data (G_OBJECT (window),
							WINDOW_DATA_KEY);
	g_return_val_if_fail (window_data != NULL, FALSE);

	action = gtk_action_group_get_action (window_data->action_group,
					      ACTION_NAME);
	g_return_val_if_fail (action != NULL, FALSE);

	if (show_install == TRUE)
	{
		g_free (window_data->last_clicked_url);
		window_data->last_clicked_url = g_strdup (url);
	}

	gtk_action_set_visible (action, show_install);

	return FALSE;
}

static void
maybe_apply_script (const char *basename,
		    GreasemonkeyScript *script,
		    ApplyScriptCBData *data)
{
	char *script_str;

	if (greasemonkey_script_applies_to_url (script, data->location))
	{
		g_object_get (script, "script", &script_str, NULL);
		mozilla_evaluate_js (data->event, script_str);
		g_free (script_str);
	}
}

static void
content_loaded_cb (EphyEmbed *embed,
		   gpointer event,
		   EphyGreasemonkeyExtension *extension)
{
	ApplyScriptCBData *data;
	char *location;

	location = ephy_embed_get_location (embed, FALSE);
	if (location == NULL)
	{
		return;
	}

	data = g_new (ApplyScriptCBData, 1);
	data->location = location;
	data->event = event;

	g_hash_table_foreach (extension->priv->scripts,
			      (GHFunc) maybe_apply_script, data);

	g_free (location);
	g_free (data);
}

static void
free_window_data (WindowData *data)
{
	g_object_unref (data->action_group);
	g_free (data->last_clicked_url);
	g_free (data);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	guint ui_id;
	WindowData *data;

	LOG ("EphyGreasemonkeyExtension attach_window");

	data = g_new0 (WindowData, 1);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data->action_group = action_group =
		gtk_action_group_new ("EphyGreasemonkeyExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), window);

	gtk_ui_manager_insert_action_group (manager, action_group, 0);

	data->ui_id = ui_id = gtk_ui_manager_new_merge_id (manager);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) free_window_data);

	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "GreasemonkeySep1", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       ACTION_NAME, ACTION_NAME,
			       GTK_UI_MANAGER_MENUITEM, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "GreasemonkeySep2", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       "GreasemonkeySep1", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       ACTION_NAME, ACTION_NAME,
			       GTK_UI_MANAGER_MENUITEM, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       "GreasemonkeySep2", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	LOG ("EphyGreasemonkeyExtension detach_window");

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("impl_attach_tab");

	g_return_if_fail (EPHY_IS_TAB (tab));

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_connect (embed, "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb), ext);
	g_signal_connect (embed, "dom_content_loaded",
			  G_CALLBACK (content_loaded_cb), ext);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("impl_detach_tab");

	g_return_if_fail (EPHY_IS_TAB (tab));

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (dom_mouse_down_cb), ext);
	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (content_loaded_cb), ext);
}

static void
ephy_greasemonkey_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_greasemonkey_extension_class_init (EphyGreasemonkeyExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_greasemonkey_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyGreasemonkeyExtensionPrivate));
}

GType
ephy_greasemonkey_extension_get_type (void)
{
	return type;
}

GType
ephy_greasemonkey_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyGreasemonkeyExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_greasemonkey_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyGreasemonkeyExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_greasemonkey_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_greasemonkey_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyGreasemonkeyExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
