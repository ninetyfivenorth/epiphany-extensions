/*
 *  Copyright © 2005 Adam Hooper
 *  Copyright © 2008 Nuanti Ltd.
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
#include "ephy-debug.h"
#include "ephy-file-helpers.h"

#include <epiphany/epiphany.h>
#include <webkit/webkit.h>

#include <gtk/gtk.h>

#include <glib/gi18n-lib.h>
#include <glib.h>
#include <gio/gio.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#define EPHY_GREASEMONKEY_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GREASEMONKEY_EXTENSION, EphyGreasemonkeyExtensionPrivate))

#define WINDOW_DATA_KEY "EphyGreasemonkeyExtensionWindowData"
#define ACTION_NAME "EphyGreasemonkeyInstallScript"
#define ACTION_LINK_KEY "EphyGreasemonkeyScriptUrl"

struct _EphyGreasemonkeyExtensionPrivate
{
	GHashTable *scripts;
	GFileMonitor *monitor;
};

typedef struct
{
	EphyWindow *window;
	GtkActionGroup *action_group;
	GList *pending_downloads;
	guint ui_id;
	char *last_clicked_url;
	char *last_hovered_url;
} WindowData;

typedef struct
{
	WebKitWebView *web_view;
	const char *location;
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


static char *
get_script_dir (void)
{
	return g_build_filename (ephy_dot_dir (), "extensions", "data",
				 "greasemonkey", NULL);
}

static GHashTable *
load_scripts (const char *path)
{
	DIR *d;
	struct dirent *e;
	char *file_path;
	GreasemonkeyScript *script;
	GHashTable *scripts;
	int dir;
	const char *dirs[2];

	dirs[0] = "/usr/share/epiphany-extensions/greasemonkey/scripts";
	dirs[1] = path;

	scripts = g_hash_table_new_full (g_str_hash, g_str_equal,
					 (GDestroyNotify) g_free,
					 (GDestroyNotify) g_object_unref);
	for (dir = 0; dir < G_N_ELEMENTS(dirs); dir++)
	{
		d = opendir (dirs[dir]);
		if (d == NULL)
		{
			continue;
		}

		while ((e = readdir (d)) != NULL)
		{
			if (g_str_has_suffix (e->d_name, ".user.js"))
			{
				file_path = g_build_filename (dirs[dir], e->d_name, NULL);

				script = greasemonkey_script_new (file_path);
				g_hash_table_replace (scripts,
						      g_strdup (e->d_name), script);

				g_free (file_path);
			}
		}
		closedir (d);
	}
	return scripts;
}

static void
dir_changed_cb (GFileMonitor *monitor,
		GFile *file,
		GFile *other_file,
		GFileMonitorEvent event,
		EphyGreasemonkeyExtension *extension)
{
	char *uri;
	char *path;
	char *basename;
	GreasemonkeyScript *script;

	uri = g_file_get_uri (file);
	path = g_file_get_path (file);
	LOG ("Activity on %s", uri);

	if (g_str_has_suffix (uri, ".user.js") == FALSE) return;

	basename = g_file_get_basename (file);

	switch (event)
	{
		case G_FILE_MONITOR_EVENT_CREATED:
		case G_FILE_MONITOR_EVENT_CHANGED:
			script = greasemonkey_script_new (path);
			g_hash_table_replace (extension->priv->scripts,
					      g_strdup (basename), script);
			break;
		case G_FILE_MONITOR_EVENT_DELETED:
			g_hash_table_remove (extension->priv->scripts,
					     basename);
			break;
		default:
			break;
	}

	g_free (basename);
	g_free (uri);
	g_free (path);
}

static GFileMonitor *
monitor_scripts (const char *path,
		 EphyGreasemonkeyExtension *extension)
{
	GFileMonitor *monitor;
	GFile *file;

	file = g_file_new_for_path (path);
	monitor = g_file_monitor_directory (file,
					    0, NULL, NULL);
	if (monitor == NULL)
	{
		return NULL;
	}
	g_signal_connect (monitor, "changed",
			  G_CALLBACK (dir_changed_cb),
			  extension);

	g_object_unref (file);

	LOG ("Monitoring %s for user scripts", path);
	return monitor;
}

static void
ephy_greasemonkey_extension_init (EphyGreasemonkeyExtension *extension)
{
	char *path;

	extension->priv = EPHY_GREASEMONKEY_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyGreasemonkeyExtension initialising");

	path = get_script_dir ();
	errno = 0;
	if ((g_mkdir_with_parents (path, 0700) >= 0) ||
	    (errno == EEXIST))
	{
		extension->priv->scripts = load_scripts (path);
		extension->priv->monitor = monitor_scripts (path, extension);
	}

	g_free (path);
}

static void
ephy_greasemonkey_extension_finalize (GObject *object)
{
	EphyGreasemonkeyExtension *extension = EPHY_GREASEMONKEY_EXTENSION (object);

	LOG ("EphyGreasemonkeyExtension finalising");

	if (extension->priv->scripts != NULL)
	{
		g_hash_table_destroy (extension->priv->scripts);
	}

	if (extension->priv->monitor != NULL)
	{
		g_file_monitor_cancel (extension->priv->monitor);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static char *
script_name_build (const char *url)
{
	char *basename;
	char *path;
	char *script_dir;
	char *uri;

	basename = g_filename_from_utf8 (url, -1, NULL, NULL, NULL);
	g_return_val_if_fail (basename != NULL, NULL);

	g_strdelimit (basename, "/", '_');

	script_dir = get_script_dir ();

	path = g_build_filename (script_dir, basename, NULL);
	uri = g_filename_to_uri (path, NULL, NULL);

	g_free (script_dir);
	g_free (basename);
	g_free (path);

	return uri;
}

static void
save_source_error_cb (EphyDownload *download,
		      EphyWindow *window)
{
	WindowData *data;
	const char *dest;
	GFile *file;

	LOG ("Download from %s cancelled",
	     ephy_download_get_source_uri (download));

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	data->pending_downloads = g_list_remove (data->pending_downloads,
						 download);

	dest = ephy_download_get_destination_uri (download);
	file = g_file_new_for_uri (dest);
	g_file_delete (file, NULL, NULL);

	g_object_unref (file);
	g_object_unref (download);
}

static void
save_source_completed_cb (EphyDownload *download,
			  EphyWindow *window)
{
	WindowData *data;
	GtkWidget *dialog;
	const char *src;

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	data->pending_downloads = g_list_remove (data->pending_downloads,
						 download);

	src = ephy_download_get_source_uri (download);

	g_object_unref (download);

	dialog = gtk_message_dialog_new (GTK_WINDOW (window), 0,
					 GTK_MESSAGE_INFO,
					 GTK_BUTTONS_OK,
					 _("The user script at “%s” has "
					   "been installed"),
					 src);

	g_signal_connect_swapped (dialog, "response",
				  G_CALLBACK (gtk_widget_destroy), dialog);
	gtk_widget_show (dialog);
}

static void
ephy_greasemonkey_extension_install_cb (GtkAction *action,
					EphyWindow *window)
{
	EphyEmbed *embed;
	EphyDownload *download;
	WindowData *data;
	const char *url;
	char *filename;

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	url = data->last_clicked_url;
	g_return_if_fail (url != NULL);

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	g_return_if_fail (embed != NULL);

	LOG ("Installing script at '%s'", url);

	download = ephy_download_new_for_uri (url);

	filename = script_name_build (url);
	ephy_download_set_destination_uri (download, filename);
	g_free (filename);

	g_signal_connect (download, "completed",
			  G_CALLBACK (save_source_completed_cb), window);
	g_signal_connect (download, "error",
			  G_CALLBACK (save_source_error_cb), window);

	data->pending_downloads = g_list_prepend (data->pending_downloads,
						  download);

	ephy_download_start (download);
}

static void
hovering_over_link_cb (WebKitWebView *web_view,
		 const gchar *title,
		 const gchar *uri,
		 EphyGreasemonkeyExtension *extension)
{
	WindowData *window_data;
	EphyWindow *window;

	window = EPHY_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (web_view)));
	g_return_if_fail (window != NULL);

	window_data = (WindowData *) g_object_get_data (G_OBJECT (window),
							WINDOW_DATA_KEY);
	g_return_if_fail (window_data != NULL);

	g_free (window_data->last_hovered_url);
	window_data->last_hovered_url = g_strdup (uri);
}

static gboolean
button_press_event_cb (WebKitWebView *view,
		       GdkEventButton *event,
		       EphyGreasemonkeyExtension *extension)
{
	/*
	 * Set whether or not the action is visible before we display the
	 * context popup menu
	 */
	WindowData *window_data;
	EphyWindow *window;
	GtkAction *action;
	WebKitHitTestResult *hit_test;
	char *uri;
	guint context;
	gboolean show_install;

	if (event->button != 3 || event->type != GDK_BUTTON_PRESS)
		return FALSE;

	hit_test = webkit_web_view_get_hit_test_result (view, event);
	g_object_get (hit_test, "context", &context, NULL);

	if (!(context & WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK))
		return FALSE;

	g_object_get (hit_test, "link-uri", &uri, NULL);
	show_install = g_str_has_suffix (uri, ".user.js");

	window = EPHY_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view)));
	g_return_val_if_fail (window != NULL, FALSE);

	window_data = (WindowData *) g_object_get_data (G_OBJECT (window),
							WINDOW_DATA_KEY);
	g_return_val_if_fail (window_data != NULL, FALSE);

	action = gtk_action_group_get_action (window_data->action_group,
					      ACTION_NAME);
	g_return_val_if_fail (action != NULL, FALSE);

	if (show_install == TRUE)
	{
		g_free (window_data->last_clicked_url);
		window_data->last_clicked_url = g_strdup (uri);
	}

	gtk_action_set_visible (action, show_install);

	g_free (uri);
	g_object_unref (hit_test);

	return FALSE;
}

static void
maybe_apply_script (const char *basename,
		    GreasemonkeyScript *script,
		    ApplyScriptCBData *data)
{
	char *script_str;

	if (!greasemonkey_script_applies_to_url (script, data->location))
	    return;

	g_object_get (script, "script", &script_str, NULL);
	webkit_web_view_execute_script (data->web_view, script_str);
	g_free (script_str);
}

static void
load_status_notify_cb (GObject *object,
		       GParamSpec *spec,
		       EphyGreasemonkeyExtension *extension)
{
	ApplyScriptCBData *data;
	WebKitWebView *web_view;
	WebKitLoadStatus status;
	const char *location;

	web_view = WEBKIT_WEB_VIEW (object);

	status = webkit_web_view_get_load_status (web_view);
	if (status != WEBKIT_LOAD_FINISHED)
		return;

	location = webkit_web_view_get_uri (web_view);
	if (location == NULL)
		return;

	data = g_new (ApplyScriptCBData, 1);
	data->web_view = web_view;
	data->location = location;

	g_hash_table_foreach (extension->priv->scripts,
			      (GHFunc) maybe_apply_script, data);

	g_free (data);
}

static void
kill_download (EphyDownload *download,
	       EphyWindow *window)
{
	LOG ("kill_download %p", download);

	g_signal_handlers_disconnect_by_func
		(download, save_source_error_cb, window);
	g_signal_handlers_disconnect_by_func
		(download, save_source_completed_cb, window);

	ephy_download_cancel (download);
	g_object_unref (download);
}

static void
free_window_data (WindowData *data)
{
	LOG ("free_window_data %p", data);

	g_object_unref (data->action_group);
	g_free (data->last_clicked_url);

	g_list_foreach (data->pending_downloads,
			(GFunc) kill_download, data->window);
	g_list_free (data->pending_downloads);

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

	data->window = window;

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
		 EphyEmbed *embed)
{
	WebKitWebView *web_view;
	LOG ("impl_attach_tab");

	g_return_if_fail (EPHY_IS_EMBED (embed));

	web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);

	g_signal_connect (web_view, "button-press-event",
			  G_CALLBACK (button_press_event_cb), ext);

	g_signal_connect (web_view, "hovering-over-link",
			  G_CALLBACK (hovering_over_link_cb), ext);

	g_signal_connect (web_view, "notify::load-status",
			  G_CALLBACK (load_status_notify_cb), ext);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	WebKitWebView *web_view;
	LOG ("impl_detach_tab");

	g_return_if_fail (EPHY_IS_EMBED (embed));

	web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);

	g_signal_handlers_disconnect_by_func
		(web_view, G_CALLBACK (button_press_event_cb), ext);

	g_signal_handlers_disconnect_by_func
		(web_view, G_CALLBACK (hovering_over_link_cb), ext);

	g_signal_handlers_disconnect_by_func
		(web_view, G_CALLBACK (load_status_notify_cb), ext);
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
	const GTypeInfo our_info =
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

	const GInterfaceInfo extension_info =
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
