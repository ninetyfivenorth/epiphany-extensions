/* 
 * Copyright (C) 2005 Jean-Yves Lefort <jylefort@brutele.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG_H
#define _EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG_H

#include <epiphany/ephy-dialog.h>
#include <epiphany/ephy-extension.h>

G_BEGIN_DECLS

#define EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG		(ephy_actions_extension_editor_dialog_type)
#define EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG, EphyActionsExtensionEditorDialog))
#define EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG, EphyActionsExtensionEditorDialogClass))
#define EPHY_IS_ACTIONS_EXTENSION_EDITOR_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG))
#define EPHY_IS_ACTIONS_EXTENSION_EDITOR_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG))
#define EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG, EphyActionsExtensionEditorDialogClass))

typedef struct _EphyActionsExtensionEditorDialog		EphyActionsExtensionEditorDialog;
typedef struct _EphyActionsExtensionEditorDialogClass		EphyActionsExtensionEditorDialogClass;
typedef struct _EphyActionsExtensionEditorDialogPrivate		EphyActionsExtensionEditorDialogPrivate;

struct _EphyActionsExtensionEditorDialog
{
	EphyDialog parent;

	/*< private >*/
	EphyActionsExtensionEditorDialogPrivate *priv;
};

struct _EphyActionsExtensionEditorDialogClass
{
	EphyDialogClass parent_class;
};

extern GType ephy_actions_extension_editor_dialog_type;

GType	ephy_actions_extension_editor_dialog_register_type
						(GTypeModule	*module);

EphyActionsExtensionEditorDialog *
	ephy_actions_extension_editor_dialog_new
						(EphyExtension	*extension);

G_END_DECLS

#endif /* _EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG_H */