/*
 *  Copyright (C) 2000 Marco Pesenti Gritti
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

#ifndef SMART_BOOKMARKS_MOZILLA_SELECTION_H
#define SMART_BOOKMARKS_MOZILLA_SELECTION_H

#include "config.h"

#include <glib.h>

#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

char *mozilla_get_selected_text (EphyEmbed *embed);

G_END_DECLS

#endif
