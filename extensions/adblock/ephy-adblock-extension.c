/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#include <epiphany/ephy-embed-shell.h>
#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-statusbar.h>
#include <epiphany/ephy-window.h>

#include "ephy-adblock-extension.h"
#include "ephy-debug.h"

#include "ad-blocker.h"
#include "ad-uri-tester.h"

#include "mozilla/mozilla-helpers.h"

#include <gtk/gtkeventbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkimage.h>
#include <gtk/gtknotebook.h>

#include <gmodule.h>

#include <glib/gi18n-lib.h>

#define EPHY_ADBLOCK_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_ADBLOCK_EXTENSION, EphyAdblockExtensionPrivate))

#define STATUSBAR_FRAME_KEY	"EphyAdblockExtensionStatusbarFrame"
#define STATUSBAR_EVBOX_KEY	"EphyAdblockExtensionStatusbarEvbox"
#define ICON_FILENAME		"adblock-statusbar-icon.svg"

struct EphyAdblockExtensionPrivate
{
	AdUriTester *tester;
};

static void ephy_adblock_extension_class_init	(EphyAdblockExtensionClass *klass);
static void ephy_adblock_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_adblock_extension_init		(EphyAdblockExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_adblock_extension_get_type (void)
{
	return type;
}

GType
ephy_adblock_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyAdblockExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_adblock_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyAdblockExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_adblock_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_adblock_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyAdblockExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_adblock_extension_init (EphyAdblockExtension *extension)
{
	LOG ("EphyAdblockExtension initialising")

	ephy_embed_shell_get_embed_single (embed_shell); /* Fire up Mozilla */

	mozilla_register_ad_blocker ();

	extension->priv = EPHY_ADBLOCK_EXTENSION_GET_PRIVATE (extension);

	extension->priv->tester = ad_uri_tester_new ();
}

static void
ephy_adblock_extension_finalize (GObject *object)
{
	EphyAdblockExtension *extension = EPHY_ADBLOCK_EXTENSION (object);

	LOG ("EphyAdblockExtension finalising")

	mozilla_unregister_ad_blocker ();

	g_object_unref (extension->priv->tester);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
update_statusbar (EphyWindow *window)
{
	AdBlocker *blocker;
	EphyEmbed *embed;
	GObject *statusbar;
	GtkWidget *evbox;
	GtkWidget *frame;
	int num_blocked;

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (embed != NULL);

	blocker = g_object_get_data (G_OBJECT (embed), AD_BLOCKER_KEY);
	g_return_if_fail (blocker != NULL);

	statusbar = G_OBJECT (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	frame = g_object_get_data (statusbar, STATUSBAR_FRAME_KEY);
	g_return_if_fail (frame != NULL);

	evbox = g_object_get_data (statusbar, STATUSBAR_EVBOX_KEY);
	g_return_if_fail (evbox != NULL);

	g_object_get (G_OBJECT (blocker), "num-blocked", &num_blocked, NULL);

	if (num_blocked == 0)
	{
		gtk_widget_hide (frame);
	}
	else
	{
		char *tooltip;

		tooltip = g_strdup_printf (ngettext ("%d hidden advertisement",
						     "%d hidden advertisements",
						     num_blocked),
					   num_blocked);

		gtk_tooltips_set_tip ((EPHY_STATUSBAR(statusbar))->tooltips,
				      evbox, tooltip, NULL);

		g_free (tooltip);

		gtk_widget_show (frame);
	}
}

static void
create_statusbar_icon (EphyWindow *window)
{
	EphyStatusbar *statusbar;
	char *filename;
	GdkPixbuf *pixbuf;
	GtkWidget *frame;
	GtkWidget *icon;
	GtkWidget *evbox;
	int w, h;

	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);

	filename = g_build_filename (SHARE_DIR, ICON_FILENAME, NULL);
	pixbuf = gdk_pixbuf_new_from_file_at_size (filename, w, h, NULL);
	g_free (filename);
	g_return_if_fail (pixbuf != NULL);

	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	frame = gtk_frame_new (NULL);

	icon = gtk_image_new_from_pixbuf (pixbuf);

	evbox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (evbox), FALSE);

	gtk_container_add (GTK_CONTAINER (frame), evbox);
	gtk_container_add (GTK_CONTAINER (evbox), icon);

	gtk_widget_show (evbox);
	gtk_widget_show (icon);
	/* don't show the frame */

	ephy_statusbar_add_widget (statusbar, frame);

	g_object_set_data (G_OBJECT (statusbar), STATUSBAR_FRAME_KEY, frame);
	g_object_set_data (G_OBJECT (statusbar), STATUSBAR_EVBOX_KEY, evbox);

	g_object_unref (pixbuf);
}

static void
destroy_statusbar_icon (EphyWindow *window)
{
	EphyStatusbar *statusbar;
	GtkWidget *frame;
	GtkWidget *evbox;

	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	frame = g_object_get_data (G_OBJECT (statusbar), STATUSBAR_FRAME_KEY);
	evbox = g_object_get_data (G_OBJECT (statusbar), STATUSBAR_EVBOX_KEY);

	g_object_set_data (G_OBJECT (statusbar), STATUSBAR_FRAME_KEY, NULL);
	g_object_set_data (G_OBJECT (statusbar), STATUSBAR_EVBOX_KEY, NULL);

	g_return_if_fail (frame != NULL);
	g_return_if_fail (evbox != NULL);

	gtk_tooltips_set_tip (statusbar->tooltips, evbox, NULL, NULL);

	ephy_statusbar_remove_widget (statusbar, frame);
}

static void
switch_page_cb (GtkNotebook *notebook,
		GtkNotebookPage *page,
		guint page_num,
		EphyWindow *window)
{
	if (GTK_WIDGET_REALIZED (window) == FALSE) return; /* on startup */

	update_statusbar (window);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	create_statusbar_icon (window);

	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (switch_page_cb), window);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (switch_page_cb), window);

	destroy_statusbar_icon (window);
}

static void
location_changed_cb (EphyEmbed *embed,
		     const char *address,
		     AdBlocker *blocker)
{
	ad_blocker_reset (blocker);
}

static void
num_blocked_cb (AdBlocker *blocker,
		GParamSpec *pspec,
		EphyEmbed *embed)
{
	EphyTab *tab;
	EphyWindow *window;

	tab = ephy_tab_for_embed (embed);
	g_return_if_fail (tab != NULL);

	window = EPHY_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab)));
	g_return_if_fail (window != NULL);

	if (embed == ephy_window_get_active_embed (window))
	{
		update_statusbar (window);
	}
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyAdblockExtension *extension = EPHY_ADBLOCK_EXTENSION (ext);
	AdBlocker *blocker;
	EphyEmbed *embed;
	
	embed = ephy_tab_get_embed (tab);

	blocker = ad_blocker_new (extension->priv->tester);
	g_return_if_fail (blocker != NULL);

	g_object_set_data (G_OBJECT (embed), AD_BLOCKER_KEY, blocker);

	g_signal_connect (G_OBJECT (embed), "ge-location",
			  G_CALLBACK (location_changed_cb), blocker);

	g_signal_connect (G_OBJECT (blocker), "notify::num-blocked",
			  G_CALLBACK (num_blocked_cb), embed);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	AdBlocker *blocker;
	EphyEmbed *embed;
	
	embed = ephy_tab_get_embed (tab);

	blocker = g_object_get_data (G_OBJECT (embed), AD_BLOCKER_KEY);
	g_return_if_fail (blocker != NULL);

	g_signal_handlers_disconnect_by_func
		(G_OBJECT (embed), G_CALLBACK (location_changed_cb), blocker);
	g_signal_handlers_disconnect_by_func
		(G_OBJECT (blocker), G_CALLBACK (num_blocked_cb), blocker);

	g_object_unref (blocker);
}

static void
ephy_adblock_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_adblock_extension_class_init (EphyAdblockExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_adblock_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyAdblockExtensionPrivate));
}
