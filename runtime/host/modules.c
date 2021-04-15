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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "str.h"
#include "config.h"
#include "ort_prive.h"

#ifdef PORTABLE_BUILD
  char *InstallPath = "./";     /* dummy initialization */
#endif
static volatile ee_lock_t mod_lock; /* Lock for functions in this file */


static ort_device_t *add_device(ort_module_t *module, int id_in_module)
{
	ort_device_t *dev = &(ort->ort_devices[ort->num_devices]);

	dev->id           = ort->num_devices++;  /* advance the counter too */
	dev->id_in_module = id_in_module;
	dev->module       = module;
	dev->initialized  = false;
	dev->device_info  = NULL;
	return (dev);
}


/* Add and initialize the host "device" */
static void setup_host_moddev()
{
  ort_device_t *d;

	d = add_device(ort_get_host_module(), 0);
	d->initialized  = true;

	/* Initialize device lock */
	d->lock = (volatile void *)ort_alloc(sizeof(ee_lock_t));
	ee_init_lock((ee_lock_t *) d->lock, ORT_LOCK_NORMAL);
}


/**
 * Get the device descriptor given a device id
 * @param devid the device id
 * @return      the device descriptor (ort_device_t)
 */
ort_device_t *ort_get_device(int devid)
{
	if (devid == AUTODEV_ID)  /* Use the default device */
		devid = omp_get_default_device();

	if (devid >= 0 && devid < ort->num_devices)
	{
		ee_set_lock((ee_lock_t *) &mod_lock);
		if (!ort->ort_devices[devid].initialized)
			ort_initialize_device(devid);
		ee_unset_lock((ee_lock_t *) &mod_lock);

		/* If the device has failed to initialize correctly fall back to host */
		if (ort->ort_devices[devid].device_info == NULL)
			devid = HOSTDEV_ID;
	}
  else
		devid = ort_illegal_device("device", devid);

	return &(ort->ort_devices[devid]);
}


/**
 * Gives a one-time warning on illegal device id and returns a fallback id.
 * Used for uniform handling of the situation across the runtime.
 * @param reason a message describing the failing device
 * @param devid  the failing device id
 * @return       the fallback device id
 */
int ort_illegal_device(char *reason, int devid)
{
	static bool warned = false;
	
	if (ort->icvs.targetoffload == OFFLOAD_MANDATORY)
		ort_error(1, "Invalid %s value (%d); mandatory exiting.\n", reason, devid);

	if (!warned)
	{
		ee_set_lock((ee_lock_t *) &mod_lock);
		if (!warned)
		{
			ort_warning("Invalid %s value (%d); falling back to device 0 (host).\n",
			            reason, devid);
			warned = true; FENCE;
		}
		ee_unset_lock((ee_lock_t *) &mod_lock);
	}
	return (HOSTDEV_ID);
}


#ifdef HAVE_DLOPEN

#include <dlfcn.h>


static void *open_module(char *name, int type)
{
	void *handle;
	str tmp = Strnew();

	/* Check current folder */
	str_printf(tmp, "./%s.so", name);
	handle = dlopen(str_string(tmp), type);
	if (handle)
	{
		str_free(tmp);
		return handle;
	}

	/* Check ompi's library folder */
	str_truncate(tmp);
#ifdef PORTABLE_BUILD
	/* Use InstallPath as passed by ompicc */
	str_printf(tmp, "%slib/ompi/devices/%s/hostpart.so", InstallPath, name);
#else
	/* Use hard-coded LibDir as provided by configure */
	str_printf(tmp, "%s/devices/%s/hostpart.so", LibDir, name);
#endif
	handle = dlopen(str_string(tmp), type);
	if (handle)
	{
		str_free(tmp);
		return handle;
	}

	/* Finally check system's library folder */
	str_truncate(tmp);
	str_printf(tmp, "%s.so", name);
	handle = dlopen(str_string(tmp), type);

	str_free(tmp);
	return handle;
}


static inline void *load_symbol(void *module, char *moduleName, char *sym, int show_warn)
{
	char *error;
	void *temp;

	temp = dlsym(module, sym);

	if (((error = dlerror()) != NULL) && show_warn)
	{
		ort_warning("module: %s, symbol: %s, %s\n", moduleName, sym, error);
		return NULL;
	}

	return temp;
}


void ort_discover_modules(int nModules, va_list ap)
{
	char *temp;
	int  i = 0, j, nDevices = 1;  /* we also count the host in nDevices */
	int  (*get_num_devices)(void);
	void ort_setup_host_moddev();

#ifdef PORTABLE_BUILD
	/* The installation path has sneaked in as 1st argument (not counted) */
	InstallPath = va_arg(ap, char *);
#endif

	ort->modules = ort_alloc(nModules * sizeof(ort_module_t));
	for (i = 0; i < nModules; i++)
	{
		ort->modules[i].name = va_arg(ap, char *);
		ort->modules[i].handle = NULL;

		void *module = open_module(ort->modules[i].name, RTLD_LAZY);
		if (!module)
			ort_warning("Failed to open module \"%s\"\n", ort->modules[i].name);
		else
		{
			/* Clear dlerror */
			dlerror();

			get_num_devices = load_symbol(module, ort->modules[i].name,
			                                "hm_get_num_devices", 1);
			if (get_num_devices != NULL)
			{
				ort->modules[i].initialized = false;
				ort->modules[i].number_of_devices = get_num_devices();
				nDevices += ort->modules[i].number_of_devices;
				dlclose(module);
				continue;
			}
			dlclose(module);
		}

		/* If we reached here we failed to get the number of devices */
		ort->modules[i].initialized = true;
		ort->modules[i].initialized_succesful = false;
		ort->modules[i].number_of_devices = 0;
	}

	ort->ort_devices = ort_alloc(nDevices * sizeof(ort_device_t));

	/* The host "module" and "device" 0; call it here to get id 0 */
	setup_host_moddev();

	for (i = 0; i < nModules; i++)
	{
		for (j = 0; j < ort->modules[i].number_of_devices; j++)
			add_device(&(ort->modules[i]), j);
	}

	ort->num_modules = nModules;

	ee_init_lock((ee_lock_t *) &mod_lock, ORT_LOCK_NORMAL);
}


static bool load_functions(ort_module_t *m)
{
	int *x;
	void (*register_ee_calls)(void (*)(omp_lock_t *, int), void (*)(omp_lock_t *),
	                          void (*)(omp_lock_t *), int  (*hyield_in)(void));
	void (*set_module_name)(char*);

	if ((x = load_symbol(m->handle, m->name, "hm_sharedspace", 1)) == NULL)
		return false;
	m->sharedspace = *x;
	if ((m->initialize = load_symbol(m->handle, m->name,
	                                   "hm_initialize", 1)) == NULL)
		return false;
	if ((m->finalize = load_symbol(m->handle, m->name, "hm_finalize", 1)) == NULL)
		return false;
	if ((m->offload = load_symbol(m->handle, m->name, "hm_offload", 1)) == NULL)
		return false;
	if ((m->dev_alloc = load_symbol(m->handle, m->name, "hm_dev_alloc", 1)) == NULL)
		return false;
	if ((m->dev_free = load_symbol(m->handle, m->name, "hm_dev_free", 1)) == NULL)
		return false;
	if ((m->todev = load_symbol(m->handle, m->name, "hm_todev", 1)) == NULL)
		return false;
	if ((m->fromdev = load_symbol(m->handle, m->name, "hm_fromdev", 1)) == NULL)
		return false;
	if ((m->imed2umed_addr = load_symbol(m->handle, m->name,
	                                        "hm_imed2umed_addr", 1)) == NULL)
		return false;
	if ((m->umed2imed_addr = load_symbol(m->handle, m->name,
	                                        "hm_umed2imed_addr", 1)) == NULL)
		return false;

	if ((set_module_name = load_symbol(m->handle, m->name,
	                                   "hm_set_module_name", 1)) == NULL)
		return false;
	set_module_name(m->name);

	if ((register_ee_calls = load_symbol(m->handle, m->name,
	                                       "hm_register_ee_calls", 1)) == NULL)
		return false;
	register_ee_calls(ort_prepare_omp_lock, omp_set_lock, omp_unset_lock,
	                  ee_yield);

	return true;
}


static void initialize_module(ort_module_t *m)
{
	m->initialized = true;
	m->initialized_succesful = false;

	m->handle = open_module(m->name, RTLD_NOW);
	if (!m->handle)
	{
		ort_warning("Failed to initialize module \"%s\"\n", m->name);
		return;
	}

	/* Clear dlerror */
	dlerror();

	m->initialized_succesful = load_functions(m);

	if (!m->initialized_succesful)
	{
		ort_warning("Failed to initialize module \"%s\" functions\n", m->name);
		dlclose(m->handle);
	}
}


void ort_initialize_device(int device_id)
{
	ort_device_t *d = &(ort->ort_devices[device_id]);

	if (d->initialized)
		return;

	d->initialized = true;

	/* Check if module is initialized */
	if (!d->module->initialized)
		initialize_module(d->module);


	if (!d->module->initialized_succesful)
		d->device_info = NULL;
	else
	{
		/* Call initialize function for device */
		d->device_info = (d->module->initialize)(d->id_in_module, &(ort->icvs),
				ort->argc, ort->argv);

		/* Initialize device lock */
		d->lock = (volatile void *)ort_alloc(sizeof(ee_lock_t));
		ee_init_lock((ee_lock_t *) d->lock, ORT_LOCK_NORMAL);
		FENCE; /* 100% initialized, before been assigned to "lock" */
	}
}


void ort_finalize_devices()
{
	int i;

	for (i = 0; i < ort->num_devices; i++)
		if (ort->ort_devices[i].initialized)
		{
			if (ort->ort_devices[i].device_info)
				ort->ort_devices[i].module->finalize(ort->ort_devices[i].device_info);

			/* Deinitialize and free device lock */
			ee_destroy_lock((ee_lock_t *) ort->ort_devices[i].lock);
			free((ee_lock_t *) ort->ort_devices[i].lock);
		};

	for (i = 0; i < ort->num_modules; i++)
		if (ort->modules[i].initialized && ort->modules[i].initialized_succesful)
			dlclose(ort->modules[i].handle);
}


#else   /* not HAVE_DLOPEN */


void ort_discover_modules(int nmods, va_list ap)
{
	ort->ort_devices = ort_alloc(1 * sizeof(ort_device_t));
	add_device(NULL, 0)->initialized = true;   /* Host */
	ee_init_lock((ee_lock_t *) &mod_lock, ORT_LOCK_NORMAL);
}

void ort_initialize_device(int device_id) {}

void ort_finalize_devices() {}


#endif  /* HAVE_DLOPEN */
