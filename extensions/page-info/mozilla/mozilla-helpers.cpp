/*
 *  Copyright (C) 2004 Adam Hooper
 *
 *  Ripped from GaleonWrapper.cpp, which has no copyright info besides Marco...
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mozilla-helpers.h"

#include <string.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#include <nsEmbedString.h>
#include <nsCOMPtr.h>
#include <nsICacheEntryDescriptor.h>
#include <nsICacheService.h>
#include <nsICacheSession.h>
#include <nsIDOMDocument.h>
#include <nsIDOMHTMLAnchorElement.h>
#include <nsIDOMHTMLCollection.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMHTMLFormElement.h>
#include <nsIDOMHTMLImageElement.h>
#include <nsIDOMHTMLLinkElement.h>
#include <nsIDOMLocation.h>
#include <nsIDOMNode.h>
#include <nsIDOMNodeList.h>
#include <nsIDOMNSDocument.h>
#include <nsIDOMNSHTMLDocument.h>
#include <nsIDOMWindow.h>
#include <nsMemory.h>
#include <nsNetCID.h>
#include <nsIServiceManagerUtils.h>
#include <nsIURI.h>
#include <nsIWebBrowser.h>
#include <nsTime.h>
#include <nsIHTMLDocument.h>
#include <nsCompatibility.h>

static char *
embed_string_to_c_string (const nsEmbedString& embed_string)
{
	nsEmbedCString c_string;
	NS_UTF16ToCString (embed_string, NS_CSTRING_ENCODING_UTF8, c_string);

	return g_strdup (c_string.get());
}

extern "C" void
mozilla_free_page_properties (EmbedPageProperties *props)
{
	g_free (props->content_type);
	g_free (props->encoding);
	g_free (props->referring_url);
	g_free (props);
}

static nsresult
get_cache_entry_descriptor (const char *url,
			    nsICacheEntryDescriptor **aCacheEntryDescriptor)
{
	nsresult rv;

	nsCOMPtr<nsICacheService> cacheService =
		do_GetService(NS_CACHESERVICE_CONTRACTID);
	NS_ENSURE_TRUE (cacheService, NS_ERROR_FAILURE);

	const char *cacheTypes[] = { "HTTP", "FTP" };
	for (unsigned int i = 0 ; i < G_N_ELEMENTS (cacheTypes); i++)
	{
		nsCOMPtr<nsICacheSession> cacheSession;
		cacheService->CreateSession(cacheTypes[i],
					    nsICache::STORE_ANYWHERE,
					    PR_TRUE,
					    getter_AddRefs(cacheSession));
		NS_ENSURE_TRUE (cacheSession, NS_ERROR_FAILURE);

		cacheSession->SetDoomEntriesIfExpired(PR_FALSE);

		nsCOMPtr<nsICacheEntryDescriptor> cacheEntryDescriptor;

		rv = cacheSession->OpenCacheEntry(url,
						  nsICache::ACCESS_READ,
						  PR_FALSE,
						  aCacheEntryDescriptor);

		if (NS_SUCCEEDED (rv)) return NS_OK;
	}
	*aCacheEntryDescriptor = NULL;
	return NS_ERROR_FAILURE;
}

extern "C" EmbedPageProperties *
mozilla_get_page_properties (EphyEmbed *embed)
{
	EmbedPageProperties *props;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	nsCOMPtr<nsIDOMWindow> domWindow;
	browser->GetContentDOMWindow (getter_AddRefs (domWindow));
	NS_ENSURE_TRUE (domWindow, NULL);

	nsCOMPtr<nsIDOMDocument> doc;
	domWindow->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, NULL);

	nsCOMPtr<nsIDOMNSDocument> domNSDoc = do_QueryInterface (doc);
	NS_ENSURE_TRUE (domNSDoc, NULL);

	nsEmbedString value;

	props = g_new0 (EmbedPageProperties, 1);

	nsresult rv;
	rv = domNSDoc->GetLastModified (value);
	NS_ENSURE_SUCCESS (rv, props);

	char *c_time = embed_string_to_c_string (value);
	nsTime last_modified (c_time, PR_TRUE);
	LL_DIV (props->modification_time,
		NS_STATIC_CAST(PRTime, last_modified), PR_USEC_PER_SEC);
	g_free (c_time);

	rv = domNSDoc->GetContentType (value);
	NS_ENSURE_SUCCESS (rv, props;);
	props->content_type = embed_string_to_c_string (value);

	rv = domNSDoc->GetCharacterSet (value);
	NS_ENSURE_SUCCESS (rv, props;);
	props->encoding = embed_string_to_c_string (value);

	/* Might not work on XUL pages */
	nsCOMPtr<nsIDOMHTMLDocument> domHtmlDoc = do_QueryInterface (doc);
	if (domHtmlDoc)
	{
		domHtmlDoc->GetReferrer (value);
		if (value.Length())
		{
			props->referring_url = embed_string_to_c_string (value);
		}
	}

	/* Might not work on XUL pages */
	nsCOMPtr<nsIHTMLDocument> htmlDoc = do_QueryInterface (doc);
	if (htmlDoc)
	{
		props->rendering_mode =
			(EmbedPageRenderMode) htmlDoc->GetCompatibilityMode();
	}

	/* Get the URL so we can look in the cache for this page */
	nsCOMPtr<nsIDOMLocation> domLocation;
	domNSDoc->GetLocation (getter_AddRefs (domLocation));
	NS_ENSURE_TRUE (domLocation, props);

	nsEmbedString url;
	domLocation->ToString (url);

	nsCOMPtr<nsICacheEntryDescriptor> cacheEntryDescriptor;
	char *c_url = embed_string_to_c_string (url);
	rv = get_cache_entry_descriptor (c_url,
					 getter_AddRefs (cacheEntryDescriptor));
	g_free (c_url);
	NS_ENSURE_SUCCESS (rv, props);

	if (cacheEntryDescriptor)
	{
		PRUint32 expiry = 0, dataSize = 0;
		char *source = nsnull;

		cacheEntryDescriptor->GetExpirationTime (&expiry);
		cacheEntryDescriptor->GetDataSize (&dataSize);
		cacheEntryDescriptor->GetDeviceID (&source);
		NS_ENSURE_TRUE (source, props);

		props->expiration_time = expiry;
		props->size = dataSize;

		if (strcmp (source, "disk") == 0)
		{
			props->page_source = EMBED_SOURCE_DISK_CACHE;
		}
		else if (strcmp (source, "memory") == 0)
		{
			props->page_source = EMBED_SOURCE_MEMORY_CACHE;
		}
		else
		{
			props->page_source = EMBED_SOURCE_UNKNOWN_CACHE;
		}

		nsMemory::Free (source);
	}
	else
	{
		props->page_source = EMBED_SOURCE_NOT_CACHED;
		props->size = -1;
		props->expiration_time = 0;
	}

	return props;
}

extern "C" void
mozilla_free_embed_page_image (EmbedPageImage *image)
{
	g_free (image->url);
	g_free (image->alt);
	g_free (image->title);
	g_free (image);
}

extern "C" GList *
mozilla_get_images (EphyEmbed *embed)
{
	nsresult rv;
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *ret = NULL;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	nsCOMPtr<nsIDOMWindow> dom_window;
	browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_TRUE (dom_window, NULL);

	nsCOMPtr<nsIDOMDocument> doc;
	dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, NULL);

	nsCOMPtr<nsIDOMHTMLDocument> htmlDoc = do_QueryInterface(doc);
	NS_ENSURE_TRUE (htmlDoc, NULL);

	nsCOMPtr<nsIDOMHTMLCollection> nodes;
	/* FIXME: DOM spec recomments searching by name for "img", "object" */
	htmlDoc->GetImages(getter_AddRefs(nodes));

	PRUint32 count(0);
	nodes->GetLength(&count);
	for (PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIDOMNode> node;
		rv = nodes->Item(i, getter_AddRefs(node));
		if (NS_FAILED(rv) || !node) continue;

		nsCOMPtr<nsIDOMHTMLImageElement> element;
		element = do_QueryInterface(node, &rv);
		if (NS_FAILED(rv) || !element) continue;

		EmbedPageImage *image = g_new0(EmbedPageImage, 1);

		nsEmbedString tmp;
		rv = element->GetSrc(tmp);
		if (NS_SUCCEEDED(rv))
		{
			char *c = embed_string_to_c_string (tmp);
			if (g_hash_table_lookup(hash, c))
			{
				g_free (image);
				g_free (c);
				continue;
			}
			image->url = c;
			g_hash_table_insert(hash, image->url,
					    GINT_TO_POINTER(TRUE));
		}

		rv = element->GetAlt(tmp);
		if (NS_SUCCEEDED(rv))
		{
			image->alt = embed_string_to_c_string (tmp);
		}
		rv = element->GetTitle(tmp);
		if (NS_SUCCEEDED(rv))
		{
			image->title = embed_string_to_c_string (tmp);
		}
		rv = element->GetWidth(&(image->width));
		rv = element->GetHeight(&(image->height));

		ret = g_list_append(ret, image);
	}

	g_hash_table_destroy (hash);

	return ret;
}

extern "C" void
mozilla_free_embed_page_link (EmbedPageLink *link)
{
	g_free (link->url);
	g_free (link->title);
	g_free (link->rel);
	g_free (link);
}

template <class T>
static nsresult
process_link_node (nsIDOMNode *node,
		   nsIDOMDocument *doc,
		   GList **ret)
{
	nsresult rv;

	nsCOMPtr<T> element = do_QueryInterface(node);
	NS_ENSURE_TRUE (element, NS_ERROR_FAILURE);

	nsEmbedString tmp;

	EmbedPageLink *link = g_new0 (EmbedPageLink, 1);

	rv = element->GetHref(tmp);
	NS_ENSURE_SUCCESS (rv, rv);
	link->url = embed_string_to_c_string (tmp);

	rv = element->GetTitle(tmp);
	NS_ENSURE_SUCCESS (rv, rv);
	if (tmp.Length())
	{
		link->title = embed_string_to_c_string (tmp);
	}

	rv = element->GetRel(tmp);
	NS_ENSURE_SUCCESS (rv, rv);
	if (tmp.Length())
	{
		link->rel = embed_string_to_c_string (tmp);
	}
	if (!link->rel)
	{
		rv = element->GetRev(tmp);
		NS_ENSURE_SUCCESS (rv, rv);
		if (tmp.Length())
		{
			link->rel = embed_string_to_c_string (tmp);
		}
	}

	*ret = g_list_prepend(*ret, link);

	return NS_OK;
}

extern "C" GList *
mozilla_get_links (EphyEmbed *embed)
{
	nsresult rv;
	GList *ret = NULL;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, ret);

	nsCOMPtr<nsIDOMWindow> dom_window;
	browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_TRUE (dom_window, ret);

	nsCOMPtr<nsIDOMDocument> doc;
	dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, ret);

	/* first, get a list of <link> elements */
	nsCOMPtr<nsIDOMNodeList> links;
	nsEmbedString str_link;
	NS_CStringToUTF16 (nsEmbedCString("link"), NS_CSTRING_ENCODING_UTF8,
			   str_link);
	rv = doc->GetElementsByTagName (str_link, getter_AddRefs(links));
	NS_ENSURE_SUCCESS (rv, ret);

	PRUint32 links_count;
	rv = links->GetLength(&links_count);
	NS_ENSURE_SUCCESS (rv, ret);

	for (PRUint32 i = 0; i < links_count; i++)
	{
		nsCOMPtr<nsIDOMNode> node;
		rv = links->Item(i, getter_AddRefs(node));
		if (NS_FAILED(rv) || !node) continue;

		process_link_node<nsIDOMHTMLLinkElement>(node, doc, &ret);
	}

	/* next, get a list of anchors */
	nsCOMPtr<nsIDOMHTMLDocument> htmlDoc = do_QueryInterface(doc);
	NS_ENSURE_TRUE (htmlDoc, ret);

	nsCOMPtr<nsIDOMHTMLCollection> anchors;
	rv = htmlDoc->GetLinks(getter_AddRefs(anchors));
	NS_ENSURE_SUCCESS (rv, ret);

	PRUint32 anchor_count;
	anchors->GetLength(&anchor_count);
	for (PRUint32 i = 0; i < anchor_count; i++)
	{
		nsCOMPtr<nsIDOMNode> node;
		rv = anchors->Item(i, getter_AddRefs(node));
		if (NS_FAILED(rv) || !node) continue;

		process_link_node<nsIDOMHTMLAnchorElement>(node, doc, &ret);
	}

	ret = g_list_reverse (ret);

	return ret;
}

extern "C" void
mozilla_free_embed_page_form (EmbedPageForm *form)
{
	g_free (form->name);
	g_free (form->method);
	g_free (form->action);
	g_free (form);
}

extern "C" GList *
mozilla_get_forms (EphyEmbed *embed)
{
	nsresult rv;
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *ret = NULL;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	nsCOMPtr<nsIDOMWindow> dom_window;
	browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_TRUE (dom_window, NULL);

	nsCOMPtr<nsIDOMDocument> doc;
	dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, NULL);

	nsCOMPtr<nsIDOMHTMLDocument> htmlDoc = do_QueryInterface(doc);
	NS_ENSURE_TRUE (htmlDoc, NULL);

	nsCOMPtr<nsIDOMHTMLCollection> nodes;
	rv = htmlDoc->GetForms(getter_AddRefs(nodes));
	NS_ENSURE_SUCCESS (rv, NULL);

	PRUint32 count(0);
	rv = nodes->GetLength(&count);
	NS_ENSURE_SUCCESS (rv, NULL);

	for (PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIDOMNode> node;
		rv = nodes->Item(i, getter_AddRefs(node));
		if (NS_FAILED(rv) || !node) continue;

		nsCOMPtr<nsIDOMHTMLFormElement> element;
		element = do_QueryInterface(node, &rv);
		if (NS_FAILED(rv) || !element) continue;

		EmbedPageForm *form = g_new0 (EmbedPageForm, 1);

		nsEmbedString tmp;

		rv = element->GetAction(tmp);
		if (NS_SUCCEEDED(rv) && tmp.Length())
		{
			/*
			nsCOMPtr<nsIDocument> document;
			document = do_QueryInterface(doc, &rv);
			if (NS_FAILED(rv))
			{
				g_free(form);
				continue;
			}

			nsIURI *uri = document->GetDocumentURI();

			const nsACString &s = NS_ConvertUCS2toUTF8(tmp);
			nsCAutoString c;
			rv = uri->Resolve(s, c);

			form->action = c.Length() ?
				       g_strdup (c.get()) :
				       g_strdup (PromiseFlatCString(s).get());
			*/
			form->action = embed_string_to_c_string (tmp);
		}

		rv = element->GetName(tmp);
		if (NS_SUCCEEDED(rv) && tmp.Length())
		{
			form->name = embed_string_to_c_string (tmp);
		}

		rv = element->GetMethod(tmp);
		if (NS_SUCCEEDED(rv) && tmp.Length())
		{
			form->method = embed_string_to_c_string (tmp);
		}

		ret = g_list_prepend (ret, form);
	}

	ret = g_list_reverse (ret);

	return ret;
}