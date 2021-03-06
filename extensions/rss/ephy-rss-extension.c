/*
 *  Copyright © 2005 Raphaël Slinckx <raphael@slinckx.net>
 *  Copyright © 2010 Igalia S.L.
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

#include "ephy-rss-extension.h"
#include "rss-ui.h"
#include "rss-feedlist.h"

#include "ephy-debug.h"

#include <epiphany/epiphany.h>

#include <gtk/gtk.h>

#include <glib/gi18n-lib.h>

#include <dbus/dbus-glib.h>

#define EPHY_RSS_EXTENSION_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE((object), EPHY_TYPE_RSS_EXTENSION, EphyRssExtensionPrivate))

struct _EphyRssExtensionPrivate
{
	RssUI *dialog;
	/* The proxy to the rss reader */
	DBusGProxy *proxy;
};

static GObjectClass *parent_class = NULL;
static GType type = 0;

/* Menu item stuff */
#define WINDOW_DATA_KEY	"EphyRssExtensionWindowData"
#define EPHY_RSS_EXTENSION_DATA_KEY "EphyRssExtension"
#define MENU_PATH	"/ui/PagePopup/ExtensionsMenu"

/* Status Bar stuff */
#define FEED_ICON	"feed-presence"

static void ephy_rss_display_cb 	(GtkAction *action,
					 EphyWindow *window);
#if 0
static void ephy_rss_update_statusbar 	(EphyWindow *window,
					 gboolean show);
#endif
static void ephy_rss_update_action 	(EphyWindow *window);
static void ephy_rss_feed_subscribe_cb (GtkAction *action,
					 EphyWindow *window);
typedef struct
{
	EphyRssExtension *extension;
	GtkActionGroup *action_group;
	GtkAction *info_action;
	GtkAction *subscribe_action;
	guint ui_id;

	GtkWidget *evbox;
} WindowData;

static const GtkActionEntry action_entries [] =
{
	{ "RssInfo",
	  NULL,
	  N_("News Feeds Subscription"),
	  NULL,
	  N_("Subscribe to this website's news feeds in your favorite news reader"),
	  G_CALLBACK (ephy_rss_display_cb)
	},
	{ "RssSubscribe",
	  NULL,
	  N_("_Subscribe to this feed"),
	  NULL,
	  N_("Subscribe to this feed in your favorite news reader"),
	  G_CALLBACK (ephy_rss_feed_subscribe_cb)
	},
};

/* We got an rss feed from a tab */
static void
ephy_rss_ge_feed_cb (EphyWebView *view,
		     const char *type,
		     const char *title,
		     const char *address,
		     EphyWindow *window)
{
	FeedList *list;

	list = (FeedList *) g_object_steal_data (G_OBJECT (view), FEEDLIST_DATA_KEY);
	list = rss_feedlist_add (list, type, title, address);
	g_object_set_data_full (G_OBJECT (view), FEEDLIST_DATA_KEY, list,
				(GDestroyNotify) rss_feedlist_free);

	LOG ("Got a new feed for the site: type=%s, title=%s, address=%s\nWe now have %d feeds", type, title, address, rss_feedlist_length (list));

	ephy_rss_update_action (window);
}

static void
ephy_rss_feed_subscribe_cb (GtkAction *action,
			    EphyWindow *window)
{
	GError *error = NULL;
	gboolean success = FALSE;
	EphyRssExtension *extension;
	EphyEmbed *embed;
	WebKitWebView *view;
	GdkEventButton *event;
	WebKitHitTestResult *hit_test;
	char *uri;

	LOG ("Subscribing to the feed");

	extension = EPHY_RSS_EXTENSION (g_object_get_data (G_OBJECT (window), EPHY_RSS_EXTENSION_DATA_KEY));

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);

	event = (GdkEventButton *) (g_object_get_data (G_OBJECT (window),
				                     "rss-event"));

	hit_test = webkit_web_view_get_hit_test_result (view, event);
	gdk_event_free ((GdkEvent *) event);

	g_object_get (hit_test, "link-uri", &uri, NULL);
	g_object_unref (hit_test);

	dbus_g_proxy_call (extension->priv->proxy,
			   RSS_DBUS_SUBSCRIBE, &error,
			   G_TYPE_STRING, uri,
			   G_TYPE_INVALID,
			   G_TYPE_BOOLEAN, &success,
			   G_TYPE_INVALID);

	if (!success)
	{
		LOG ("Error while retreiving method answer: %s", error->message);
		g_error_free (error);
	}

	g_object_set (action, "sensitive", FALSE, "visible", FALSE, NULL);
}

static gboolean
ephy_rss_button_press_cb (EphyWebView *view,
			  GdkEventButton *event,
			  EphyWindow *window)
{
	WindowData *data;
	FeedList *list;
	WebKitHitTestResult *hit_test;
	guint context;
	char *uri;
	gboolean active = FALSE;

	if (event->button != 3 || event->type != GDK_BUTTON_PRESS)
		return FALSE;

	list = g_object_get_data (G_OBJECT (view), FEEDLIST_DATA_KEY);
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_val_if_fail (data != NULL, FALSE);

	hit_test = webkit_web_view_get_hit_test_result (WEBKIT_WEB_VIEW (view), event);
	g_object_get (hit_test, "context", &context, NULL);

	if ((context & WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK) && (list != NULL))
	{
		g_object_get (hit_test, "link-uri", &uri, NULL);
		active = rss_feedlist_contains (list, uri);
		g_free (uri);

		g_object_set_data (G_OBJECT (window), "rss-event",
				   gdk_event_copy ((GdkEvent *) event));

		g_object_set (data->subscribe_action,
			      "sensitive", active,
			      "visible", active,
			      NULL);
	}
	g_object_unref (hit_test);

	return FALSE;
}

static void
ephy_rss_dialog_display (EphyWindow *window)
{
	EphyRssExtensionPrivate *priv;
	EphyEmbed *embed;
	EphyWebView *view;
	FeedList *list;
	WindowData *data;

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	priv = data->extension->priv;

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	g_return_if_fail (embed != NULL);
	view = ephy_embed_get_web_view (embed);
	g_return_if_fail (view != NULL);

	list = g_object_get_data (G_OBJECT (view), FEEDLIST_DATA_KEY);
	g_return_if_fail (list != NULL);

	if (priv->dialog == NULL)
	{
		LOG ("Trying to build dialog");
		priv->dialog = rss_ui_new (list, embed, data->extension);
	}

	ephy_dialog_set_parent (EPHY_DIALOG (priv->dialog),
				GTK_WIDGET (window));
	ephy_dialog_show (EPHY_DIALOG (priv->dialog));
}

/* Called when the user clicks on our menu item */
static void
ephy_rss_display_cb (GtkAction *action,
		     EphyWindow *window)
{
	ephy_rss_dialog_display (window);
}

/* When the pages changes, invalidate the previous feeds */
static void
ephy_rss_ge_content_cb (EphyEmbed *embed,
			char *uri,
			EphyWindow *window)
{
	LOG ("Page changed, invalidating previous feeds");

	g_object_set_data (G_OBJECT (embed), FEEDLIST_DATA_KEY, NULL);

	ephy_rss_update_action (window);
}

#if 0
static void
ephy_rss_update_statusbar (EphyWindow *window,
			   gboolean show)
{
	WindowData *data;

	/* Show / Hide statusbar icon */
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	g_object_set (data->evbox, "visible", show, NULL);
}
#endif

/* Update the menu item availability */
static void
ephy_rss_update_action (EphyWindow *window)
{
	WindowData *data;
	FeedList *list;
	gboolean show = TRUE;
	EphyEmbed *embed;
	EphyWebView *view;

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	g_return_if_fail (embed != NULL);
	view = ephy_embed_get_web_view (embed);
	g_return_if_fail (view != NULL);

	/* The page is loaded, do we have a feed ? */
	list = g_object_get_data (G_OBJECT (view), FEEDLIST_DATA_KEY);
	show = rss_feedlist_length (list) > 0;

	/* Disable the menu item when loading the page */
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	g_object_set (data->info_action, "sensitive", show, NULL);

#if 0
	ephy_rss_update_statusbar (window, show);
#endif

	g_object_set (data->subscribe_action,
		      "sensitive", show,
		      "visible", show,
		      NULL);
}

/* Called when the user changes tab */
static void
ephy_rss_sync_active_tab (EphyWindow *window,
			  GParamSpec *pspec,
			  gpointer data)
{
	if (gtk_widget_get_realized (GTK_WIDGET (window)) == FALSE)
		return; /* on startup */

	ephy_rss_update_action (window);
}

/* Attach the callback to this new tab */
static void
impl_attach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	EphyWebView *view;

	LOG ("Attach rss listener to tab");

	g_return_if_fail (EPHY_IS_EMBED (embed));

	view = ephy_embed_get_web_view (embed);

	/* Notify when a new rss feed is parsed */
	g_signal_connect_after (view, "new-document-now",
				G_CALLBACK (ephy_rss_ge_content_cb), window);
	g_signal_connect_after (view, "ge-feed-link",
				G_CALLBACK (ephy_rss_ge_feed_cb), window);
	g_signal_connect (view, "button-press-event",
			  G_CALLBACK (ephy_rss_button_press_cb), window);
}

/* Stop listening for the detached tab rss feeds */
static void
impl_detach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	EphyWebView *view;

	LOG ("Detach tab rss listener");

	g_return_if_fail (EPHY_IS_EMBED (embed));

	view = ephy_embed_get_web_view (embed);

	/* We don't want any new rss notif for this tab */
	g_signal_handlers_disconnect_by_func
		(view, G_CALLBACK (ephy_rss_ge_feed_cb), window);

	g_signal_handlers_disconnect_by_func
		(view, G_CALLBACK (ephy_rss_ge_content_cb), window);

	g_signal_handlers_disconnect_by_func
		(view, G_CALLBACK (ephy_rss_button_press_cb), window);

	/* destroy data */
	g_object_set_data (G_OBJECT (embed), FEEDLIST_DATA_KEY, NULL);
}

#if 0
static gboolean
ephy_rss_statusbar_icon_clicked_cb (GtkWidget *widget,
				    GdkEventButton *event,
				    EphyWindow *window)
{
	if (event->button == 1)
	{
		ephy_rss_dialog_display (window);

		return TRUE;
	}

	return FALSE;
}

static void
ephy_rss_create_statusbar_icon (EphyWindow *window,
				WindowData *data)
{
	EphyStatusbar *statusbar;
	GtkWidget *icon;

	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	data->evbox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (data->evbox), FALSE);

	icon = gtk_image_new_from_icon_name (FEED_ICON, GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (data->evbox), icon);
	gtk_widget_show (icon);

	gtk_widget_set_tooltip_text (data->evbox, _("Subscribe to site's news feed"));

	ephy_statusbar_add_widget (statusbar, data->evbox);

	g_signal_connect_after (data->evbox, "button-press-event",
				G_CALLBACK (ephy_rss_statusbar_icon_clicked_cb),
				window);
}

static void
ephy_rss_destroy_statusbar_icon (EphyWindow *window,
				 WindowData *data)
{
	EphyStatusbar *statusbar;

	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	g_return_if_fail (data->evbox != NULL);

	ephy_statusbar_remove_widget (statusbar, GTK_WIDGET (data->evbox));
}
#endif

/* Create the menu item to subscribe to an rss feed */
static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyRssExtension *extension = EPHY_RSS_EXTENSION (ext);
	GtkActionGroup *action_group;
	GtkUIManager *manager;
	guint ui_id;
	WindowData *data;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	action_group = gtk_action_group_new ("EphyRssExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), window);

	gtk_ui_manager_insert_action_group (manager, action_group, -1);
	g_object_unref (action_group);

	ui_id = gtk_ui_manager_new_merge_id (manager);

	/* Add feed info to tools menu */
	gtk_ui_manager_add_ui (manager, ui_id, MENU_PATH,
			       "RssInfoSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, MENU_PATH,
			       "RssInfo", "RssInfo",
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	/* Add subscribe to popup context (normal document) */
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "RssInfoSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "RssSubscribe", "RssSubscribe",
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	/* store data */
	data = g_new (WindowData, 1);

	data->extension = extension;
	data->action_group = action_group;
	data->info_action = gtk_action_group_get_action (action_group, "RssInfo");
	data->subscribe_action = gtk_action_group_get_action (action_group, "RssSubscribe");
	data->ui_id = ui_id;

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) g_free);

	g_object_set_data (G_OBJECT (window), EPHY_RSS_EXTENSION_DATA_KEY, extension);

#if 0
	/* Create the status bar icon */
	ephy_rss_create_statusbar_icon (window, data);
#endif

	/* Register for tab switch events */
	ephy_rss_sync_active_tab (window, NULL, NULL);
	g_signal_connect_after (window, "notify::active-child",
				G_CALLBACK (ephy_rss_sync_active_tab), NULL);
}

/* Delete the menu item to subscribe to a feed */
static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	/* Remove the menu item */
	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	/* Remove the tab switch notification */
	g_signal_handlers_disconnect_by_func
		(window, G_CALLBACK (ephy_rss_sync_active_tab), NULL);

#if 0
	/* Remove the status bar icon */
	ephy_rss_destroy_statusbar_icon (window, data);
#endif

	/* Destroy data */
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
	g_object_set_data (G_OBJECT (window), EPHY_RSS_EXTENSION_DATA_KEY, NULL);
}

/* Epiphany init/dispose stuff ------------------------- */
static void
ephy_rss_extension_init (EphyRssExtension *extension)
{
	DBusGConnection *connection;
	GError *error = NULL;
	extension->priv = EPHY_RSS_EXTENSION_GET_PRIVATE (extension);

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL)
	{
		LOG ("No connection to dbus:%s", error->message);
		g_error_free (error);
		return;
	}

	extension->priv->proxy = dbus_g_proxy_new_for_name (connection,
                                     RSS_DBUS_SERVICE,
                                     RSS_DBUS_OBJECT_PATH,
                                     RSS_DBUS_INTERFACE);
}

static void
ephy_rss_extension_finalize (GObject *object)
{
	EphyRssExtension *extension = EPHY_RSS_EXTENSION (object);

	g_object_unref (extension->priv->proxy);

	/* Dispose the dialog */
	if (extension->priv->dialog != NULL)
		g_object_unref (extension->priv->dialog);

	parent_class->finalize (object);
}

static void
ephy_rss_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_rss_extension_class_init (EphyRssExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_rss_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyRssExtensionPrivate));
}

GType
ephy_rss_extension_get_type (void)
{
	return type;
}

GType
ephy_rss_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyRssExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_rss_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyRssExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_rss_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_rss_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyRssExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

RssUI *
ephy_rss_extension_get_dialog (EphyRssExtension *extension)
{
	return extension->priv->dialog;
}

void
ephy_rss_extension_set_dialog (EphyRssExtension *extension, RssUI *dialog)
{
	extension->priv->dialog = dialog;
}
