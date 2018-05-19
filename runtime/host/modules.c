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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "str.h"
#include "boolean.h"
#include "config.h"
#include "ort_prive.h"

#include<stdio.h>

static ort_device_t *add_device(ort_module_t *module, int id_in_module)
{
	int added_devices = ort->added_devices;

	ort->ort_devices[added_devices].id           = added_devices;
	ort->ort_devices[added_devices].id_in_module = id_in_module;
	ort->ort_devices[added_devices].module       = module;
	ort->ort_devices[added_devices].initialized  = false;
	ort->ort_devices[added_devices].device_info  = NULL;

	return &(ort->ort_devices[ort->added_devices++]);
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
	//TODO LibDir should be provided by ompicc, and not during compilation of
	//ompi, in order for modules to work in portable mode
	str_printf(tmp, "%s/devices/%s/hostmodule.so", LibDir, name);
	//fprintf(stderr, "-----%s\n", str_string(tmp));
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

static inline void *load_function(void *module, char *moduleName, char *func)
{
	char *error;
	void *temp;

	temp = dlsym(module, func);

	if ((error = dlerror()) != NULL)
	{
		ort_warning("module: %s, function: %s, %s\n", moduleName, func, error);
		return NULL;
	}

	return temp;
}

void ort_discover_modules(int nModules, va_list ap)
{
	char    *temp;
	        /* we also count the host in nDevices */
	int     nDevices = 1, i = 0, j;
	int     (*get_num_devices)(void);

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

			get_num_devices = load_function(module, ort->modules[i].name,
			                                "hm_get_num_devices");
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

	//ort_add_device("HOST Device", "System sh_memory SMP");/* OpenMP 4.0.0 */
	add_device(NULL, 0)->initialized = true; /* Host */
	for (i = 0; i < nModules; i++)
	{
		for (j = 0; j < ort->modules[i].number_of_devices; j++)
			add_device(&(ort->modules[i]), j);
	}

	ort->num_modules = nModules;
}

static bool load_functions(ort_module_t *m)
{
	void (*register_ee_calls)(void (*)(omp_lock_t *, int), void (*)(omp_lock_t *),
	                          void (*)(omp_lock_t *), int  (*hyield_in)(void));
	void (*set_module_name)(char*);

	if ((m->initialize = load_function(m->handle, m->name,
	                                   "hm_initialize")) == NULL)
		return false;
	if ((m->finalize = load_function(m->handle, m->name, "hm_finalize")) == NULL)
		return false;
	if ((m->offload = load_function(m->handle, m->name, "hm_offload")) == NULL)
		return false;
	if ((m->dev_alloc = load_function(m->handle, m->name, "hm_dev_alloc")) == NULL)
		return false;
	if ((m->dev_free = load_function(m->handle, m->name, "hm_dev_free")) == NULL)
		return false;
	if ((m->todev = load_function(m->handle, m->name, "hm_todev")) == NULL)
		return false;
	if ((m->fromdev = load_function(m->handle, m->name, "hm_fromdev")) == NULL)
		return false;
	if ((m->get_dev_address = load_function(m->handle, m->name,
	                                        "hm_get_dev_address")) == NULL)
		return false;

	if ((set_module_name = load_function(m->handle, m->name,
	                                   "hm_set_module_name")) == NULL)
		return false;
	set_module_name(m->name);

	if ((register_ee_calls = load_function(m->handle, m->name,
	                                       "hm_register_ee_calls")) == NULL)
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
		d->device_info = (d->module->initialize)(d->id_in_module, &(ort->icvs));

		/* Initialize device lock */
		d->lock = (volatile void *)ort_alloc(sizeof(ee_lock_t));
		ee_init_lock((ee_lock_t *) d->lock, ORT_LOCK_NORMAL);
		FENCE; /* 100% initialized, before been assigned to "lock" */
	}
}

void ort_finalize_devices() {
	int i;

	for (i = 0; i < ort->added_devices; i++)
		if (ort->ort_devices[i].initialized && ort->ort_devices[i].device_info)
		{
			ort->ort_devices[i].module->finalize(ort->ort_devices[i].device_info);

			/* Deinitialize and free device lock */
			ee_destroy_lock((ee_lock_t *) ort->ort_devices[i].lock);
			free((ee_lock_t *) ort->ort_devices[i].lock);
		}

	for (i = 0; i < ort->num_modules; i++)
		if (ort->modules[i].initialized && ort->modules[i].initialized_succesful)
			dlclose(ort->modules[i].handle);
}

#else   /* HAVE_DLOPEN */

void ort_discover_modules(int nmods, va_list ap) {
	ort->ort_devices = ort_alloc(1 * sizeof(ort_device_t));
	add_device(NULL, 0)->initialized = true; /* Host */
}

void ort_initialize_device(int device_id) {}

void ort_finalize_devices() {}

#endif  /* HAVE_DLOPEN */
