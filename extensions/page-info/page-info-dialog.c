/*
 *  Copyright (C) 2004 Adam Hooper
 *  Copyright (C) 2004 Christian Persch
 *
 *  Heavily based on Galeon's:
 *      Copyright (C) 2003 Philip Langdale
 *
 *  ...in turn, Galeon's was based on PDM Dialog:
 *  	Copyright (C) 2002 Jorn Baayen
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "page-info-dialog.h"
#include "ephy-debug.h"

#include <epiphany/ephy-embed-persist.h>
// #include <epiphany/ephy-embed-factory.h>

/* non-installed ephy headers */
#include "ephy-state.h"

#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkstock.h>

#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>

#include "mozilla/mozilla-helpers.h"

#include <glib/gi18n.h>

#if 0
#include <gtk/gtkliststore.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkclipboard.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkimagemenuitem.h>
#include <gdk/gdkdisplay.h>
#include <gtk/gtkmain.h>
#include <glib/gconvert.h>
#endif

#include <time.h>

/* FIXME: ephy-embed-factory.h isn't released with Epiphany 1.4 */
GObject *ephy_embed_factory_new_object (const char *object_id);

/*
#define STATE_PAGE_INFO_IMAGE_PANED_SIZE "page_info_dialog/page_info_image_pane_size"
#define STATE_PAGE_INFO_IMAGE_PANED_SIZE_DEFAULT 250
*/

#define COLUMN_KEY "PageInfoURLColumn"

/* Glade callbacks */
void page_info_dialog_close_button_clicked_cb	  (GtkWidget *button,
						   PageInfoDialog *dialog);
void page_info_dialog_view_cert_button_clicked_cb (GtkWidget *button,
						   PageInfoDialog *dialog);
void page_info_dialog_imagesave_button_clicked_cb (GtkWidget *button,
						   PageInfoDialog *dialog);
void page_info_image_box_realize_cb		  (GtkContainer *box,
						   PageInfoDialog *dialog);

enum {
	GENERAL_PAGE,
	IMAGES_PAGE,
	LINKS_PAGE,
	FORMS_PAGE,
	/*
	PAGE_INFO_MEDIA,
	*/
	/* PAGE_INFO_STYLESHEETS, */
	/* PAGE_INFO_PRIVACY, */
	/*PAGE_INFO_SECURITY,*/

	LAST_PAGE
};

typedef struct _InfoPage	InfoPage;

struct _InfoPage
{
	/* Methods */
	void	(* construct)	(InfoPage *info);
	void	(* fill)	(InfoPage *info);

	/* Data */
	PageInfoDialog *dialog;
};

#define PAGE_INFO_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_PAGE_INFO_DIALOG, PageInfoDialogPrivate))

struct _PageInfoDialogPrivate
{
	InfoPage *pages[LAST_PAGE];
	GtkWidget *dialog;
	EphyWindow *window;
	EphyEmbed *embed;
	GtkUIManager *manager;
	GtkActionGroup *action_group;
};

enum
{
	PROP_0,
	PROP_EMBED,
	PROP_WINDOW
};

enum
{
	PROP_DIALOG,
	PROP_NOTEBOOK,

	PROP_GENERAL_PAGE_TITLE,
	PROP_GENERAL_URL,
	PROP_GENERAL_TYPE,
	PROP_GENERAL_MIME_TYPE,
	PROP_GENERAL_RENDER_MODE,
	PROP_GENERAL_SOURCE,
	PROP_GENERAL_ENCODING,
	PROP_GENERAL_SIZE,
	PROP_GENERAL_REFERRING_URL,
	PROP_GENERAL_MODIFIED,
	PROP_GENERAL_EXPIRES,
/*
	PROP_GENERAL_META_TREEVIEW,
*/
	PROP_FORMS_FORM_TREEVIEW,

	PROP_LINKS_LINK_TREEVIEW,

	PROP_IMAGES_IMAGE_TREEVIEW,
	PROP_IMAGES_IMAGE_BOX,
	PROP_IMAGES_IMAGE_VPANED,
	PROP_IMAGES_SAVE_BUTTON,

/*
	PROP_SECURITY_CERT_TITLE,
	PROP_SECURITY_CERT_INFO,
	PROP_SECURITY_CIPHER_TITLE,
	PROP_SECURITY_CIPHER_INFO,
	PROP_SECURITY_VIEW_CERT_VBOX
*/
};

static const
EphyDialogProperty properties [] =
{
	{ "page_info_dialog",		NULL, PT_NORMAL, 0 },
	{ "page_info_notebook",		NULL, PT_NORMAL, 0 },

	{ "page_info_page_title",	NULL, PT_NORMAL, 0 },
	{ "page_info_url",		NULL, PT_NORMAL, 0 },
	{ "page_info_type",		NULL, PT_NORMAL, 0 },
	{ "page_info_mime",		NULL, PT_NORMAL, 0 },
	{ "page_info_render_mode",	NULL, PT_NORMAL, 0 },
	{ "page_info_source",		NULL, PT_NORMAL, 0 },
	{ "page_info_encoding",		NULL, PT_NORMAL, 0 },
	{ "page_info_size",		NULL, PT_NORMAL, 0 },
	{ "page_info_referring_url",	NULL, PT_NORMAL, 0 },
	{ "page_info_modified",		NULL, PT_NORMAL, 0 },
	{ "page_info_expires",		NULL, PT_NORMAL, 0 },
	/*{ "page_info_meta_list",	NULL, PT_NORMAL, 0 },*/

	{ "page_info_form_list",	NULL, PT_NORMAL, 0 },

	{ "page_info_link_list",	NULL, PT_NORMAL, 0 },

	{ "page_info_image_list",	NULL, PT_NORMAL, 0 },
	{ "page_info_image_box",	NULL, PT_NORMAL, 0 },
	{ "page_info_image_vpaned",	NULL, PT_NORMAL, 0 },
	{ "page_info_image_save",	NULL, PT_NORMAL, 0 },

	/*
	{ PROP_SECURITY_CERT_TITLE,     "page_info_security_title", NULL, PT_NORMAL, NULL },
	{ PROP_SECURITY_CERT_INFO,      "page_info_security_info",  NULL, PT_NORMAL, NULL },
	{ PROP_SECURITY_CIPHER_TITLE,   "page_info_cipher_title",   NULL, PT_NORMAL, NULL },
	{ PROP_SECURITY_CIPHER_INFO,    "page_info_cipher_info",    NULL, PT_NORMAL, NULL },
	{ PROP_SECURITY_VIEW_CERT_VBOX, "page_info_view_cert_vbox", NULL, PT_NORMAL, NULL },
	*/

	{ NULL }
};

/*
enum
{
	COL_META_NAME,
	COL_META_CONTENT
};
*/

static void page_info_dialog_class_init	(PageInfoDialogClass *klass);
static void page_info_dialog_init	(PageInfoDialog *dialog);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
page_info_dialog_get_type (void)
{
	return type;
}

GType 
page_info_dialog_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (PageInfoDialogClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) page_info_dialog_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (PageInfoDialog),
		0, /* n_preallocs */
		(GInstanceInitFunc) page_info_dialog_init
	};

	type = g_type_module_register_type (module,
					    EPHY_TYPE_DIALOG,
					    "PageInfoDialog",
					    &our_info, 0);

	return type;
}

/* not-yet ported stuff */

/*
static GtkTreeView *
setup_meta_treeview (PageInfoDialog *dialog)
{
	GtkTreeView *treeview;
	GtkListStore *liststore;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	treeview = GTK_TREE_VIEW(galeon_dialog_get_control 
				 (GALEON_DIALOG(dialog),
				 PROP_GENERAL_META_TREEVIEW));
	
*/
	/* set tree model */
/*
	liststore = gtk_list_store_new (2,
					G_TYPE_STRING,
					G_TYPE_STRING);
	gtk_tree_view_set_model (treeview, GTK_TREE_MODEL(liststore));
	g_object_unref (liststore);

	gtk_tree_view_set_headers_visible (treeview, TRUE);
	selection = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_set_mode (selection,
				     GTK_SELECTION_SINGLE);
	
	renderer = gtk_cell_renderer_text_new ();

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_META_NAME,
						     _("Name"),
						     renderer,
						     "text", COL_META_NAME,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_META_NAME);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_META_NAME);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_META_CONTENT, 
						     _("Content"),
						     renderer,
						     "text", COL_META_CONTENT,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_META_CONTENT);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_META_CONTENT);

	return treeview;
}
*/

/*
static void
setup_page_security (PageInfoDialog *dialog, EmbedPageProperties *props)
{
	GtkWidget *widget;
	gchar *location;
	gchar *text, *title, *msg1, *host = NULL;
	const gchar *const_title, *msg2;
	GaleonTab *tab = dialog->priv->tab;
	GaleonEmbed *embed = galeon_tab_get_embed (tab);

	galeon_embed_get_location (embed, TRUE, TRUE, &location);
	if (location)
	{
		GnomeVFSURI* uri = gnome_vfs_uri_new (location);
		const char *hostname;
		if (uri && (hostname = gnome_vfs_uri_get_host_name (uri)) != NULL)
		{
			host = g_strdup_printf("\"<tt>%s</tt>\"",
					       gnome_vfs_uri_get_host_name (uri));
		}

		if (uri) gnome_vfs_uri_unref (uri);
		g_free (location);
	}

*/
	/* Web Site Identity Verification */
/*
	widget = galeon_dialog_get_control (GALEON_DIALOG (dialog), 
					    PROP_SECURITY_VIEW_CERT_VBOX);
	if (!props->secret_key_length)
	{
		const_title = _("Web Site Identity Not Verified");
		msg1 = NULL;
		gtk_widget_hide (widget);
	}
	else
	{
		gchar *issuer = g_strdup_printf( "\"<tt>%s</tt>\"", props->cert_issuer_name);

		const_title = _("Web Site Identity Verified");
		msg1 = g_strdup_printf (_("The web site %s supports authentication "
					  "for the page you are viewing. The "
					  "identity of this web site has been "
					  "verified by %s, a certificate "
					  "authority you trust for this purpose."),
					host, issuer);
		gtk_widget_show (widget);
		g_free (issuer);
	}

	text = g_strdup_printf ("<b>%s</b>", const_title);
	page_info_set_text (dialog, PROP_SECURITY_CERT_TITLE, text);
	page_info_set_text (dialog, PROP_SECURITY_CERT_INFO, msg1);
	g_free (text);
	g_free (msg1);

*/
	/* Connection encryption */
/*

	if (props->key_length > 90)
	{
		title = g_strdup_printf(_("Connection Encrypted: High-grade "
					  "Encryption (%s %d bit)"),
					props->cipher_name, props->key_length);
		msg1 = g_strdup (_("The page you are viewing was encrypted before "
				   "being transmitted over the Internet."));
		msg2 = _("Encryption makes it very difficult for unauthorized "
			 "people to view information traveling between computers. "
			 "It is therefore very unlikely that anyone read this page "
			 "as it traveled across the network.");
	}
	else if (props->key_length > 0)
	{
		title = g_strdup_printf(_("Connection Encrypted: Low-grade "
					  "Encryption (%s %d bit)"),
					props->cipher_name, props->key_length);

		msg1 = g_strdup_printf (_("The web site %s is using low-grade "
					  "encryption for the page you are viewing."),
					host);
		msg2 = _("Low-grade encryption may allow some unauthorized "
			 "people to view this information.");
	}
	else
	{
		title = g_strdup (_("Connection Not Encrypted"));

		if (host)
		{
			msg1 = g_strdup_printf (_("The web site %s does not support "
						  "encryption for the page you are "
						  "viewing."), host);
		}
		else
		{
			msg1 = g_strdup (_("The page you are viewing is not encrypted."));
		}
				

		msg2 = _("Information sent over the Internet without encryption "
			 "can be seen by other people while it is in transit.");
	}


	text = g_strdup_printf ("<b>%s</b>", title);
	page_info_set_text (dialog, PROP_SECURITY_CIPHER_TITLE, text);
	g_free (text);

	text = g_strdup_printf ("%s\n\n%s", msg1, msg2);
	page_info_set_text (dialog, PROP_SECURITY_CIPHER_INFO, text);

	g_free (text);
	g_free (title);
	g_free (msg1);
	g_free (host);
}
*/

/*
static void
setup_page_general_add_meta_tag(GtkTreeView *treeView, EmbedPageMetaTag *tag)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model
			       (GTK_TREE_VIEW(treeView)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store,
			   &iter,
			   COL_META_NAME, tag->name,
			   COL_META_CONTENT, tag->content,
			    -1);
}
*/

/*
void
page_info_dialog_set_current_page (PageInfoDialog *dialog, PageInfoDialogPage page)
{
	GtkNotebook *notebook;
	guint        n_pages;
	
	notebook = GTK_NOTEBOOK(dialog->priv->notebook);
	n_pages  = gtk_notebook_get_n_pages(notebook);

	if (page < n_pages)
	{
		gtk_notebook_set_current_page(notebook, page);
	}
	else
	{
		g_warning("%s: invalid page `%d'", G_STRLOC, page);
	}
}
*/

void
page_info_dialog_close_button_clicked_cb (GtkWidget *button,
					  PageInfoDialog *dialog)
{
	g_object_unref (dialog);
}

void
page_info_dialog_view_cert_button_clicked_cb (GtkWidget *button,
					      PageInfoDialog *dialog)
{
	/*
	GaleonEmbed *embed = galeon_tab_get_embed (dialog->priv->tab);
	galeon_embed_show_page_certificate (embed);
	*/
}

/* a generic treeview info page */

typedef struct _TreeviewInfoPage TreeviewInfoPage;
	
struct _TreeviewInfoPage
{
	InfoPage page;

	GtkListStore *store;
	GtkTreeView *treeview;
	GtkActionEntry *action_entries;
	guint n_action_entries;
	const char *popup_path;
};

static gboolean
treeview_info_page_show_popup (TreeviewInfoPage *page)
{
	InfoPage *ipage = (InfoPage *) page;
	PageInfoDialog *dialog = ipage->dialog;
	GtkWidget *widget;

	widget = gtk_ui_manager_get_widget (dialog->priv->manager,
					    page->popup_path);
	gtk_menu_popup (GTK_MENU (widget), NULL, NULL, NULL, NULL, 2,
			gtk_get_current_event_time ());

	return TRUE;
}

static gboolean
treeview_info_page_button_pressed_cb (GtkTreeView *treeview,
				      GdkEventButton *event,
				      TreeviewInfoPage *page)
{
	GtkTreeModel *model = GTK_TREE_MODEL (page->store);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreePath *path = NULL;

	/* right-click? */
	if (event->button != 3)
	{
	       return FALSE;
	}

	/* Get tree path for row that was clicked */
	if (!gtk_tree_view_get_path_at_pos (treeview,
					    event->x, event->y,
					    &path, NULL, NULL, NULL))
	{
	       return FALSE;
	}

	if (!gtk_tree_model_get_iter (model, &iter, path))
	{
	       gtk_tree_path_free(path);
	       return FALSE;
	}

	/* Select the row the user clicked on */
	selection = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_unselect_all (selection);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	return treeview_info_page_show_popup (page);
}

static void
treeview_info_page_construct (InfoPage *ipage)
{
	TreeviewInfoPage *page = (TreeviewInfoPage *) ipage;
	PageInfoDialog *dialog = ipage->dialog;

	gtk_action_group_add_actions (dialog->priv->action_group,
				      page->action_entries,
				      page->n_action_entries,
				      page);

	g_signal_connect (page->treeview, "button-press-event", 
			  G_CALLBACK (treeview_info_page_button_pressed_cb), 
			  page);
	g_signal_connect_swapped (page->treeview, "popup-menu", 
				  G_CALLBACK (treeview_info_page_show_popup),
				  page);
}

/* "General" page */

static void
page_info_set_text (PageInfoDialog *dialog,
		    const char *prop,
		    const char *text)
{
	GtkWidget *widget;
	/* FIXME: Switch to prop strings instead of enum */
	widget = ephy_dialog_get_control (EPHY_DIALOG (dialog), prop);

	g_return_if_fail (GTK_IS_ENTRY (widget) || GTK_IS_LABEL (widget));

	if (!text) text = "";

	if (GTK_IS_ENTRY(widget))
	{
		gtk_entry_set_text(GTK_ENTRY(widget), text);
	}
	else
	{
		gtk_label_set_markup(GTK_LABEL(widget), text);
	}
}

static void
general_info_page_fill (InfoPage *page)
{
	PageInfoDialog *dialog = page->dialog;
	EphyEmbed *embed = dialog->priv->embed;
	EmbedPageProperties *props;
	/* GtkTreeView *page_info_meta_list; */
	GList *i;
	const char *text;
	const char *date_hack = "%c"; /* quiet gcc */
	char date[64];
	char *val;
	struct tm tm;
	time_t t;

	LOG ("general_info_page_fill")

	props = mozilla_get_page_properties (embed);
	g_return_if_fail (props != NULL);

	val = ephy_embed_get_title (embed);
	page_info_set_text (dialog, properties[PROP_GENERAL_PAGE_TITLE].id, val);
	g_free (val);

	val = ephy_embed_get_location (embed, TRUE);
	page_info_set_text (dialog, properties[PROP_GENERAL_URL].id, val);
	g_free (val);

	page_info_set_text (dialog, properties[PROP_GENERAL_MIME_TYPE].id, props->content_type);

	text = gnome_vfs_mime_get_description (props->content_type);
	page_info_set_text (dialog, properties[PROP_GENERAL_TYPE].id,
			    text ? text : _("Unknown type"));

	switch (props->rendering_mode)
	{
		case EMBED_RENDER_FULL_STANDARDS:
			text = _("Full standards compliance");
			break;
		case EMBED_RENDER_ALMOST_STANDARDS:
			text = _("Almost standards compliance");
			break;
		case EMBED_RENDER_QUIRKS:
			text = _("Compatibility");
			break;
		default:
			text = _("Undetermined");
			break;
	}
	page_info_set_text (dialog, properties[PROP_GENERAL_RENDER_MODE].id, text);

	switch (props->page_source)
	{
		case EMBED_SOURCE_NOT_CACHED:
			text = _("Not cached");
			break;
		case EMBED_SOURCE_DISK_CACHE:
			text = _("Disk cache");
			break;
		case EMBED_SOURCE_MEMORY_CACHE:	
			text = _("Memory cache");
			break;
		case EMBED_SOURCE_UNKNOWN_CACHE:
			text = _("Unknown cache");
			break;
	}

	page_info_set_text (dialog, properties[PROP_GENERAL_SOURCE].id, text);

	page_info_set_text (dialog, properties[PROP_GENERAL_ENCODING].id, props->encoding);

	if (props->size != -1) 
	{
		val = gnome_vfs_format_file_size_for_display (props->size);
		page_info_set_text (dialog, properties[PROP_GENERAL_SIZE].id, val);
		g_free (val);
	}
	else
	{
		page_info_set_text (dialog, properties[PROP_GENERAL_SIZE].id, _("Unknown"));
	}

	page_info_set_text (dialog, properties[PROP_GENERAL_REFERRING_URL].id,
			    props->referring_url ? props->referring_url :
			    _("No referrer" ));
	
	if (props->modification_time)
	{
		t = props->modification_time;
		strftime (date, sizeof(date), date_hack, localtime_r (&t, &tm));
		val = g_locale_to_utf8 (date, -1, NULL, NULL, NULL);
		
		page_info_set_text (dialog, properties[PROP_GENERAL_MODIFIED].id, val);
		g_free (val);
	}
	else
	{
		page_info_set_text (dialog, properties[PROP_GENERAL_MODIFIED].id,
				    _("Not specified"));

	}

	if (props->expiration_time)
	{
		t = props->expiration_time;
		strftime (date, sizeof(date), date_hack, localtime_r (&t, &tm));
		val = g_locale_to_utf8 (date, -1, NULL, NULL, NULL);

		page_info_set_text (dialog, properties[PROP_GENERAL_EXPIRES].id, val);
		g_free (val);
	}
	else
	{
		page_info_set_text (dialog, properties[PROP_GENERAL_EXPIRES].id,
				    _("Not specified"));

	}

	/*
	page_info_meta_list = setup_meta_treeview(dialog);
	for (i=props->metatags ; i ; i = i->next)
	{
		setup_page_general_add_meta_tag(page_info_meta_list,
						(EmbedPageMetaTag*)i->data);
	}	
	*/

	mozilla_free_page_properties (props);
}

static InfoPage *
general_info_page_new (PageInfoDialog *dialog)
{
	InfoPage *ipage = g_new0 (InfoPage, 1);

	ipage->dialog = dialog;
	ipage->fill = general_info_page_fill;

	return ipage;
}

/* "Images" page */

typedef struct _ImagesInfoPage ImagesInfoPage;

struct _ImagesInfoPage
{
	TreeviewInfoPage tpage;

	GtkWidget *save_button;
	EphyEmbed *embed;
};

enum
{
	COL_IMAGE_URL,
	COL_IMAGE_ALT,
	COL_IMAGE_TITLE,
	COL_IMAGE_WIDTH,
	COL_IMAGE_HEIGHT
};

static void
images_save_image_cb (GtkAction *action,
		      ImagesInfoPage *page)
{
	g_print ("save image!\n");
}

static GtkActionEntry images_action_entries[] =
{
	{ "SaveAs", GTK_STOCK_SAVE_AS, N_("_Save Image As..."), NULL,
	  N_("Save image"),
	  G_CALLBACK (images_save_image_cb) },
};

void
page_info_image_box_realize_cb (GtkContainer *box,
				PageInfoDialog *dialog)
{
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) dialog->priv->pages[IMAGES_PAGE];
	ImagesInfoPage *page = (ImagesInfoPage *) tpage;
	EphyEmbed *embed;
	GtkWidget *treeview;

	page->embed = embed = EPHY_EMBED (ephy_embed_factory_new_object ("EphyEmbed"));

	/* When the image has loaded grab the focus for the treeview
	 * again. This means that you can navigate in the treeview
	 * using the arrow keys */
	g_signal_connect_swapped (embed, "net_stop",
				  G_CALLBACK (gtk_widget_grab_focus),
				  tpage->treeview);

	ephy_embed_load_url (embed, "about:blank");

	gtk_widget_show (GTK_WIDGET(embed));

	gtk_container_add (box, GTK_WIDGET (embed));
}

void
page_info_dialog_imagesave_button_clicked_cb (GtkWidget *button,
					      PageInfoDialog *dialog)
{
	ImagesInfoPage *page = (ImagesInfoPage *) dialog->priv->pages[IMAGES_PAGE];
	EphyEmbedPersist *persist;

	persist = EPHY_EMBED_PERSIST
		(ephy_embed_factory_new_object ("EphyEmbedPersist"));

	ephy_embed_persist_set_embed (persist, page->embed);
	ephy_embed_persist_set_flags (persist, EMBED_PERSIST_COPY_PAGE |
					       EMBED_PERSIST_ASK_DESTINATION);
	ephy_embed_persist_set_fc_title (persist, _("Save Image As..."));
	ephy_embed_persist_set_fc_parent (persist, GTK_WINDOW (dialog->priv->window));

	ephy_embed_persist_save (persist);

	g_object_unref (persist);
}

static void
images_treeview_selection_changed_cb (GtkTreeSelection *selection,
				      ImagesInfoPage *page)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *url;
	gboolean has_selected;

	has_selected = gtk_tree_selection_get_selected (selection, &model, &iter);

	if (has_selected)
	{
		gtk_tree_model_get(model, &iter,
				   COL_IMAGE_URL, &url,
				   -1);
		ephy_embed_load_url (page->embed, url);
		g_free (url);
	}
	else
	{
		ephy_embed_load_url (page->embed, "about:blank");
	}

	gtk_widget_set_sensitive (page->save_button, has_selected);
}

static void
images_info_page_construct (InfoPage *ipage)
{
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) ipage;
	ImagesInfoPage *page = (ImagesInfoPage *) ipage;
	PageInfoDialog *dialog = ipage->dialog;
	GtkTreeView *treeview;
	GtkListStore *liststore;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	GtkWidget *button, *vpaned;

	treeview = GTK_TREE_VIEW (ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_IMAGES_IMAGE_TREEVIEW].id));

	liststore = gtk_list_store_new (5,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_INT,
					G_TYPE_INT);
	gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (liststore));
	g_object_unref (liststore);

	selection = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	g_signal_connect (selection, "changed",
			  G_CALLBACK (images_treeview_selection_changed_cb),
			  page);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_IMAGE_URL,
						     _("URL"),
						     renderer,
						     "text", COL_IMAGE_URL,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_IMAGE_URL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_IMAGE_URL);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_IMAGE_ALT, 
						     _("Alt Text"),
						     renderer,
						     "text", COL_IMAGE_ALT,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_IMAGE_ALT);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_IMAGE_ALT);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_IMAGE_TITLE, 
						     _("Title"),
						     renderer,
						     "text", COL_IMAGE_TITLE,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_IMAGE_TITLE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_IMAGE_TITLE);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_IMAGE_WIDTH, 
						     _("Width"),
						     renderer,
						     "text", COL_IMAGE_WIDTH,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_IMAGE_WIDTH);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_IMAGE_WIDTH);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_IMAGE_HEIGHT, 
						     _("Height"),
						     renderer,
						     "text", COL_IMAGE_HEIGHT,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_IMAGE_HEIGHT);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_IMAGE_HEIGHT);

	button = ephy_dialog_get_control (EPHY_DIALOG (dialog),
					  properties[PROP_IMAGES_SAVE_BUTTON].id);
	vpaned = ephy_dialog_get_control (EPHY_DIALOG (dialog),
					  properties[PROP_IMAGES_IMAGE_VPANED].id);

	ephy_state_add_paned (vpaned, "PageInfoDialog::ImagesPage::VPaned", 250);

	tpage->store = liststore;
	tpage->treeview = treeview;
	page->save_button = button;

	treeview_info_page_construct (ipage);
}

static void
images_info_page_fill (InfoPage *ipage)
{
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) ipage;
	ImagesInfoPage *page = (ImagesInfoPage *) ipage;
	PageInfoDialog *dialog = ipage->dialog;
	GtkListStore *store = tpage->store;
	GtkTreeIter iter;
	GList *images, *l;

	images = mozilla_get_images (dialog->priv->embed);

	for (l = images; l != NULL; l = l->next)
	{
		EmbedPageImage *image = (EmbedPageImage *) l->data;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COL_IMAGE_URL, image->url,
				    COL_IMAGE_ALT, image->alt,
				    COL_IMAGE_TITLE, image->title,
				    COL_IMAGE_WIDTH, image->width,
				    COL_IMAGE_HEIGHT, image->height,
				    -1);
	}

	g_list_foreach (images, (GFunc) mozilla_free_embed_page_image, NULL);
	g_list_free (images);
}

static InfoPage *
images_info_page_new (PageInfoDialog *dialog)
{
	ImagesInfoPage *page = g_new0 (ImagesInfoPage, 1);
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) page;
	InfoPage *ipage = (InfoPage *) page;

	ipage->dialog = dialog;
	ipage->construct = images_info_page_construct;
	ipage->fill = images_info_page_fill;

	tpage->popup_path = "/ImagesPopup";
	tpage->action_entries = images_action_entries;
	tpage->n_action_entries = G_N_ELEMENTS (images_action_entries);

	return ipage;
}

/* "Links" page */

typedef struct _LinksInfoPage LinksInfoPage;

struct _LinksInfoPage
{
	TreeviewInfoPage tpage;
};

enum
{
	COL_LINK_URL,
	COL_LINK_TITLE,
	COL_LINK_REL
};

static GtkActionEntry links_action_entries[] =
{
};

static void
links_info_page_construct (InfoPage *ipage)
{
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) ipage;
	LinksInfoPage *page = (LinksInfoPage *) ipage;
	PageInfoDialog *dialog = ipage->dialog;
	GtkTreeView *treeview;
	GtkListStore *liststore;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	treeview = GTK_TREE_VIEW (ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_LINKS_LINK_TREEVIEW].id));

	liststore = gtk_list_store_new (3,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);
	gtk_tree_view_set_model (treeview, GTK_TREE_MODEL(liststore));
	g_object_unref (liststore);

	gtk_tree_view_set_headers_visible (treeview, TRUE);
	selection = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_set_mode (selection,
				     GTK_SELECTION_SINGLE);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_LINK_URL,
						     _("URL"),
						     renderer,
						     "text", COL_LINK_URL,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_LINK_URL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_LINK_URL);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_LINK_TITLE, 
						     _("Title"),
						     renderer,
						     "text", COL_LINK_TITLE,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_LINK_TITLE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_LINK_TITLE);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_LINK_REL, 
						     _("Relation"),
						     renderer,
						     "text", COL_LINK_REL,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_LINK_REL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_LINK_REL);

	tpage->store = liststore;
	tpage->treeview = treeview;

//	treeview_info_page_construct (ipage);
}

static void
links_info_page_fill (InfoPage *ipage)
{
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) ipage;
	LinksInfoPage *page = (LinksInfoPage *) ipage;
	PageInfoDialog *dialog = ipage->dialog;
	GtkListStore *store = tpage->store;
	GtkTreeIter iter;
	GList *links, *l;

	links = mozilla_get_links (dialog->priv->embed);

	for (l = links; l != NULL; l = l->next)
	{
		EmbedPageLink *link = (EmbedPageLink *) l->data;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COL_LINK_URL, link->url,
				    COL_LINK_TITLE, link->title,
				    COL_LINK_REL, link->rel,
				    -1);
	}

	g_list_foreach (links, (GFunc) mozilla_free_embed_page_link, NULL);
	g_list_free (links);
}

static InfoPage *
links_info_page_new (PageInfoDialog *dialog)
{
	LinksInfoPage *page = g_new0 (LinksInfoPage, 1);
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) page;
	InfoPage *ipage = (InfoPage *) page;

	ipage->dialog = dialog;
	ipage->construct = links_info_page_construct;
	ipage->fill = links_info_page_fill;

	tpage->popup_path = "/LinksPopup";
	tpage->action_entries = links_action_entries;
	tpage->n_action_entries = G_N_ELEMENTS (links_action_entries);

	return ipage;
}

/* "Forms" page */

typedef struct _FormsInfoPage FormsInfoPage;

struct _FormsInfoPage
{
	TreeviewInfoPage tpage;
};

enum
{
	COL_FORM_NAME,
	COL_FORM_METHOD,
	COL_FORM_ACTION
};

static GtkActionEntry forms_action_entries[] =
{
};

static void
forms_info_page_construct (InfoPage *ipage)
{
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) ipage;
	FormsInfoPage *page = (FormsInfoPage *) ipage;
	PageInfoDialog *dialog = ipage->dialog;

	GtkTreeView *treeview;
	GtkListStore *liststore;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	treeview = GTK_TREE_VIEW (ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_FORMS_FORM_TREEVIEW].id));

	liststore = gtk_list_store_new (3,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);
	gtk_tree_view_set_model (treeview, GTK_TREE_MODEL(liststore));
	g_object_unref (liststore);

	gtk_tree_view_set_headers_visible (treeview, TRUE);
	selection = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_FORM_NAME,
						     _("Name"),
						     renderer,
						     "text", COL_FORM_NAME,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_FORM_NAME);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_FORM_NAME);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_FORM_METHOD, 
						     _("Method"),
						     renderer,
						     "text", COL_FORM_METHOD,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_FORM_METHOD);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_FORM_METHOD);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_FORM_ACTION, 
						     _("Action"),
						     renderer,
						     "text", COL_FORM_ACTION,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_FORM_ACTION);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_FORM_ACTION);

	tpage->store = liststore;
	tpage->treeview = treeview;

//	treeview_info_page_construct (ipage);
}

static void
forms_info_page_fill (InfoPage *ipage)
{
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) ipage;
	ImagesInfoPage *page = (ImagesInfoPage *) ipage;
	PageInfoDialog *dialog = ipage->dialog;
	GtkListStore *store = tpage->store;
	GtkTreeIter iter;
	GList *forms, *l;

	forms = mozilla_get_forms (dialog->priv->embed);

	for (l = forms; l != NULL; l = l->next)
	{
		EmbedPageForm *form = (EmbedPageForm *) l->data;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COL_FORM_NAME, form->name,
				    COL_FORM_METHOD, form->method,
				    COL_FORM_ACTION, form->action,
				    -1);
	}

	g_list_foreach (forms, (GFunc) mozilla_free_embed_page_form, NULL);
	g_list_free (forms);
}

static InfoPage *
forms_info_page_new (PageInfoDialog *dialog)
{
	FormsInfoPage *page = g_new0 (FormsInfoPage, 1);
	TreeviewInfoPage *tpage = (TreeviewInfoPage *) page;
	InfoPage *ipage = (InfoPage *) page;

	ipage->dialog = dialog;
	ipage->construct = forms_info_page_construct;
	ipage->fill = forms_info_page_fill;

	tpage->popup_path = "/FormsPopup";
	tpage->action_entries = forms_action_entries;
	tpage->n_action_entries = G_N_ELEMENTS (forms_action_entries);

	return ipage;
}

/* object stuff */

static void
page_info_dialog_init (PageInfoDialog *dialog)
{
	InfoPage *page;

	dialog->priv = PAGE_INFO_DIALOG_GET_PRIVATE (dialog);

	dialog->priv->pages[GENERAL_PAGE] = general_info_page_new (dialog);
	dialog->priv->pages[IMAGES_PAGE] = images_info_page_new (dialog);
	dialog->priv->pages[LINKS_PAGE] = links_info_page_new (dialog);
	dialog->priv->pages[FORMS_PAGE] = forms_info_page_new (dialog);

	/*
	dialog->priv->pages[SECURITY_PAGE] = security_info_page_new (dialog);
	*/
}

static GObject *
page_info_dialog_constructor (GType type,
			      guint n_construct_properties,
			      GObjectConstructParam *construct_params)
{
	GObject *object;
	PageInfoDialog *dialog;
	EphyDialog *edialog;
	GtkWidget *notebook;
	GtkAction *action;
	InfoPage *page;
	GError *error = NULL;
	int i;

	object = parent_class->constructor (type, n_construct_properties,
					    construct_params);

	dialog = PAGE_INFO_DIALOG (object);
	edialog = EPHY_DIALOG (object);

	ephy_dialog_construct (edialog,
			       properties,
			       SHARE_DIR "/glade/page-info.glade",
			       "page_info_dialog",
			       GETTEXT_PACKAGE);

	/*
	notebook = ephy_dialog_get_control (edialog, properties[PROP_NOTEBOOK].id);
	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (sync_notebook_page), dialog);
	*/

	dialog->priv->dialog = ephy_dialog_get_control (edialog, properties[PROP_DIALOG].id);

	dialog->priv->manager = gtk_ui_manager_new ();
	dialog->priv->action_group = gtk_action_group_new ("PageInfoContextActions");
	gtk_action_group_set_translation_domain (dialog->priv->action_group,
						 GETTEXT_PACKAGE);
	action = g_object_new (GTK_TYPE_ACTION,
			       "name", "PopupAction",
			       NULL);
	gtk_action_group_add_action (dialog->priv->action_group, action);
	g_object_unref (action);

	gtk_ui_manager_insert_action_group (dialog->priv->manager,
					    dialog->priv->action_group, -1);

	gtk_ui_manager_add_ui_from_file (dialog->priv->manager,
					 SHARE_DIR "/xml/page-info-context-ui.xml",
					 &error);
	if (error != NULL)
	{
		g_warning ("Context Menu UI not loaded!\n");
		g_error_free (error);
	}

	for (i = GENERAL_PAGE; i < LAST_PAGE; i++)
	{
		page = dialog->priv->pages[i];

		if (page->construct != NULL)
		{
			page->construct (page);
		}

		/* FIXME: lazily initialise the pages as needed, instead */
		/* or maybe not :) */
		page->fill (page);
	}

	return object;
}

static void
page_info_dialog_finalize (GObject *object)
{
	PageInfoDialog *dialog = PAGE_INFO_DIALOG (object);

	LOG ("PageInfoDialog finalizing")

	G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
page_info_dialog_get_property (GObject *object,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *pspec)
{
	/* no readable properties */
	g_return_if_reached ();
}

static void
page_info_dialog_set_property (GObject *object,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *pspec)
{
	PageInfoDialog *dialog = PAGE_INFO_DIALOG (object);

	switch (prop_id)
	{
		case PROP_EMBED:
			dialog->priv->embed = g_value_get_object (value);
			break;
		case PROP_WINDOW:
			dialog->priv->window = g_value_get_object (value);
			break;
	}
}

static void
page_info_dialog_class_init (PageInfoDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = page_info_dialog_finalize;
	object_class->constructor = page_info_dialog_constructor;
	object_class->get_property = page_info_dialog_get_property;
	object_class->set_property = page_info_dialog_set_property;

	g_object_class_install_property
		(object_class,
		 PROP_EMBED,
		 g_param_spec_object ("embed",
				      "Embed",
				      "Embed",
				      G_TYPE_OBJECT,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property
		(object_class,
		 PROP_WINDOW,
		 g_param_spec_object ("window",
				      "Window",
				      "Window",
				      EPHY_TYPE_WINDOW,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof (PageInfoDialogPrivate));
}

PageInfoDialog *
page_info_dialog_new (EphyWindow *window,
		      EphyEmbed *embed)
{
	return g_object_new (TYPE_PAGE_INFO_DIALOG,
			     "parent-window", window,
			     "window", window,
			     "embed", embed,
			     NULL);
}