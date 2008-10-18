#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  Epiphany extension: Creative Commons license viewer
#  Version 0.2 (27/07/2006)
#  Copyright © 2006  Jaime Frutos Morales  <acidborg@gmail.com>
#  Copyright © 2008  Diego Escalante Urrelo  <diegoe@gnome.org>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

from gettext import gettext as _
import os.path
import time

import gtk
import epiphany

# Create CC icon from code generated using "gdk-pixbuf-csource --raw cc.png"
_icon = (""
    # Pixbuf magic (0x47646b50)
    "GdkP"
    # length: header (24) + pixel_data (6400)
    "\0\0\31\30"
    # pixdata_type (0x1010002)
    "\1\1\0\2"
    # rowstride (160)
    "\0\0\0\240"
    # width (40)
    "\0\0\0("
    # height (40)
    "\0\0\0("
    # pixel_data:
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0"
    "\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\243\243\243\377___"
    "\377666\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377666\377mmm\377\243\243\243\377\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0"
    "\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\276\276\276\377m"
    "mm\377\33\33\33\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\33\33\33\377zzz\377\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0"
    "\0\0\21\0\0\0\210\210\210\377\33\33\33\377\0\0\0\377\0\0\0\377\0\0\0"
    "\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0"
    "\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0"
    "\0\0\377\0\0\0\377\33\33\33\377\210\210\210\377\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\276\276\276\377"
    "DDD\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\21"
    "\21\21\377UUU\377\210\210\210\377\210\210\210\377\273\273\273\377\273"
    "\273\273\377\210\210\210\377\210\210\210\377DDD\377\21\21\21\377\0\0"
    "\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377RRR\377\276\276"
    "\276\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0"
    "\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\276\276\276"
    "\377)))\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377fff\377"
    "\314\314\314\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\252\252\252\377fff\377\0\0\0"
    "\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377)))\377\261\261\261\377\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\276\276\276\377\33\33\33\377\0\0\0\377\0"
    "\0\0\377\0\0\0\377\0\0\0\377UUU\377\335\335\335\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\335\335\335\377DDD\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\16\16"
    "\16\377\243\243\243\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\276\276\276\377)))\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\21\21\21\377\231\231\231\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\210"
    "\210\210\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\16\16\16\377\276"
    "\276\276\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0)))\377\0\0\0\377\0\0\0\377\0\0\0\377\21\21\21\377\314"
    "\314\314\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\314\314"
    "\314\377\20\20\20\377\0\0\0\377\0\0\0\377\0\0\0\377)))\377\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0mmm\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\21\21\21\377\314\314\314\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\314\314"
    "\314\377\20\20\20\377\0\0\0\377\0\0\0\377\0\0\0\377___\377\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\243\243\243\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\273\273\273\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\273\273\273\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\226\226\226\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0""666\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\210\210\210\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\210\210\210\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377666\377\21\0\0\0\21\0\0\0\21\0\0\0\226\226\226\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\"\"\"\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377!!!\377\0\0\0\377\0\0\0\377\0\0\0\377\226\226\226\377\21"
    "\0\0\0\21\0\0\0DDD\377\0\0\0\377\0\0\0\377\0\0\0\377\273\273\273\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\231\231\231\377\0\0\0\377\0"
    "\0\0\377\0\0\0\377RRR\377\21\0\0\0\276\276\276\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\"\"\"\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\314"
    "\314\314\377\210\210\210\377\210\210\210\377\231\231\231\377\314\314"
    "\314\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\356\356\356\377\273\273\273\377"
    "\210\210\210\377\210\210\210\377\273\273\273\377\356\356\356\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377!!!\377\0\0\0\377\0\0\0\377\16\16\16\377\21\0"
    "\0\0\210\210\210\377\0\0\0\377\0\0\0\377\0\0\0\377\210\210\210\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\335\335\335\377DDD\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377DDD\377\314\314\314\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\231\231\231\377\21\21\21\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\21\21\21\377\210\210\210\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377www\377\0\0\0\377\0\0"
    "\0\377\0\0\0\377\226\226\226\377___\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\335\335\335\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\356\356\356\377\21\21\21\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\21\21\21\377\314\314\314\377"
    "\377\377\377\377\210\210\210\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0"
    "\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377fff\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\273\273\273\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377mmm\377666\377\0\0\0\377\0\0\0\377\21\21\21\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377fff\377\0\0\0\377\0\0\0\377\0\0\0\377\210\210\210\377\273\273"
    "\273\377\252\252\252\377333\377\0\0\0\377\21\21\21\377\231\231\231\377"
    "\335\335\335\377\0\0\0\377\0\0\0\377\0\0\0\377333\377\252\252\252\377"
    "\273\273\273\377\210\210\210\377\0\0\0\377\0\0\0\377DDD\377\356\356\356"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377DDD\377\33\33\33\377\0\0\0\377\0\0\0\377"
    "DDD\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\335\335\335\377\0\0\0\377\0\0\0\377\0\0\0\377www\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\335\335\335\377\210\210\210\377\356"
    "\356\356\377\377\377\377\377www\377\0\0\0\377\0\0\0\377\0\0\0\377\335"
    "\335\335\377\377\377\377\377\377\377\377\377\377\377\377\377\210\210"
    "\210\377\273\273\273\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377!!!\377\0\0\0\377"
    "\0\0\0\377666\377\0\0\0\377\0\0\0\377\0\0\0\377UUU\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\273\273\273\377\0\0"
    "\0\377\0\0\0\377\0\0\0\377\273\273\273\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377DDD\377\0\0\0\377\0\0\0\377333\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377CCC\377\0\0\0\377\0\0\0\377)))\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\210\210\210\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\210\210\210\377\0\0\0\377\0"
    "\0\0\377\0\0\0\377\273\273\273\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\21\21\21\377\0\0\0\377\0\0\0\377DDD\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377DDD\377\0\0\0\377\0\0\0\377\0\0\0"
    "\377\0\0\0\377\0\0\0\377\0\0\0\377www\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\231\231\231\377\0\0\0\377\0\0\0"
    "\377\0\0\0\377\273\273\273\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377DDD\377\0\0\0\377\0\0\0\377DDD\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377CCC\377\0\0\0\377\0\0\0\377\16\16\16\377\0\0"
    "\0\377\0\0\0\377\0\0\0\377DDD\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\273\273\273\377\0\0\0\377\0\0\0\377\0\0"
    "\0\377\231\231\231\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\252\252\252\377\377\377\377\377\377\377\377\377UUU\377"
    "\0\0\0\377\0\0\0\377\21\21\21\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\335\335\335\377\335\335\335\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377BBB\377\0\0\0\377\0\0\0\377666\377666\377\0\0\0\377"
    "\0\0\0\377333\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\"\"\"\377\0\0\0\377\0\0\0\377\"\"\"\377"
    "\356\356\356\377\377\377\377\377\377\377\377\377\231\231\231\377\0\0"
    "\0\377\"\"\"\377\252\252\252\377\231\231\231\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377www\377\377\377\377\377\377\377\377\377\356\356\356\377333"
    "\377\0\0\0\377fff\377\356\356\356\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\0\0\0\377\0\0\0\377\0\0\0\377666\377"
    "DDD\377\0\0\0\377\0\0\0\377\0\0\0\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\252\252\252\377\0\0"
    "\0\377\0\0\0\377\0\0\0\377\0\0\0\377DDD\377333\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\252\252\252\377\377\377\377\377333\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\"\"\"\377DDD\377\21\21\21\377\0\0\0\377\0\0\0\377333\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\335\335"
    "\335\377\0\0\0\377\0\0\0\377\0\0\0\377___\377mmm\377\0\0\0\377\0\0\0"
    "\377\0\0\0\377\252\252\252\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377www\377\0\0\0\377\0\0\0\377\0"
    "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\231\231\231\377\377"
    "\377\377\377\377\377\377\377\314\314\314\377333\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377333\377\356\356\356\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\231\231\231"
    "\377\0\0\0\377\0\0\0\377\0\0\0\377zzz\377\243\243\243\377\0\0\0\377\0"
    "\0\0\377\0\0\0\377fff\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\273\273\273\377"
    "fff\377333\377\0\0\0\377333\377fff\377\314\314\314\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\356\356\356\377\210"
    "\210\210\377DDD\377\21\21\21\377\21\21\21\377DDD\377\210\210\210\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377RRR\377\0\0\0\377\0\0\0\377\0\0\0\377\261"
    "\261\261\377\21\0\0\0\33\33\33\377\0\0\0\377\0\0\0\377\0\0\0\377\335"
    "\335\335\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\335\335\335\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377)))\377\21\0\0\0\21\0\0\0mmm\377\0\0\0"
    "\377\0\0\0\377\0\0\0\377fff\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "ccc\377\0\0\0\377\0\0\0\377\0\0\0\377mmm\377\21\0\0\0\21\0\0\0\276\276"
    "\276\377\16\16\16\377\0\0\0\377\0\0\0\377\0\0\0\377\273\273\273\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\314\314\314\377\0\0\0\377\0\0\0\377\0\0\0\377\16\16\16\377\276\276"
    "\276\377\21\0\0\0\21\0\0\0\21\0\0\0mmm\377\0\0\0\377\0\0\0\377\0\0\0"
    "\377\"\"\"\377\356\356\356\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\356\356\356\377000\377\0\0\0\377\0\0\0\377\0\0\0\377mmm\377\21\0"
    "\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0)))\377\0\0\0\377\0\0\0\377\0"
    "\0\0\377333\377\356\356\356\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377BBB\377\0\0\0"
    "\377\0\0\0\377\0\0\0\377)))\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\243\243\243\377\16\16\16\377\0\0\0\377\0\0\0\377\0\0"
    "\0\377333\377\356\356\356\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\356\356\356\377ddd\377\0\0\0\377\0\0\0\377\0\0\0\377\16\16\16"
    "\377\243\243\243\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0zzz\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377333\377"
    "\335\335\335\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\335\335\335\377222\377\0\0\0\377\0\0"
    "\0\377\0\0\0\377\0\0\0\377zzz\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0zzz\377\0\0\0\377\0\0\0"
    "\377\0\0\0\377\0\0\0\377\21\21\21\377\231\231\231\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\231\231\231\377\20\20\20\377\0\0"
    "\0\377\0\0\0\377\0\0\0\377\0\0\0\377zzz\377\21\0\0\0\21\0\0\0\21\0\0"
    "\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0zzz\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377DDD\377"
    "\252\252\252\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
    "\377\231\231\231\377BBB\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0"
    "\0\0\377zzz\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\226\226"
    "\226\377\16\16\16\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\"\"\"\377www\377\252\252\252\377\314\314\314\377\377\377\377\377\377"
    "\377\377\377\377\377\377\377\377\377\377\377\273\273\273\377\231\231"
    "\231\377fff\377\20\20\20\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\16\16\16\377\226\226\226\377\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\261\261\261\377666\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377DDD\377\276\276\276"
    "\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\210\210\210\377\33\33\33\377\0\0\0\377\0"
    "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377666\377\226\226\226\377\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0"
    "\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\226\226\226\377RRR\377\16\16\16\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
    "\0\0\0\377\0\0\0\377\33\33\33\377RRR\377\243\243\243\377\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0"
    "\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0"
    "\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\243\243\243\377zzz\377mmm\377___\377666\377"
    "666\377mmm\377mmm\377zzz\377\243\243\243\377\21\0\0\0\21\0\0\0\21\0\0"
    "\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21"
    "\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0\21\0\0\0")

icon_size = gtk.icon_size_lookup(gtk.ICON_SIZE_MENU)
pixbuf = gtk.gdk.pixbuf_new_from_inline(len(_icon), _icon, False)
pixbuf = pixbuf.scale_simple(icon_size[0],
    icon_size[1], gtk.gdk.INTERP_BILINEAR)

def _switch_page_cb (notebook, page, page_num, window):
    ui_show (window, window.get_active_child())

def _load_status_cb(embed, data, window):
    if not embed.get_property('load-status'):
        # page is loaded
        detect_license(window, embed)
    else:
        embed._has_cc = False
    ui_show (window, embed)

def _cc_button_pressed(button,event,window):
    show_license(window)

def ui_init (window):
    cc_image = gtk.Image()
    cc_image.set_from_pixbuf(pixbuf)
    cc_image.show()

    eventbox = gtk.EventBox()
    eventbox.set_visible_window(True)
    eventbox.connect ("button-press-event",_cc_button_pressed, window);
    # Pack the widgets
    eventbox.add(cc_image)

    statusbar = window.get_statusbar()
    statusbar.add_widget(eventbox)
    statusbar._cc_eventbox = eventbox
    eventbox.set_tooltip_text (_("View Creative Commons license"))

def ui_show(window, embed):
    if embed != window.get_active_child(): return

    statusbar = window.get_statusbar()
    eventbox = statusbar._cc_eventbox

    try:
        if embed._has_cc:
            eventbox.show()
        else:
            eventbox.hide()
    except:
        eventbox.hide()

def ui_destroy(window):
    statusbar = window.get_statusbar()
    eventbox = statusbar._cc_eventbox
    del statusbar._cc_eventbox
    statusbar.remove_widget(eventbox)

def show_license(window):
    statusbar = window.get_statusbar()
    notebook = window.get_notebook()

    license_url = statusbar._cc_url

    position = notebook.get_n_pages() - 1
    previous_tab = notebook.get_nth_page(position)

    shell = epiphany.ephy_shell_get_default()
    shell.new_tab(window,previous_tab, license_url,
        epiphany.NEW_TAB_IN_EXISTING_WINDOW |
        epiphany.NEW_TAB_JUMP |
        epiphany.NEW_TAB_OPEN_PAGE)

def detect_license(window, embed):
    # Get the HTML code
    persist = epiphany.ephy_embed_factory_new_object(epiphany.EmbedPersist)
    persist.set_flags(
        epiphany.EMBED_PERSIST_NO_VIEW | epiphany.EMBED_PERSIST_COPY_PAGE)
    persist.set_embed(embed)

    page_string = persist.to_string()
    license_url = get_license_url(page_string)

    if license_url is not None:
        statusbar = window.get_statusbar()
        statusbar._cc_url = license_url
        embed._has_cc = True
    else:
        embed._has_cc = False

def get_license_url(page_string):
    result = page_string.find(
        '<License rdf:about="http://creativecommons.org/licenses/')
    if result is not -1:
        # Get the license's url from RDF code
        begin = result + 20
        index = begin
        while index < (len(page_string) - 1):
            if page_string[index] is '"':
                return page_string[begin:index]
            else:
                index = index + 1
    else:
        return None

def attach_window(window):
    notebook = window.get_notebook()
    ui_init(window)
    signal_tab_switch = notebook.connect_after("switch_page",
        _switch_page_cb, window);
    notebook._cc_signal_tab_switch = signal_tab_switch

def detach_window(window):
    notebook = window.get_notebook()
    notebook.disconnect(notebook._cc_signal_tab_switch)
    del notebook._cc_signal_tab_switch
    ui_destroy(window)

def attach_tab(window, embed):
    signal_load_status = embed.connect_after("notify::load-status",
        _load_status_cb, window)
    embed._cc_signal_load_status = signal_load_status

def detach_tab(window, embed):
    embed.disconnect(embed._cc_signal_load_status)
    del embed._cc_signal_load_status
