/*
 *  Copyright (C) 2000, 2001, 2002 Marco Pesenti Gritti
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

#ifndef EPHY_EMBED_DIALOG_H
#define EPHY_EMBED_DIALOG_H

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-dialog.h>

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

#define EPHY_TYPE_EMBED_DIALOG		(ephy_embed_dialog_get_type ())
#define EPHY_EMBED_DIALOG(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_EMBED_DIALOG, EphyEmbedDialog))
#define EPHY_EMBED_DIALOG_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_EMBED_DIALOG, EphyEmbedDialogClass))
#define EPHY_IS_EMBED_DIALOG(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_EMBED_DIALOG))
#define EPHY_IS_EMBED_DIALOG_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_EMBED_DIALOG))
#define EPHY_EMBED_DIALOG_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_EMBED_DIALOG, EphyEmbedDialogClass))

typedef struct EphyEmbedDialogClass	EphyEmbedDialogClass;
typedef struct EphyEmbedDialog		EphyEmbedDialog;
typedef struct EphyEmbedDialogPrivate	EphyEmbedDialogPrivate;

struct EphyEmbedDialog
{
        EphyDialog parent;

	/*< private >*/
        EphyEmbedDialogPrivate *priv;
};

struct EphyEmbedDialogClass
{
        EphyDialogClass parent_class;
};

GType			ephy_embed_dialog_get_type		(void);

EphyEmbedDialog	       *ephy_embed_dialog_new			(EphyEmbed *embed);

EphyEmbedDialog	       *ephy_embed_dialog_new_with_parent	(GtkWidget *parent_window,
								EphyEmbed *embed);

void			ephy_embed_dialog_set_embed		(EphyEmbedDialog *dialog,
								 EphyEmbed *embed);

EphyEmbed *		ephy_embed_dialog_get_embed		(EphyEmbedDialog *dialog);

G_END_DECLS

#endif
