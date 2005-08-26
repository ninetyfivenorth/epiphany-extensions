/*
 *  Copyright (C) 2002  Ricardo Fern�ndez Pascual
 *  Copyright (C) 2005  Crispin Flowerday
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
 */

#ifndef __ephy_auto_scroller_h
#define __ephy_auto_scroller_h

#include <gtk/gtkwidget.h>
#include <glib-object.h>
#include <epiphany/ephy-embed.h>


G_BEGIN_DECLS

#define EPHY_TYPE_AUTO_SCROLLER	        (ephy_auto_scroller_get_type())
#define EPHY_AUTO_SCROLLER(object)	(G_TYPE_CHECK_INSTANCE_CAST((object), \
					 EPHY_TYPE_AUTO_SCROLLER,\
					 EphyAutoScroller))
#define EPHY_AUTO_SCROLLER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), \
					 EPHY_TYPE_AUTO_SCROLLER,\
					 EphyAutoScrollerClass))
#define EPHY_IS_AUTO_SCROLLER(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), \
					 EPHY_TYPE_AUTO_SCROLLER))
#define EPHY_IS_AUTO_SCROLLER_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE((klass), \
					 EPHY_TYPE_AUTO_SCROLLER))
#define EPHY_AUTO_SCROLLER_GET_CLASS(obj)(G_TYPE_INSTANCE_GET_CLASS((obj), \
					 EPHY_TYPE_AUTO_SCROLLER,\
					 EphyAutoScrollerClass))

typedef struct _EphyAutoScroller EphyAutoScroller;
typedef struct _EphyAutoScrollerClass EphyAutoScrollerClass;
typedef struct _EphyAutoScrollerPrivate EphyAutoScrollerPrivate;

struct _EphyAutoScrollerClass 
{
	GObjectClass parent_class;
};

struct _EphyAutoScroller
{
	GObject parent_object;
	EphyAutoScrollerPrivate *priv;
};

GType			ephy_auto_scroller_get_type	(void);
GType                   ephy_auto_scroller_register_type (GTypeModule *module);
EphyAutoScroller *	ephy_auto_scroller_new		(void);
void			ephy_auto_scroller_set_embed	(EphyAutoScroller *as,
                                                         EphyEmbed *embed);
void			ephy_auto_scroller_start_scroll	(EphyAutoScroller *as,
							 GtkWidget *widget, 
							 gint x, gint y);

G_END_DECLS

#endif
