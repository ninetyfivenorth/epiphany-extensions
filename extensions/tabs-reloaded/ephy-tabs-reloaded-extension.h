/*
 *  Copyright © 2009 Benjamin Otte
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

#ifndef EPHY_TABS_RELOADED_EXTENSION_H
#define EPHY_TABS_RELOADED_EXTENSION_H

#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define EPHY_TYPE_TABS_RELOADED_EXTENSION		(ephy_tabs_reloaded_extension_get_type())
#define EPHY_TABS_RELOADED_EXTENSION(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_TABS_RELOADED_EXTENSION, EphyTabsReloadedExtension))
#define EPHY_TABS_RELOADED_EXTENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_TABS_RELOADED_EXTENSION, EphyTabsReloadedExtensionClass))
#define EPHY_IS_TABS_RELOADED_EXTENSION(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_TABS_RELOADED_EXTENSION))
#define EPHY_IS_TABS_RELOADED_EXTENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_TABS_RELOADED_EXTENSION))
#define EPHY_TABS_RELOADED_EXTENSION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_TABS_RELOADED_EXTENSION, EphyTabsReloadedExtensionClass))

typedef struct _EphyTabsReloadedExtension		EphyTabsReloadedExtension;
typedef struct _EphyTabsReloadedExtensionClass	        EphyTabsReloadedExtensionClass;
typedef struct _EphyTabsReloadedExtensionPrivate	EphyTabsReloadedExtensionPrivate;

struct _EphyTabsReloadedExtensionClass
{
	GObjectClass	parent_class;
};

struct _EphyTabsReloadedExtension
{
	GObject	parent_object;
};

GType	ephy_tabs_reloaded_extension_get_type	(void);

GType	ephy_tabs_reloaded_extension_register_type	(GTypeModule *module);

G_END_DECLS

#endif
