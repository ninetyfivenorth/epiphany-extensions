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
 */

#ifndef ERROR_VIEWER_CONSOLE_LISTENER_H
#define ERROR_VIEWER_CONSOLE_LISTENER_H

#include <nsIConsoleListener.h>

#include <glib-object.h>

class nsIScriptError;

class ErrorViewerConsoleListener : public nsIConsoleListener
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_NSICONSOLELISTENER

	ErrorViewerConsoleListener();
	virtual ~ErrorViewerConsoleListener();

	GObject *mDialog; /* FIXME */
private:
	nsresult GetMessageFromError (nsIScriptError *aError, char **aMessage);
};

#endif /* ERROR_VIEWER_CONSOLE_LISTENER_H */
