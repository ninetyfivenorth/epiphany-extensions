/*
 *  Copyright (C) 2002 Marco Pesenti Gritti
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

#ifndef EPHY_STRING_H
#define EPHY_STRING_H

#include <glib.h>

G_BEGIN_DECLS

gboolean	 ephy_string_to_int			(const char *string,
							 gulong *integer);

char		*ephy_string_blank_chr			(char *source);

char		*ephy_string_shorten			(const char *str,
							 int target_length);

char		*ephy_string_elide_underscores		(const char *original);

char            *ephy_string_double_underscores         (const char *string);

GList		*ephy_string_parse_uri_list		(const char *uri_list);

G_END_DECLS

#endif
