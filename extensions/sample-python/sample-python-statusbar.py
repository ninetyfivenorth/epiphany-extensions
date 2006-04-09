#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Sample Python Statusbar extension: creates a statusbar icon using Python
#
# Copyright (C) 2005 Adam Hooper

import epiphany

# Icon generated using "gdk-pixbuf-csource --raw myimage.png"
_icon = ( ""
	#/* Pixbuf magic (0x47646b50) */
	"GdkP"
	#/* length: header (24) + pixel_data (2304) */
	"\0\0\11\30"
	#/* pixdata_type (0x1010002) */
	"\1\1\0\2"
	#/* rowstride (96) */
	"\0\0\0`"
	#/* width (24) */
	"\0\0\0\30"
	#/* height (24) */
	"\0\0\0\30"
	#/* pixel_data: */
	"\0\0\0\0\0\0\0\0\0\0\0\0\256FF\11\256FF\24\256FF!\256FF$\256FF'\256F"
	"F#\256FF\40\256FF\40\256FF\27\256FF\16\256FF\2\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\256FF\27\256FF<\256FFb\256FFx\256FF\205\256FF\202\256FF|\256F"
	"F{\256FFw\256FFi\256FFK\256FF&\256FF\11\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\26\256FFL\256"
	"FF\233\256FF\320\256FF\333\256FF\341\256FF\334\256FF\326\256FF\325\256"
	"FF\331\256FF\326\256FF\265\256FFv\256FF8\256FF\15\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\7\256FF/\256FF\216"
	"\256FF\337\256FF\372\256FF\366\256FF\367\256FF\361\256FF\355\256FF\355"
	"\256FF\362\256FF\373\256FF\357\256FF\314\256FF\177\256FF.\256FF\7\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\13\256FFK"
	"\256FF\273\256FF\366\256FF\362\256FF\341\256FF\322\256FF\306\256FF\276"
	"\256FF\301\256FF\321\256FF\347\256FF\365\256FF\362\256FF\271\256FFS\256"
	"FF\22\256FF\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\22"
	"\256FFX\256FF\305\256FF\360\256FF\345\256FF\260\256FFz\256FF\\\256FF"
	"T\256FFY\256FFz\256FF\262\256FF\344\256FF\363\256FF\322\256FFm\256FF"
	"\33\256FF\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\21"
	"\256FFa\256FF\320\256FF\372\256FF\347\256FF\225\256FF9\256FF\32\256F"
	"F\25\256FF\"\256FFM\256FF\230\256FF\332\256FF\362\256FF\324\256FFp\256"
	"FF\34\256FF\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\23"
	"\256FF\\\256FF\313\256FF\363\256FF\341\256FF\215\256FF\77\256FF,\256"
	"FF=\256FFZ\256FF\206\256FF\274\256FF\350\256FF\365\256FF\307\256FF_\256"
	"FF\26\256FF\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\16"
	"\256FFV\256FF\300\256FF\355\256FF\332\256FF\230\256FFt\256FF\202\256"
	"FF\240\256FF\300\256FF\332\256FF\350\256FF\353\256FF\337\256FF\237\256"
	"FF\77\256FF\13\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\256FF\16\256FFP\256FF\273\256FF\354\256FF\335\256FF\264\256FF\301"
	"\256FF\321\256FF\342\256FF\357\256FF\372\256FF\360\256FF\335\256FF\253"
	"\256FFd\256FF$\256FF\2\0\0\0\0\0\0\0\0\0\0\0\0\256FF\11\256FF\13\256"
	"FF\11\0\0\0\0\256FF\16\256FFP\256FF\273\256FF\356\256FF\346\256FF\307"
	"\256FF\337\256FF\346\256FF\344\256FF\350\256FF\347\256FF\313\256FF\242"
	"\256FF\177\256FF^\256FF4\256FF\20\0\0\0\0\0\0\0\0\256FF\20\256FF/\256"
	"FF@\256FF/\256FF\20\256FF\14\256FFL\256FF\266\256FF\356\256FF\355\256"
	"FF\303\256FF\256\256FF\254\256FF\246\256FF\241\256FF\222\256FFv\256F"
	"Ft\256FF\242\256FF\270\256FF\177\256FF.\256FF\11\256FF\11\256FF5\256"
	"FF\213\256FF\265\256FF\213\256FF5\256FF\14\256FFG\256FF\260\256FF\353"
	"\256FF\353\256FF\266\256FFk\256FFL\256FF@\256FF:\256FF0\256FF6\256FF"
	"s\256FF\325\256FF\366\256FF\274\256FFQ\256FF\15\256FF\22\256FFk\256F"
	"F\330\256FF\372\256FF\327\256FFj\256FF\13\256FFD\256FF\256\256FF\355"
	"\256FF\355\256FF\256\256FFK\256FF\27\256FF\12\256FF\10\256FF\10\256F"
	"F\40\256FFo\256FF\322\256FF\360\256FF\314\256FFg\256FF\32\256FF&\256"
	"FF\223\256FF\360\256FF\375\256FF\356\256FF\216\256FF\12\256FFA\256FF"
	"\247\256FF\347\256FF\347\256FF\247\256FFA\256FF\12\0\0\0\0\0\0\0\0\0"
	"\0\0\0\256FF\24\256FFb\256FF\320\256FF\371\256FF\335\256FF\205\256FF"
	",\256FF5\256FF\257\256FF\367\256FF\377\256FF\366\256FF\241\256FF\11\256"
	"FF;\256FF\240\256FF\342\256FF\342\256FF\240\256FF;\256FF\11\0\0\0\0\0"
	"\0\0\0\0\0\0\0\256FF\17\256FFM\256FF\272\256FF\355\256FF\352\256FF\257"
	"\256FF_\256FFb\256FF\302\256FF\372\256FF\376\256FF\365\256FF\246\256"
	"FF\11\256FF<\256FF\241\256FF\342\256FF\342\256FF\241\256FF<\256FF\11"
	"\0\0\0\0\0\0\0\0\0\0\0\0\256FF\6\256FF9\256FF\234\256FF\346\256FF\362"
	"\256FF\331\256FF\253\256FF\246\256FF\334\256FF\372\256FF\375\256FF\363"
	"\256FF\255\256FF\12\256FFA\256FF\247\256FF\350\256FF\350\256FF\247\256"
	"FFA\256FF\12\0\0\0\0\0\0\0\0\0\0\0\0\256FF\3\256FF\36\256FFp\256FF\313"
	"\256FF\364\256FF\355\256FF\346\256FF\342\256FF\361\256FF\375\256FF\374"
	"\256FF\365\256FF\267\256FF\13\256FFD\256FF\256\256FF\355\256FF\355\256"
	"FF\256\256FFD\256FF\13\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\256FF\14\256F"
	"F:\256FF\210\256FF\310\256FF\345\256FF\365\256FF\364\256FF\366\256FF"
	"\366\256FF\367\256FF\363\256FF\302\256FF\14\256FFB\256FF\250\256FF\347"
	"\256FF\347\256FF\250\256FFB\256FF\14\0\0\0\0\256FF\4\256FF\12\256FF\15"
	"\256FF\12\256FF\33\256FF\77\256FFr\256FF\245\256FF\304\256FF\316\256"
	"FF\316\256FF\327\256FF\356\256FF\370\256FF\314\256FF\10\256FF4\256FF"
	"\222\256FF\340\256FF\340\256FF\222\256FF4\256FF\10\256FF\10\256FF\36"
	"\256FF7\256FFC\256FFD\256FFA\256FFF\256FFP\256FF`\256FFq\256FF|\256F"
	"F\223\256FF\273\256FF\347\256FF\365\256FF\314\256FF\4\256FF\35\256FF"
	"Y\256FF\225\256FF\225\256FFY\256FF\35\256FF\10\256FF\33\256FFS\256FF"
	"\224\256FF\251\256FF\247\256FF\250\256FF\250\256FF\236\256FF\226\256"
	"FF\222\256FF\231\256FF\264\256FF\332\256FF\357\256FF\362\256FF\275\0"
	"\0\0\0\256FF\10\256FF\37\256FF5\256FF5\256FF\37\256FF\10\256FF\7\256"
	"FF(\256FF{\256FF\327\256FF\345\256FF\343\256FF\347\256FF\355\256FF\345"
	"\256FF\344\256FF\336\256FF\333\256FF\346\256FF\362\256FF\365\256FF\325"
	"\256FF\206\0\0\0\0\0\0\0\0\256FF\4\256FF\12\256FF\12\256FF\4\0\0\0\0"
	"\256FF\6\256FF!\256FFg\256FF\266\256FF\320\256FF\337\256FF\347\256FF"
	"\356\256FF\357\256FF\365\256FF\360\256FF\353\256FF\346\256FF\344\256"
	"FF\314\256FF\221\256FFD")

def attach_window(window):
	import gtk.gdk

	pixbuf = gtk.gdk.pixbuf_new_from_inline(len(_icon), _icon, False)
	frame = gtk.Frame()
	image = gtk.Image()
	image.set_from_pixbuf(pixbuf)
	evbox = gtk.EventBox()
	evbox.set_visible_window(False)

	frame.add(evbox)
	evbox.add(image)

	frame.show()
	evbox.show()
	image.show()

	statusbar = window.get_statusbar()
	statusbar.add_widget(frame)
	statusbar.tooltips.set_tip (evbox, "A useless tip")
	statusbar._sample_python_statusbar_frame = frame

def detach_window(window):
	statusbar = window.get_statusbar()

	frame = statusbar._sample_python_statusbar_frame
	del statusbar._sample_python_statusbar_frame

	statusbar.remove_widget(frame)
