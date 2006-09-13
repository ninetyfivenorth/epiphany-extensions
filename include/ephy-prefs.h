/*
 *  Copyright © 2000-2003 Marco Pesenti Gritti
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

#ifndef EPHY_PREFS_H
#define EPHY_PREFS_H

G_BEGIN_DECLS

#define EPIPHANY_SCHEMA_VERSION 1
#define CONF_SCHEMA_VERSION "/apps/epiphany/schema_version"

/* General */
#define CONF_GENERAL_HOMEPAGE			"/apps/epiphany/general/homepage"
#define CONF_ALWAYS_SHOW_TABS_BAR		"/apps/epiphany/general/always_show_tabs_bar"
#define CONF_WINDOWS_SHOW_TOOLBARS		"/apps/epiphany/general/show_toolbars"
#define CONF_WINDOWS_SHOW_BOOKMARKS_BAR		"/apps/epiphany/general/show_bookmarks_bar"
#define CONF_WINDOWS_SHOW_STATUSBAR		"/apps/epiphany/general/show_statusbar"
#define CONF_INTERFACE_MIDDLE_CLICK_OPEN_URL	"/apps/epiphany/general/middle_click_open_url"
#define CONF_INTERFACE_TOOLBAR_STYLE		"/apps/epiphany/general/toolbar_style"
#define CONF_AUTO_DOWNLOADS			"/apps/epiphany/general/automatic_downloads"
#define CONF_DESKTOP_IS_HOME_DIR		"/apps/nautilus/preferences/desktop_is_home_dir"
#define CONF_NETWORK_MANAGED			"/apps/epiphany/general/managed_network"

/* Directories */
#define CONF_STATE_SAVE_DIR		"/apps/epiphany/directories/save"
#define CONF_STATE_SAVE_IMAGE_DIR	"/apps/epiphany/directories/saveimage"
#define CONF_STATE_OPEN_DIR		"/apps/epiphany/directories/open"
#define CONF_STATE_DOWNLOAD_DIR		"/apps/epiphany/directories/downloads_folder"
#define CONF_STATE_UPLOAD_DIR		"/apps/epiphany/directories/upload"

/* Lockdown */
#define CONF_LOCKDOWN_FULLSCREEN		"/apps/epiphany/lockdown/fullscreen"
#define CONF_LOCKDOWN_DISABLE_ARBITRARY_URL	"/apps/epiphany/lockdown/disable_arbitrary_url"
#define CONF_LOCKDOWN_DISABLE_BOOKMARK_EDITING  "/apps/epiphany/lockdown/disable_bookmark_editing"
#define CONF_LOCKDOWN_DISABLE_TOOLBAR_EDITING	"/apps/epiphany/lockdown/disable_toolbar_editing"
#define CONF_LOCKDOWN_DISABLE_HISTORY		"/apps/epiphany/lockdown/disable_history"
#define CONF_LOCKDOWN_DISABLE_SAVE_TO_DISK	"/desktop/gnome/lockdown/disable_save_to_disk"
#define CONF_LOCKDOWN_DISABLE_HISTORY		"/apps/epiphany/lockdown/disable_history"
#define CONF_LOCKDOWN_DISABLE_PRINTING		"/desktop/gnome/lockdown/disable_printing"
#define CONF_LOCKDOWN_DISABLE_PRINT_SETUP	"/desktop/gnome/lockdown/disable_print_setup"
#define CONF_LOCKDOWN_DISABLE_COMMAND_LINE	"/desktop/gnome/lockdown/disable_command_line"
#define CONF_LOCKDOWN_DISABLE_QUIT		"/apps/epiphany/lockdown/disable_quit"
#define CONF_LOCKDOWN_DISABLE_JAVASCRIPT_CHROME	"/apps/epiphany/lockdown/disable_javascript_chrome"

/* System prefs */
#define CONF_DESKTOP_FTP_HANDLER	"/desktop/gnome/url-handlers/ftp/command"
#define CONF_DESKTOP_TOOLBAR_STYLE	"/desktop/gnome/interface/toolbar_style"
#define CONF_DESKTOP_BG_PICTURE		"/desktop/gnome/background/picture_filename"
#define CONF_DESKTOP_BG_TYPE		"/desktop/gnome/background/picture_options"

G_END_DECLS

#endif
