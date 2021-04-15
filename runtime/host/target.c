/*
  OMPi OpenMP Compiler
  == Copyright since 2001 the OMPi Team
  == Dept. of Computer Science & Engineering, University of Ioannina

  This file is part of OMPi.

  OMPi is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OMPi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OMPi; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* target.c -- Handle target-related compiler calls */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ort_prive.h"
#include "targdenv.h"
#include "set.h"

//#define __DBG_DECLVARS
//#define __DBG_NORMVARS

#if defined(__DBG_DECLVARS) || defined(__DBG_NORMVARS)
static void dbgshow(char *msg, void *var, void *haddr, size_t off, size_t len,
                    void *imaddr, int dev, int ref)
{
	static int inited = 0;
	
	if (!inited)
	{
		fprintf(stderr, "From; Reason; Var; HostAddr; Dev(imed)Addr; Off; Len; "
		                "Ref; DevId\n\n");
		inited = 1;
	}
	if (imaddr)
	{
		/* var, haddr, daddr (imed), off, len, ref, [devid] */
		if (ref >= 0 && dev >= 0)
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; %10p;%ld;%ld;"
			       "%d;[%d]\n", msg, var, haddr, imaddr, off, len, ref, dev);
		else if (ref >= 0 && dev < 0)
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; %10p; %ld;%ld;"
			       "%d;\n", msg, var, haddr, imaddr, off, len, ref);
		else if (ref < 0 && dev >= 0)
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; %10p;%ld;%ld;"
			       ";[%d]\n", msg, var, haddr, imaddr, off, len, dev);
		else
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; %10p;%ld;%ld;"
			       ";\n", msg, var, haddr, imaddr, off, len);
	}
	else
	{
		if (ref >= 0 && dev >= 0)
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; ; %ld;%ld;"
			       "%d; [%d]\n", msg, var, haddr, off, len, ref, dev);
		else if (ref >= 0 && dev < 0)
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; ; %ld;%ld;"
			       "%d;\n", msg, var, haddr, off, len, ref);
		else if (ref < 0 && dev >= 0)
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; ; %ld;%ld;"
			       "; [%d]\n", msg, var, haddr, off, len, dev);
		else
			fprintf(stderr, "[host]; %-15.15s; %10p; %10p; ; %ld;%ld;;\n",
			        msg, var, haddr, off, len);
	}
}
#endif

#ifdef __DBG_DECLVARS
	#define DEBUGDECL(msg,var,haddr,off,len,daddr,dev,ref) \
		dbgshow((msg),(var),(haddr),(off),(len),(daddr),(dev),(ref))
#else
	#define DEBUGDECL(msg,var,haddr,off,len,daddr,dev,ref)
#endif

#ifdef __DBG_NORMVARS
	#define DEBUGNORM(msg,var,haddr,off,len,daddr,ref) \
		dbgshow((msg),(var),(haddr),(off),(len),(daddr),-111,(ref))
#else
	#define DEBUGNORM(msg,var,haddr,off,len,daddr,ref)
#endif


/* OpenMP4.0.0.pdf page 177 line 14:
 * The original and corresponding list items may share storage
 */


/**
 * Reads bytes from the device and transfers them to the host.
 * @param haddr Where the item's host address starts ("base" address)
 * @param hoff  Offset from the base address in bytes
 * @param maddr Where the corresponding item's mediary address starts
 * @param moff  Offset from maddr
 * @param size  Number of bytes to transfer
 * @param d     The device
 */
static
void device_read(void *haddr, size_t hoff, void *maddr, size_t moff,size_t size,
                 ort_device_t *d)
{
	if (!(d->module->sharedspace) && size > 0)
		d->module->fromdev(d->device_info, haddr, hoff, maddr, moff, size);
}


/**
 * Write bytes to the device from the host.
 * @param haddr Where the item's host address starts ("base" address)
 * @param hoff  Offset from the base address in bytes
 * @param maddr Where the corresponding item's mediary address starts
 * @param moff  Offset from daddr
 * @param size  Number of bytes to transfer
 * @param d     The device
 */
static
void device_write(void *haddr, size_t hoff, void *maddr,size_t moff,size_t size,
                  ort_device_t *d)
{
	if (!(d->module->sharedspace) && size > 0)
		d->module->todev(d->device_info, haddr, hoff, maddr, moff, size);
}


/**
 * Creates (allocates space for) a corresponding item on the given device.
 * This is only called from map_var().
 * @param hostaddr The address of the original host item
 * @param size     The space it occupies in bytes
 * @param offset   Where (#bytes after hostaddr) to start mapping from
 * @param d        The device
 * @return         The item's address on the device (mediary)
 */
static
void *device_alloc(void *hostaddr, size_t size, size_t offset, ort_device_t *d)
{
	if (d->module->sharedspace)
		return ( hostaddr+offset );
	else
		return ( d->module->dev_alloc(d->device_info, size, 0, hostaddr) );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * DECLARE-TARGET (GLOBAL) VARIABLES                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* TODO: ort_finalize_decl() to free all declared variables from devices
 */


void ort_decltarg_initialize()
{
	tdenv_init_globals(); /* Create the global device environments */
}


/**
 * Adds a new #declare target variable. No mapping at this point (lazy).
 * @param var    the host var
 * @param size   the var's size
 * @param init   pointer to initialize a device's copy from
 * @param bylink (flag) true if it is a link() variable
 */
void ort_decltarg_register(void *var, unsigned long size, const void *init,
                           int bylink)
{
	static int init_imaddr = 1;
	int i;
	tditem_t e;

	if (ort->num_devices <= 1)
		return;                     /* Do nothing if there are no devices */
	if (tdenv_global_get(HOSTDEV_ID, var) != NULL)  /* Should never happen */
		return;
	for (i = 0; i < ort->num_devices; i++) /* Insert in all devices; no locking */
	{
		e = tdenv_put(tdenv_global_env(i), var, var, 0L, size, NULL, 1);
		e->decltarg = 1;
		e->bylink   = bylink;
		e->initfrom = (void *) init;
		e->init_imedaddr = init_imaddr;
	}
	++init_imaddr;
}


/* Bind a declare-target link var to its device address */
void decltarg_link_var(void *var, void *imedaddr, int devid)
{
	DEBUGDECL("linking ", var, var, 0L, 0L, imedaddr, devid, -1);
	tdenv_global_get(devid, var)->imedaddr = imedaddr;
}


/* Unbind a declare-target link var from its device address */
void decltarg_unlink_var(void *var, int devid)
{
	DEBUGDECL("unlinking ", var, var, 0L, 0L, NULL, devid, -1);
	tdenv_global_get(devid, var)->imedaddr = NULL;
}


/* Check if a variable is global (#declare'd) */
int decltarg_is_var(void *var, int devid)
{
	tditem_t e = tdenv_global_get(devid, var);
	return ( e && e->decltarg );
}


/**
 * Returns the mediary address of the given declare-target item; if the
 * corresponding item has not been mapped yet, it allocates and initializes
 * it properly, given it is a normal (TO) global var.
 * If it is a link() var, it returns whatever the mapping table reports.
 * Consequenlty, all link vars() SHOULD have been mapped already (otherwise
 * a NULL will be returned).
 * @param var the host variable
 * @param d   the device we refer to
 * @return    the internal mediary address of the corresponding item
 */
static
void *decltarg_get_imedaddr(void *var, ort_device_t *d)
{
	tditem_t e;
	int      did = d->id;

	if (did == HOSTDEV_ID)     /* TODO: Maybe not */
		return var;

	e = tdenv_global_get(did, var);
	assert(e != NULL);         /* sanity check */
	/* Check if declared variable has been initialized on device */
	if (DTto(e) && !IsMapped(e))
	{
		ee_set_lock((ee_lock_t *) d->lock);
		if (!IsMapped(e))
		{
			static char *zeroblock = NULL;  /* Only used for non-inited globals */
			static int  zeroblocksize = 0;
			void *imedaddr = d->module->dev_alloc(d->device_info, e->seclen, 0, var);
			
			assert(imedaddr != NULL);
			tdenv_global_get(did, var)->imedaddr = imedaddr;
			if (e->initfrom)
				device_write((void*) e->initfrom, 0L, imedaddr, 0L, e->seclen, d);
			else
			{
				/* Must initialize to all zeroes, so get a large enough 0s block... */
				if (e->seclen > zeroblocksize)
				{
					zeroblock = realloc(zeroblock, e->seclen);
					memset(zeroblock + zeroblocksize, 0, e->seclen - zeroblocksize);
					zeroblocksize = e->seclen;
				}
				device_write((void*) zeroblock, 0L, imedaddr, 0L, e->seclen, d);
			}
			DEBUGDECL("inited lazily", var, var, 0L, e->seclen, imedaddr, d->id, -1);
		}
		ee_unset_lock((ee_lock_t *) d->lock);
	}
	return tdenv_global_get(did, var)->imedaddr;
}


/**
 * Gets the usable address of a declare-target var on the device.
 * @param var    The host address of the item
 * @param devnum The device we are interested in
 * @return       The address of the corresponding item on the given device
 */
void *ort_decltarg_host2med_addr(void *var, int devnum)
{
	ort_device_t *d = ort_get_device(devnum);
	void         *imedaddr;

	if (d->id == HOSTDEV_ID)
		return var;
	if ((imedaddr = decltarg_get_imedaddr(var, d)) == NULL)
		return (NULL);   /* don't trust modules to handle this correctly.. */
	else
		return ( d->module->imed2umed_addr(d->device_info, imedaddr) );
}


/* Allocates space for kernel arguments */
void *ort_devdata_alloc(unsigned long size, int devnum)
{
	ort_device_t *d = ort_get_device(devnum);
	return d->module->dev_alloc(d->device_info, size, 1, NULL);
}


/* Kernel data deallocation */
void ort_devdata_free(void *data, int devnum)
{
	ort_device_t *d = ort_get_device(devnum);
	d->module->dev_free(d->device_info, data, 1);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * TARGET/TARGET DATA ENVIRONMENT ALLOC/READ/WRITE                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/**
 * Called by the compiler when a new target data region begins.
 * a) allocates a new hash table to store symbols (addresses).
 * b) stores the new table in the task icvs & makes it current
 *
 * @param tdvars The maximum number of symbols in this region
 * @param devnum The device this region refers to
 * @return The previous hash table (current at the time of call)
 */
void *ort_start_target_data(int tdvars, int devnum)
{
	ort_task_icvs_t *ticvs = &(__CURRTASK(__MYCB)->icvs);
	void            *prev = ticvs->cur_de;

	ticvs->cur_de = tdenv_start(tdvars, prev, ort_get_device(devnum)->id);
	return prev;
}


/**
 * Called by the compiler at the end of a target data region.
 *
 * @param de The previous device environment.
 */
void ort_end_target_data(void *de)
{
	tdenv_t      *dtenv = (tdenv_t *) &(__CURRTASK(__MYCB)->icvs.cur_de);
	ort_device_t *d     = ort_get_device((*dtenv)->devid);

	tdenv_end(*dtenv, d->module->sharedspace ? NULL : d->module->dev_free);
	*dtenv = de;
}


/* In the following ort_tdvar_*() calls we have:
 *
 *       var:  the variable (the "name") used to access the item's host space
 *  hostaddr:  the host space the variable refers to; normally var = hostaddr,
 *             except for pointers where hostaddr is the space var points to.
 *    offset:  the offset in bytes from hostaddr where the mapping starts from
 *      size:  the number of bytes mapped
 *   imedaddr: the corresponding space in the device, i.e. bytes
 *             [hostaddr+offset, hostaddr+offset+size) are mapped to
 *             [imedaddr, imedaddr+size)
 */


#define INCREASE(refctr) ( ((refctr) < INT_MAX) && ++(refctr) )


static
tditem_t add_to_env(tdenv_t t, void *var, void *hostaddr, size_t secoffset,
                    size_t seclen, void *imedaddr, int noalloc)
{
	tditem_t tdi;

	if (t->depth)
	{
		tdi = tdenv_put(t, var, hostaddr, secoffset, seclen, imedaddr, noalloc);
		INCREASE(tdi->refctr);
	}
	else    /* global environment; lock */
	{
		ort_device_t *d = ort_get_device(t->devid);

		ee_set_lock((ee_lock_t *) d->lock);
		tdi = tdenv_put(t, var, hostaddr, secoffset, seclen, imedaddr, noalloc);
		//TODO: should new (included) mappings of link vars be marked as "link" too?
		INCREASE(tdi->refctr);
		ee_unset_lock((ee_lock_t *) d->lock);
	}
	return tdi;
}


/**
 * Associates (maps) an existing host space with an existing device space.
 * It is basically a stripped-down version of map_var() which mainly 
 * puts an entry in the environment map table. It is assumed that devid is
 * not equal to HOSTDEV_ID.
 *
 * @param hostaddr The host pointer
 * @param umedaddr The device pointer (usable mediary address)
 * @param size     The space size in bytes
 * @param devoff   Where to start mapping from (varlb-var gives the offset)
 * @param devid    The device id
 * @return 0 if all OK, non-zero if not OK
 */
int target_associate_ptr(void *hostaddr, void *umedaddr, 
                            unsigned long size, unsigned long devoff, int devid)
{
	ort_device_t *d = ort_get_device(devid);
	tditem_t     tdi;
	void         *imedaddr = d->module->umed2imed_addr(d->device_info, umedaddr);
	tdenv_t      dtenv = (tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de);
	
	if (!dtenv) dtenv = tdenv_global_env(devid);  /* Resort to globals */
	if ((tdi = tdenv_get(dtenv, devid, hostaddr)) != NULL)  /* Mapped already */
	{
		/* v4.5 specs, page 288: 
		 * Only one device buffer can be associated with a given host pointer value 
		 * and device number pair. Attempting to associate a second buffer will 
		 * return non-zero. Associating the same pair of pointers on the same device 
		 * with the same offset has no effect and returns zero. 
		 */
		if (tdi->imedaddr != imedaddr || tdi->secoffset != devoff)
		{
			ort_warning("host pointer %p already associated on device %d\n",
			            hostaddr, devid);
			return -1;
		}
		return 0;
	}
	/* New mapping */
	tdi = add_to_env(dtenv, hostaddr, hostaddr, devoff, size, imedaddr, 1);
	tdi->refctr = INT_MAX;
	tdi->firstmapguard = 1; /* first variable to ask for this range */
	DEBUGNORM("associate", hostaddr, hostaddr, devoff, size, imedaddr, 
	                       tdi->refctr);
	return 0;
}


/**
 * Adds a variable mapping to the given data environment.
 * Given that the OpenMP rules permit ('cause the request may be ingored), it:
 * a) allocates space for the corresponding variable on the related device
 * b) optionally initializes it
 * c) puts an entry in the environment map table
 * To enforce OpenMP rules, there is some bound checking to ensure there is
 * no overlap with previous mappings:
 * "If any part of the original storage of a list item has corresponding 
 * storage in the device data environment, all of the original storage must 
 * have corresponding storage in the device data environment" (v45, p.218).
 * When it comes to an array section, this means that if any element is already
 * mapped, then all the elements must have been mapped, i.e. if an element
 * of x[l:s] is mapped, all x[l:s] is mapped. This can only happen
 * if a superset of x[l:s] is already mapped; hence the range checks.
 * 
 * @param dtenv  The data environment to put the var into
 * @param var    The name through which the original host item is accessed
 * @param size   The space the item occupies in bytes
 * @param varlb  Where to start mapping from ((*)varlb-var gives the offset)
 * @param isptr  True if var is a pointer (i.e. not an array)
 * @param init   True if the corresponding item should be initialized
 */
static
void map_var(tdenv_t dtenv,
               void *var, unsigned long size, void *varlb, int isptr, int init)
{
	ort_device_t *d = ort_get_device(dtenv ? dtenv->devid : AUTODEV_ID);
	tditem_t     tdi;
	void         *imedaddr, *hostaddr;

	//TODO: double-check @ HOST
	//TODO: optimize by using tdenv_get() first and checking for proper subset
	/* Zero-length pointer-based array sections are either explicit
	 * (then because of the way the compiler passes params, var != varlb) 
	 * or implicit (the compiler treats them as plain scalars, and var = varlb).
	 * For pointers, hostaddr always has the pointed-to address.
	 */
	hostaddr = isptr ? *((void **) var) : var;
	if (isptr && var == varlb)   /* Implicit zlas */
		varlb = hostaddr;   /* where var points to; i.e. make it explicit zlas */
		
	if (!dtenv)     /* only for #declare target link or #target enter data vars */
		dtenv = tdenv_global_env(d->id);
	switch ( tdenv_search_range(dtenv, hostaddr, size, varlb-hostaddr, &tdi) )
	{
		case RANGE_SAMEBASE:
			if (!isptr)
			  ort_error(1, "illegal (samebase) map requested @ %p (%ld bytes)\n",
			            varlb, size);
		case RANGE_NEW:
			/* Don't allocate for zero-length array sections */
			imedaddr = size ? device_alloc(hostaddr, size, varlb-hostaddr, d) : NULL;
			tdi = add_to_env(dtenv, var, hostaddr, varlb-hostaddr, size, imedaddr, 1);
			tdi->firstmapguard = 1; /* first variable to ask for this range */
			if (DTlink(tdi))
			{
				decltarg_link_var(var, imedaddr, dtenv->devid);
				DEBUGDECL((init && size) ? "map+init (link)" : "map (link)",
					var, hostaddr, varlb-hostaddr, size, imedaddr, d->id, tdi->refctr);
			}
			else
				DEBUGNORM((init && size) ? "map+init" : "map",
					var, hostaddr, varlb-hostaddr, size, imedaddr, tdi->refctr);
			if (init) /* TODO: Is locking needed here? */
				device_write(hostaddr, varlb-hostaddr, imedaddr, 0L, size, d);
			break;
		case RANGE_INCLUDED:         /* Already existing */
			if (tdi->hostvar != var)
			{
				tdi = add_to_env(dtenv,var,hostaddr,varlb-hostaddr,size,tdi->imedaddr,1);
				tdi->firstmapguard = 0;  /* was already mapped; we shouldn't free it */
			}
			else
				INCREASE(tdi->refctr);
			/* update always vars */
			if (init >= 2)            /* TODO: Is locking needed here? */
				device_write(hostaddr, varlb-hostaddr, tdi->imedaddr, 0L, size, d);
			if (DTlink(tdi)) //TODO: should we link it?
				DEBUGDECL((tdi->refctr>1)?"no map (link,ignored)":"map (link,included)",
					var, hostaddr, varlb-hostaddr, size, tdi->imedaddr,d->id,tdi->refctr);
			else
				DEBUGNORM((tdi->refctr > 1) ? "no map (ignored)" : "no map (included)",
					var, hostaddr, varlb-hostaddr, size, tdi->imedaddr, tdi->refctr);
			break;
		case RANGE_OVERLAPS:
			ort_error(1, "illegal (overlapping) map requested @ %p (%ld bytes)\n",
			          varlb, size);
	}
}


/**
 * Disassociates (unmaps) an existing host space from an existing device space.
 * It is basically a stripped-down version of unmap_var() which mainly 
 * deletes an entry from the environment map table. It is assumed that devid is
 * not equal to HOSTDEV_ID.
 * @param hostaddr The host pointer
 * @param devid    The device id
 * @return 0 if all OK, non-zero if not OK
 */
int target_disassociate_ptr(void *hostaddr, int devid)
{
	tditem_t tdi;
	tdenv_t  dtenv = (tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de);
	
	if (!dtenv) dtenv = tdenv_global_env(devid);  /* Resort to globals */
	if ((tdi = tdenv_get(dtenv, devid, hostaddr)) == NULL)  /* Not mapped */
		return -1;

	DEBUGNORM("disassociate", hostaddr, hostaddr, tdi->secoffset, tdi->seclen, 
		                          tdi->imedaddr, tdi->refctr);
	if ((tdi = tdenv_rem(dtenv, devid, hostaddr)) != NULL) /* remove */
		free(tdi);
	return (0);
}


/**
 * Adds a given item from a given data environment.
 * Given that the OpenMP rules permit ('cause the request may be ingored), it:
 * a) decreases the reference counter
 * b) may updates the original item (see below)
 * c) removes the entry from the environment map table
 * @param dtenv  The data environment
 * @param e      The item to unmap
 * @param update True if the original item should be updated; for map(from:)
 *               variables this should be 1, for map(always,:) it should be > 1.
 * @param delete True if the item should be deleted (i.e. zero its refctr).
 */
static
void unmap_var(tdenv_t dtenv, tditem_t e, int update, int delete)
{
	ort_device_t *d = ort_get_device(dtenv->devid);
	void       *var = e->hostvar;

	/* Decrement reference counter or clear it (delete) */
	if (e->refctr > 0)
	{
		if (delete)
			e->refctr = 0;
		else
			(e->refctr < INT_MAX) && (--(e->refctr)); /* don't decrease infinity */
	}

	if (dtenv->depth)
		DEBUGNORM(update >= 2 ? "unmapped + always update" :
		          update ? "unmapped + update" : "unmapped, no update",
		          var, (var != e->hostaddr) ? *((void**) var) : var,
		          e->secoffset, e->seclen, e->imedaddr, e->refctr);
	else
		DEBUGDECL(update >= 2 ? "unmapped global + always update" :
		          update ? "unmapped global + update" : "unmapped global, no update",
		          var, (var != e->hostaddr) ? *((void**) var) : var,
		          e->secoffset, e->seclen, NULL, dtenv->devid, e->refctr);
	if ((e->refctr == 0 && update) || update >= 2)  /* update>=2 => map(always) */
		/* Always use the pointed-to address */
		device_read(e->hostaddr, e->secoffset, e->imedaddr, 0L, e->seclen, d);

	if (e->refctr == 0)
	{
		if (DTlink(e))            /* unbind link() vars */
			decltarg_unlink_var(var, dtenv->devid);
		if ((e = tdenv_rem(dtenv, dtenv->devid, var)) != NULL)    /* free the item */
			if (!d->module->sharedspace && e->firstmapguard)
				d->module->dev_free(d->device_info, e->imedaddr, 0);
	}
}


/**
 * Maps a variable on the given device, and optionally initializes it;
 * called from #target/#target_data transformed code for map() items, when
 * starting a data environment. This is never called for declare target
 * variables, but it is be called for variables that have previously
 * been mapped through #target enter data.
 * @param var    The name through which the original host item is accessed
 * @param size   The space the item occupies in bytes
 * @param varlb  Where to start mapping from (varlb-var gives the offset)
 * @param isptr  True if var is a pointer (i.e. not an array)
 * @param init   True if the corresponding item should be initialized
 */
void ort_map_tdvar(void *var,unsigned long size,void *varlb,int isptr,int init)
{
	map_var((tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de),var,size,varlb,isptr,init);
}


/**
 * It unmaps a variable from the current device and optionally
 * updates it from the device's value. It is called from transformed
 * #target/#target_data code when closing a data environment. The variable
 * can never be a #declare target one.
 * @param var    The host (original) item
 * @param update True if original item should be updated
 */
void ort_unmap_tdvar(void *var, int update)
{
	tdenv_t       dtenv = (tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de);
	ort_device_t *d     = ort_get_device(dtenv->devid);
	tditem_t      e     = tdenv_get(dtenv, d->id, var);

	if (!e) /* Two reasons for this: it is global or has already been unmapped */
	{
		if ((e = tdenv_global_get(d->id, var)) != NULL)
			ort_unmap_var_dev(var, dtenv->devid, update, false);
		return;
	}
	unmap_var(dtenv, e, update, false);
	/* Actual unmapping is done when closing the environment */
	/* ???????? */
}


/**
 * Maps (allocates and optionally initializes) space for a link or an enter
 * variable on the given device; called from transformed code for map(to:)
 * items, upon a #target enter data or a #target (data) with link vars.
 * @param var    The name through which the original host item is accessed
 * @param size   The space the item occupies in bytes
 * @param varlb  Where to start mapping from (varlb-var gives the offset)
 * @param isptr  True if var is a pointer (i.e. not an array)
 * @param devid  The device id
 * @param init   If true, the corresponding item should be updated
 *               according to OpenMP rules (i.e. only when a new mapping is
 *               created); A value of >=2 forces an update ("always" var).
 */
void ort_map_var_dev(void *var, unsigned long size, void *varlb, int isptr,
                     int devid, int init)
{
	if (devid == AUTODEV_ID)  /* Use the default device */
		devid = omp_get_default_device();
	if (devid < 0 || devid >= ort->num_devices)
	{
		ort_warning("map_var_dev request (%p) for invalid device %d; "
		               "ignoring.\n", var, devid);
		return;
	}
	// FIXME: better have infinite counter for declate target TO variables
	{
		tditem_t e = tdenv_get(tdenv_global_env(devid), devid, var);
		if (DTto(e))
		{
			if (init >= 2)   /* always */
				ort_write_var_dev(var, size, varlb, devid);
			return;
		}
	}
	map_var(tdenv_global_env(devid), var, size, varlb, isptr, init);
}


/**
 * Unmaps (deallocates and optionally updates the original item) a link or
 * an enter variable (but normally mapped variables are not precluded though) 
 * on the given device; called from transformed code for map(to:) items, 
 * upon a #target exit data.
 * @param var    The name through which the original host item is accessed
 * @param devid  The device id
 * @param update True if the original item should be updated; for map(from:)
 *               variables this should be 1, for map(always,:) it should be > 1.
 * @param delete True if the corresponding item is to be deleted (only
 *               for #target exit data)
 */
void ort_unmap_var_dev(void *var, int devid, int update, int delete)
{
	tdenv_t  dtenv;
	tditem_t e;

	if (devid == AUTODEV_ID)  /* Use the default device */
		devid = omp_get_default_device();
	if (devid < 0 || devid >= ort->num_devices)
	{
		ort_warning("unmap_var_dev request (%p) for invalid device %d; "
		               "ignoring.\n", var, devid);
		return;
	}

	dtenv = tdenv_global_env(devid);
	e = tdenv_get(dtenv, devid, var);
	if (e == NULL)
	{
		dtenv = (tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de);
		e = dtenv ? tdenv_get(dtenv, devid, var) : NULL;
		if (e == NULL)
			return;    /* ignored according to OpenMP 4.5, 2.15.5.1 (p. 216) rules */
	}
	else
	{
		// FIXME: better have infinite counter for TO..
		{
			if (DTto(e))
			{
				if (update >= 2)   /* always */
					/* Hate to do it here, but... */
					device_read(e->hostaddr, e->secoffset, e->imedaddr, 0L, e->seclen,
											ort_get_device(devid));
				return;
			}
		}

		/* According to OpenMP 4.5, 2.15.5.1 (p. 216) rules */
		if (DTlink(e) && !IsMapped(e))   /* It *is* a link one, but is not mapped */
			return;
			/* Maybe we should not even display the warning
			ort_warning("ignoring unmap request for non-mapped %p%son device %d\n",
									var, !e ? " " : " (link) ", devid);
			*/
	}
	unmap_var(dtenv, e, update, delete);
}


/**
 * Checks if a variable (name) is mapped in the given device and returns its
 * internal mediary address. For the moment this is only called to implement
 * omp_target_is_present().
 * @param  var    The corresponding host item
 * @param  d      The device to read from
 * @param  how    This will become 1 if it is a #declare-target to() item,
 *                2 if it is a #declare-target link(), 3 if it is a
 *                #target-enter-data item and 0 otherwise.
 * @return        The mediary address or NULL if the item is not mapped.
 */
void *ort_checkmapped_var(void *var, ort_device_t *d, int *how)
{
	tdenv_t  dtenv = (tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de);
	tditem_t e;

	if (dtenv)
		if ((e = tdenv_get(dtenv, d->id, var)) != NULL)
		{
			how && (*how = 0);
			return (e->imedaddr);
		};

	/* Check in the global environment */
	if ((e = tdenv_global_get(d->id, var)) == NULL)
		return (NULL);
	if (DTto(e))
	{
		how && (*how = 1);
		return ( decltarg_get_imedaddr(var, d) );  /* Maybe just return TRUE? */
	}
	how && ( *how = DTlink(e) ? 2 : 3 );
	return (e->imedaddr);      /* not actively mapped link() vars give NULL */
}


/**
 * Transfers the contents of a host item to/from the corresponding one in the
 * given device.
 * @param var    The host item
 * @param size   The #bytes to transfer
 * @param varlb  Where to start from (varlb-var gives the offset)
 * @param devnum The device to read from
 * @param rd     True if it is a read / false if it is a write
 */
static
void xfer_var_dev(void *var, size_t size, void *varlb, int devnum,int rd)
{
	tdenv_t      dtenv = (tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de);
	ort_device_t *d    = ort_get_device(devnum);
	tditem_t     e = NULL;
	size_t       offset;
	void         *imedaddr;

	if (!dtenv)
		e = tdenv_global_get(d->id, var);
	else
		if ((e = tdenv_get(dtenv, d->id, var)) == NULL)    /* Two cases here .. */
			if ((e = tdenv_global_get(d->id, var)) == NULL)  /* .. global or not */
			{
				DEBUGNORM(rd ? " <--read >>> NULL->addr" : " <--write >>> NULL->addr",
				          var, *((void**) var), 0L, size, 0L, 0);
				var = *((void**) var);        /* can only be orphaned pointer */
				e = tdenv_get(dtenv, d->id, var);
			};

	if (!e || (DTlink(e) && !IsMapped(e)))
	{
		ort_warning("%s request (update) from unmapped %p%s ignoring.\n",
		            rd ? "read" : "write", var, !e ? ";" : " (link);");
		return;
	}

	imedaddr = DTto(e) ? decltarg_get_imedaddr(var, d) : e->imedaddr;
	offset = e->secoffset;

	if (e->decltarg)
		DEBUGDECL(rd ? " <--read (global)" : " <-write (global)", var,
		               ((var != e->hostaddr) ? *((void**)var) : var),
	                 offset, size, imedaddr, d->id, -1);
	else
		DEBUGNORM(rd ? " <--read" : " <-write", var,
	       ((var != e->hostaddr) ? *((void**)var) : var), offset, size, imedaddr,
	       e->refctr);

	if (var != e->hostaddr)    /* pointer */
		var = *((void**) var);
	offset = (varlb - var) - offset;  /* offset from the offset */
	if (0 <= offset && offset <= e->seclen - size)
	{
		if (rd)
			device_read(var, varlb-var, imedaddr, offset, size, d);
		else
			device_write(var, varlb-var, imedaddr, offset, size, d);
	}
	else
		ort_warning("read (update) at %p is outside the mapped range (%ld,%ld)\n",
		            varlb, varlb-var, size);
}


/**
 * Reads a mapped variable from the given device and stores iτ on
 * the corresponding host item; called from transformed code upon a
 * #target update from().
 * @param var    The corresponding host item
 * @param size   The #bytes to transfer
 * @param varlb  Where to start from (varlb-var gives the offset)
 * @param devnum The device to read from
 */
void ort_read_var_dev(void *var, unsigned long size, void *varlb, int devnum)
{
	xfer_var_dev(var, (size_t) size, varlb, devnum, 1);
}


/**
 * Writes (copies) to a mapped variable in the given device, from the
 * corresponding host item's value; called from transformed code upon a
 * #target update to().
 * @param var    The corresponding host item
 * @param size   The bytes to transfer
 * @param varlb  Where to start from (varlb-var gives the offset)
 * @param devnum The device to write to
 */
void ort_write_var_dev(void *var, unsigned long size, void *varlb, int devnum)
{
	xfer_var_dev(var, (size_t) size, varlb, devnum, 0);
}


/**
 * Allocates space and transfers the buffer to the given device.
 * @param buf    The host buffer
 * @param size   The size of the biffer in bytes
 * @param devnum The device id
 * @return       The usable/real mediary address of the device buffer
 */
void *ort_unmappedcopy_dev(void *buf, unsigned long size, int devnum)
{
	ort_device_t *d = ort_get_device(devnum);
	void *imed;
	
	if ((imed = device_alloc(buf, size, 0L, d)) == NULL)
	{
		ort_warning("%s unmapped copy from %p to device %d failed\n", buf, devnum);
		return NULL;
	}
	device_write(buf, 0L, imed, 0L, size, d);
	return d->module->imed2umed_addr(d->device_info, imed);
}


/**
 * Frees unmapped regions from device.
 * @param umed   The usable mediary address of the region
 * @param devnum The device id
 */
void ort_unmappedfree_dev(void *umed, int devnum)
{
	ort_device_t *d = ort_get_device(devnum);
	
	if (d->module->sharedspace) return;
	d->module->dev_free(d->device_info, 
	                    d->module->umed2imed_addr(d->device_info, umed),
	                    0);
}


/**
 * This function searches the mapping table for a target data var (original)
 * and returns the (usable) mediary address of the corresponding item in the
 * current device. Notice that if it is an array section, it returns the device
 * address where the section *starts* (not what the base address would be).
 * ((This is called when transforming #target, so no check for dtenv == NULL.))
 * The last sentence is no longer true (see use_device_ptr) so checks happen.
 * Also, one has to wonder why there is a separate function of #declare vars...
 * @param var    The host item
 * @param devnum The device id
 * @return    The corresponding item's usable/real mediary address
 */
void *ort_host2med_addr(void *var, int devnum)
{
	ort_device_t *d = ort_get_device(devnum);
	tdenv_t       dtenv = (tdenv_t)(__CURRTASK(__MYCB)->icvs.cur_de);
	tditem_t      e;

	e = dtenv ? tdenv_get(dtenv, d->id, var) : NULL;
	if (e != NULL)
		return ( d->module->imed2umed_addr(d->device_info, e->imedaddr) );
	if (tdenv_global_get(d->id, var) == NULL)   /* global then */
		ort_error(1, "ort_host2med_addr(): target var not found (%p).\n", var);
	return ( ort_decltarg_host2med_addr(var, devnum) );
}


/* This function is used to offload kernel code
 */
void ort_offload_kernel(void *(*host_func)(void *), void *vars,
                        void *declvars, char *kernel_filename_prefix,
                        int devnum, ...)
{
	ort_device_t *d = ort_get_device(devnum);
	va_list argptr;

	/* This argument list is used in OpenCL devices to declare the set of
	 * pointers to shared virtual memory that has to be registered, so it
	 * can be used by the device.
	 */
	va_start(argptr, devnum);
	d->module->offload(d->device_info, host_func, vars, declvars,
	                   kernel_filename_prefix, /*TODO*/ 0, /*TODO*/ 0, argptr);
	va_end(argptr);
}


#ifdef __DBG_DECLVARS
#undef __DBG_DECLVARS
#undef DEBUGDECL
#endif

#ifdef __DBG_NORMVARS
#undef __DBG_DECLVARS
#undef DEBUGNORM
#endif
