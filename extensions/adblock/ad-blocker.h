/*
 *  Copyright (C) 2004 Adam Hooper
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

#ifndef AD_BLOCKER_H
#define AD_BLOCKER_H

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define TYPE_AD_BLOCKER		(ad_blocker_get_type ())
#define AD_BLOCKER(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AD_BLOCKER, AdBlocker))
#define AD_BLOCKER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), TYPE_AD_BLOCKER, AdBlockerClass))
#define IS_AD_BLOCKER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AD_BLOCKER))
#define IS_AD_BLOCKER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_AD_BLOCKER))
#define AD_BLOCKER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_AD_BLOCKER, AdBlockerClass))

typedef struct AdBlocker	AdBlocker;
typedef struct AdBlockerClass	AdBlockerClass;
typedef struct AdBlockerPrivate	AdBlockerPrivate;

struct AdBlocker
{
	GObject parent_instance;

	/*< private >*/
	AdBlockerPrivate *priv;
};

struct AdBlockerClass
{
	GObjectClass parent_class;
};

GType		 ad_blocker_get_type		(void);

GType		 ad_blocker_register_type	(GTypeModule *module);

AdBlocker	*ad_blocker_new			(void);

gboolean	 ad_blocker_test_uri		(AdBlocker *blocker,
						 const char *url);

G_END_DECLS

#endif /* AD_BLOCKER_H */