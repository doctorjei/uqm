//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include "options.h"
#include "port.h"
#include "resintrn.h"
#include "libs/misc.h"
#include "libs/log.h"
#include "libs/uio/charhashtable.h"

const char *_cur_resfile_name;
// When a file is being loaded, _cur_resfile_name is set to its name.
// At other times, it is NULL.

static ResourceDesc *
lookupResourceDesc (ResourceIndex *idx, RESOURCE res) {
	return (ResourceDesc *) CharHashTable_find (idx->map, res);
}

void *
loadResourceDesc (ResourceIndex *idx, ResourceDesc *desc)
{
	desc->resdata = loadResource (desc->fname,
			idx->typeInfo.handlers[desc->restype].loadFun);
	return desc->resdata;
}

void *
loadResource(const char *path, ResourceLoadFun *loadFun)
{
	uio_Stream *stream;
	long dataLen;
	void *resdata;

	stream = res_OpenResFile (contentDir, path, "rb");
	if (stream == NULL)
	{
		log_add (log_Warning, "Warning: Can't open '%s'", path);
		return NULL;
	}

	dataLen = LengthResFile (stream);
	log_add (log_Info, "\t'%s' -- %lu bytes", path, dataLen);
	
	if (dataLen == 0)
	{
		log_add (log_Warning, "Warning: Trying to load empty file '%s'.", path);
		goto err;
	}

	_cur_resfile_name = path;
	resdata = (*loadFun) (stream, dataLen);
	_cur_resfile_name = NULL;
	res_CloseResFile (stream);

	return resdata;

err:
	res_CloseResFile (stream);
	return NULL;
}

// Get a resource by its resource ID.
void *
res_GetResource (RESOURCE res)
{
	ResourceIndex *resourceIndex;
	ResourceDesc *desc;
	
	if (res == NULL_RESOURCE)
	{
		log_add (log_Warning, "Trying to get null resource");
		return NULL;
	}
	
	resourceIndex = _get_current_index_header ();

	desc = lookupResourceDesc (resourceIndex, res);
	if (desc == NULL)
	{
		log_add (log_Warning, "Trying to get undefined resource '%s'",
				 res);
		return NULL;
	}

	if (desc->resdata != NULL)
		return desc->resdata;

	loadResourceDesc (resourceIndex, desc);

	return desc->resdata;
			// May still be NULL, if the load failed.
}

// NB: this function appears to be never called!
void
res_FreeResource (RESOURCE res)
{
	ResourceDesc *desc;
	ResourceFreeFun *freeFun;
	ResourceIndex *idx;

	desc = lookupResourceDesc (_get_current_index_header(), res);
	if (desc == NULL)
	{
		log_add (log_Debug, "Warning: trying to free an unrecognised "
				"resource.");
		return;
	}
	
	if (desc->resdata == NULL)
	{
		log_add (log_Debug, "Warning: trying to free not loaded "
				"resource.");
		return;
	}

	idx = _get_current_index_header ();
	freeFun = idx->typeInfo.handlers[desc->restype].freeFun;
	(*freeFun) (desc->resdata);
	desc->resdata = NULL;
}

// By calling this function the caller will be responsible of unloading
// the resource. If res_GetResource() get called again for this
// resource, a NEW copy will be loaded, regardless of whether a detached
// copy still exists.
void *
res_DetachResource (RESOURCE res)
{
	ResourceDesc *desc;
	void *result;

	desc = lookupResourceDesc (_get_current_index_header(), res);
	if (desc == NULL)
	{
		log_add (log_Debug, "Warning: trying to detach from an unrecognised "
				"resource.");
		return NULL;
	}
	
	if (desc->resdata == NULL)
	{
		log_add (log_Debug, "Warning: trying to detach from a not loaded "
				"resource.");
		return NULL;
	}

	result = desc->resdata;
	desc->resdata = NULL;

	return result;
}

BOOLEAN
FreeResourceData (void *data) {
	HFree (data);
	return TRUE;
}