/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003 Christian Persch
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

#include "ephy-tab-states-extension.h"

#include <epiphany/epiphany.h>

#include "ephy-debug.h"

#include <gmodule.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#define EPHY_TAB_STATES_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TAB_STATES_EXTENSION, EphyTabStatesExtensionPrivate))

/* disable colours until they're themeable */
#undef ENABLE_COLOURS

struct _EphyTabStatesExtensionPrivate
{
#ifdef ENABLE_COLOURS
	GdkColor tab_loading_colour;
	GdkColor tab_unread_colour;
#endif
	PangoFontDescription *bold_font_desc;
};

static void ephy_tab_states_extension_class_init	(EphyTabStatesExtensionClass *klass);
static void ephy_tab_states_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_tab_states_extension_init		(EphyTabStatesExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_tab_states_extension_get_type (void)
{
	return type;
}

GType
ephy_tab_states_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyTabStatesExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tab_states_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTabStatesExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_tab_states_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_tab_states_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyTabStatesExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_tab_states_extension_init (EphyTabStatesExtension *extension)
{
	EphyTabStatesExtensionPrivate *priv;
#ifdef ENABLE_COLOURS
	char *colour;
	GSettings *settings;
#endif

	priv = extension->priv = EPHY_TAB_STATES_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyTabStatesExtension initialising");

#ifdef ENABLE_COLOURS
	settings = g_settings_new ("org.gnome.EpiphanyExtensions.tab-states");

	/* FIXME handle NULL and "" */
	colour = g_settings_get_string ("loading-color");
	gdk_color_parse (colour, &priv->tab_loading_colour);
	g_free (colour);

	colour = g_settings_get_string ("unread-color");
	gdk_color_parse (colour, &priv->tab_unread_colour);
	g_free (colour);

	g_object_unref (settings);
#endif

	priv->bold_font_desc = pango_font_description_new ();
	pango_font_description_set_weight (priv->bold_font_desc, PANGO_WEIGHT_BOLD);
}

static void
ephy_tab_states_extension_finalize (GObject *object)
{
	EphyTabStatesExtensionPrivate *priv = EPHY_TAB_STATES_EXTENSION (object)->priv;

	LOG ("EphyTabStatesExtension finalising");

	pango_font_description_free (priv->bold_font_desc);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GtkWidget *
get_real_tab_label (EphyWindow *window,
		    EphyEmbed *embed)
{
	GtkWidget *hbox, *label, *notebook;

	notebook = ephy_window_get_notebook (window);
	hbox = gtk_notebook_get_tab_label (GTK_NOTEBOOK (notebook),
					   GTK_WIDGET (embed));
	label = g_object_get_data (G_OBJECT (hbox), "label");
	g_return_val_if_fail (label != NULL, NULL);

	return label;
}

#ifdef ENABLE_COLOURS
static void
set_label_colour (GtkWidget *widget,
		  GdkColor *colour)
{
        gtk_widget_modify_fg (widget, GTK_STATE_NORMAL, colour);
        gtk_widget_modify_fg (widget, GTK_STATE_ACTIVE, colour);
}
#endif

static void
sync_active_tab (EphyWindow *window,
		 GParamSpec *pspec,
		 EphyTabStatesExtension *extension)
{
	EphyEmbed *embed;
	EphyWebView *view;
	GtkWidget *label;

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	view = ephy_embed_get_web_view (embed);

	if (ephy_web_view_is_loading (view) == FALSE)
	{
		/* mark the tab as read */
		label = get_real_tab_label (window, embed);
		gtk_widget_modify_font (label, NULL);
#ifdef ENABLE_COLOURS
		set_label_colour (label, NULL);
#endif
	}
}

static void
sync_load_status (EphyWebView *view,
		  GParamSpec *pspec,
		  EphyTabStatesExtension *extension)
{
	EphyTabStatesExtensionPrivate *priv = extension->priv;
	EphyWindow *window;
	EphyEmbed *embed;
	GtkWidget *label;
	PangoFontDescription *font_desc = NULL;
	gboolean loading;
#ifdef ENABLE_COLOURS
	GdkColor *colour = NULL;
#endif

	window = EPHY_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view)));
	g_return_if_fail (window != NULL);

	embed = EPHY_GET_EMBED_FROM_EPHY_WEB_VIEW (view);
	loading = ephy_web_view_is_loading (view);
	if (loading)
	{
#ifdef ENABLE_COLOURS
		colour = &priv->tab_loading_colour;
#endif
		font_desc = priv->bold_font_desc;
	}
	else if (embed != ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window)))
	{
#ifdef ENABLE_COLOURS
		colour = &priv->tab_unread_colour;
#endif
		font_desc = priv->bold_font_desc;
	}

	label = get_real_tab_label (window, embed);
	gtk_widget_modify_font (label, font_desc);
#ifdef ENABLE_COLOURS
	set_label_colour (label, colour);
#endif
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyTabStatesExtension *extension = EPHY_TAB_STATES_EXTENSION (ext);

	LOG ("attach_window window %p", window);

	g_signal_connect (window, "notify::active-child",
			  G_CALLBACK (sync_active_tab), extension);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyTabStatesExtension *extension = EPHY_TAB_STATES_EXTENSION (ext);

	LOG ("Edetach_window window %p", window);

	g_signal_handlers_disconnect_by_func
		(window, G_CALLBACK (sync_active_tab), extension);
}


static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	EphyTabStatesExtension *extension = EPHY_TAB_STATES_EXTENSION (ext);
	WebKitWebView *web_view;
	EphyWebView *view;

	LOG ("impl_attach_tab window %p tab %p", window, embed);

	web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);
	view = EPHY_WEB_VIEW (web_view);

	sync_load_status (view, NULL, extension);
	g_signal_connect (view, "notify::load-status",
			  G_CALLBACK (sync_load_status), extension);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	EphyTabStatesExtension *extension = EPHY_TAB_STATES_EXTENSION (ext);
	WebKitWebView *web_view;
	EphyWebView *view;

	LOG ("impl_detach_tab window %p tab %p", window, embed);

	web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);
	view = EPHY_WEB_VIEW (web_view);

	g_signal_handlers_disconnect_by_func
		(view, G_CALLBACK (sync_load_status), extension);
}

static void
ephy_tab_states_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_tab_states_extension_class_init (EphyTabStatesExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_tab_states_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyTabStatesExtensionPrivate));
}
