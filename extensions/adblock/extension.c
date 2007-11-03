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

#include "ephy-adblock-extension.h"
#include "ad-blocker.h"
#include "adblock-ui.h"
#include "ephy-debug.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

G_MODULE_EXPORT GType register_module (GTypeModule *module);

G_MODULE_EXPORT GType
register_module (GTypeModule *module)
{
	LOG ("Registering EphyAdblockExtension");

#ifdef ENABLE_NLS
	/* Initialise the i18n stuff */
	bindtextdomain (GETTEXT_PACKAGE, EPHY_EXTENSIONS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */

	ad_blocker_register_type (module);
	ad_uri_tester_register_type (module);
	adblock_ui_register_type (module);
	return ephy_adblock_extension_register_type (module);
}
