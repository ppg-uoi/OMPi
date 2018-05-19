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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ort_prive.h"
#include "dataenv.h"
#include "set.h"

/* OpenMP4.0.0.pdf page 177 line 14:
 * The original and corresponding list items may share storage
 */
#define HOST_DENVVARS_SHARE


struct  decl_data {
	const void *init;
	size_t      size;
	void      **device_data;
};

SET_TYPE_DEFINE(decl, void *, struct decl_data, 1031)
SET_TYPE_IMPLEMENT(decl)

static set(decl) decl_vars;

static void *get_declvar_entry(void *var, ort_device_t *d);

/*
 * This function emulates the ort_dev_gaddr
 * function of device.
 */
void *ort_dev_gaddr(void *local_address)
{
	return local_address;
}

/*
 * This function emulates the devrt_get_dev_address
 * function of device.
 */
void *devrt_get_dev_address(void *local_address, unsigned long size)
{
	return local_address;
}


static int get_device_index(int dev_id)
{
	/* Return default device */
	if (dev_id == AUTO_DEVICE)
		dev_id = omp_get_default_device();

	/* If the device number is outside the accepted range return the host */
	if (dev_id < 0 || dev_id >= ort->added_devices)
	{
		static bool warned = false;

		if (!warned)
		{
			ort_warning("Invalid device id '%d'. Will execute on host.\n", dev_id);
			warned = true;
		}
		return HOST_INDEX;
	}

	/* Check if device is initialized */
	if (!ort->ort_devices[dev_id].initialized)
		ort_initialize_device(dev_id);

	/* If the device has failed to initialize correctly run on host */
	if (ort->ort_devices[dev_id].device_info == NULL)
		return HOST_INDEX; /* Run on host */

	return dev_id;
}

ort_device_t *ort_get_device(int dev_id)
{
	return &(ort->ort_devices[get_device_index(dev_id)]);
}


/* This function allocates space for kernel
 * arguments.
 */
void *ort_devdata_alloc(unsigned long size, int device_num)
{
	ort_device_t *d = ort_get_device(device_num);

	if (d->id == HOST_ID) /* Run on host */
		return ort_alloc(size);

	/*
	 * When used in OpenCL device the returned memory is mapped with
	 * read/write permissions.
	 */
	return d->module->dev_alloc(d->device_info, size, 1);
}

/*
 * Kernel data deallocation
 */
void ort_devdata_free(void *data, int device_num)
{
	ort_device_t *d = ort_get_device(device_num);

	if (d->id == HOST_ID) /* Run on host */
		free(data);
	else
	/*
	 * When used in OpenCL device the memory to be freed has to be unmapped
	 */
		d->module->dev_free(d->device_info, data, 1);
}


/*
 * This function is called by the compiler to denote that
 * a new target data region begins. Every target data region
 * should get a private tdvar_map table. Runtime stores the
 * current tdvar_map in the task icvs. It returns the previous
 * tdvar_map to the compile.
 */
void *ort_start_target_data(int tdvars, int device_num)
{
	ort_device_t *d  = ort_get_device(device_num);
	ort_task_icvs_t *my_icvs = &(__CURRTASK(__MYCB)->icvs);
	void *old_de = my_icvs->cur_de;

	my_icvs->cur_de = dataenv_start(tdvars, old_de, d->id);

	return old_de;
}

static void host_free(void *device_info, void *devaddr, int unmap_memory)
{
	#ifndef HOST_DENVVARS_SHARE
		free(devaddr);
	#endif
}

/*
 * This function is called by the compiler to denote that
 * a target data region has ended. The parameter is the
 * previous device environment.
 */
void ort_end_target_data(void *de)
{
	ort_task_icvs_t *my_icvs = &(__CURRTASK(__MYCB)->icvs);
	dataenv       dtenv = (dataenv)(my_icvs->cur_de);
	ort_device_t *d     = ort_get_device(dtenv->device_id);

	if (d->id == HOST_ID) /* Run on host */
		dataenv_end(my_icvs->cur_de, host_free);
	else
		dataenv_end(my_icvs->cur_de, d->module->dev_free);

	my_icvs->cur_de = de;
}

static void read_tdvar(void *var, size_t size, ort_device_t *d,
                       void *shared_address)
{
	if (d->id == HOST_ID) /* Run on host */
	{
		#ifndef HOST_DENVVARS_SHARE
			memcpy(var, shared_address, size);
		#endif
	}
	else
		d->module->fromdev(d->device_info, var, shared_address, size);
}

static void write_tdvar(void *var, size_t size, ort_device_t *d,
                        void *shared_address)
{
	if (d->id == HOST_ID) /* Run on host */
	{
		#ifndef HOST_DENVVARS_SHARE
			memcpy(shared_address, var, size);
		#endif
	}
	else
		d->module->todev(d->device_info, var, shared_address, size);
}

/* This function defines memory in shared address space for
 * a target data variable.
 */
void *ort_alloc_tdvar(void *var, unsigned long size)
{
	dataenv       dtenv = (dataenv)(__CURRTASK(__MYCB)->icvs.cur_de);
	ort_device_t *d     = ort_get_device(dtenv->device_id);

	if (dataenv_get(dtenv, var, d->id))
		return NULL;

	if (d->id == HOST_ID) /* Run on host */
		#ifdef HOST_DENVVARS_SHARE
			return dataenv_put(dtenv, var, var);
		#else
			return dataenv_put(dtenv, var, ort_alloc(size));
		#endif

	return dataenv_put(dtenv, var,
	                   d->module->dev_alloc(d->device_info, size, 0));
}


/* This function defines memory in shared address space for
 *  a target data variable and then initializes shared var.
 */
void ort_init_tdvar(void *var, unsigned long size)
{
	dataenv       dtenv = (dataenv)(__CURRTASK(__MYCB)->icvs.cur_de);
	ort_device_t *d     = ort_get_device(dtenv->device_id);
	void         *shared_address;

	if (dataenv_get(dtenv, var, d->id))
		return;

	shared_address = ort_alloc_tdvar(var, size);

	write_tdvar(var, size, d, shared_address);
}


/* This function reads data from a target data var and copies them to
 * local (HOST) var.
 */
void ort_read_tdvar(void *var, unsigned long size, int device_num)
{
	void *cur_de = __CURRTASK(__MYCB)->icvs.cur_de;
	dataenv       dtenv = (dataenv)(cur_de);
	ort_device_t *d = ort_get_device(device_num);

	if (!(cur_de))
	{ /* Declared variable */
		if (d->id != HOST_ID)
			read_tdvar(var, size, d, get_declvar_entry(var, d));
	}
	else
		read_tdvar(var, size, d, dataenv_get(dtenv, var, d->id));

}


/* This function reads data from a local (HOST) var and copies them to
 * target data var.
 */
void ort_write_tdvar(void *var, unsigned long size, int device_num)
{
	void *cur_de = __CURRTASK(__MYCB)->icvs.cur_de;
	dataenv       dtenv = (dataenv)(cur_de);
	ort_device_t *d = ort_get_device(device_num);

	if (!(cur_de))
	{ /* Declared variable */
		if (d->id != HOST_ID)
			write_tdvar(var, size, d, get_declvar_entry(var, d));
	}
	else
		write_tdvar(var, size, d, dataenv_get(dtenv, var, d->id));
}


/*
 * This function checks whether the variable var exists in the current target data level.
 * If the above is true then I read the var data, otherwise I do nothing.
 */
void ort_finalize_tdvar(void *var, unsigned long size)
{
	dataenv       dtenv    = (dataenv)(__CURRTASK(__MYCB)->icvs.cur_de);
	ort_device_t *d        = ort_get_device(dtenv->device_id);
	void         *vaddress = dataenv_get_current_level(dtenv, var, d->id);

	if (vaddress == NULL)
		return;

	read_tdvar(var, size, d, vaddress);
}

/* This function searches mapping table for a target data var
 * and returns the address seen by the device
 */
void *ort_get_vaddress(void *var)
{
	dataenv       dtenv    = (dataenv)(__CURRTASK(__MYCB)->icvs.cur_de);
	ort_device_t *d        = ort_get_device(dtenv->device_id);
	void         *vaddress = dataenv_get(dtenv, var, d->id);

	if (vaddress == NULL)
		ort_error(1, "Error! Target data var not found.\n");

	if (d->id == HOST_ID) /* Run on host */
		return vaddress;

	return d->module->get_dev_address(d->device_info, vaddress);
}


/*
 * This function is used to offload kernel code for
 */
void ort_offload_kernel(void *(*host_func)(void *), void *vars,
                        void *declvars, char *kernel_filename_prefix,
                        int device_num, ...)
{
	ort_device_t *d = ort_get_device(device_num);
	va_list argptr;

	/*
	 * This argument list is used in OpenCL devices to declare the set of
	 * pointers to shared virtual memory that has to be registered, so it
	 * can be used by the device.
	 */
	va_start(argptr, device_num);

	if (d->id == HOST_ID) /* Run on host */
		/* HOST must execute kernel func.
		 * Must clain new eecb
		 */
		ort_execute_kernel_on_host(host_func, vars);
	else
		d->module->offload(d->device_info, host_func, vars, declvars,
		                   kernel_filename_prefix, /*TODO*/ 0, /*TODO*/ 0, argptr);

	va_end(argptr);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * DECLARED VARIABLES                                                *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
//TODO a finalize that deallocates all declared variables from the devices
void ort_initialize_decl ()
{
	decl_vars = set_new(decl);
}

void ort_call_decl_reg_func(void (**regfunc)(void))
{
	/* Lock for private access */
	ee_set_lock((ee_lock_t *) &ort->preparation_lock);

	if (*regfunc)
	{
		(*regfunc)();
		*regfunc = NULL;
	}

	ee_unset_lock((ee_lock_t *) &ort->preparation_lock);
}

void ort_register_declvar(void *var, unsigned long size, const void *init)
{
	setelem(decl) e;
	int           i;

	/* Do nothing if there are no devices */
	if (ort->added_devices <= 1)
		return;

	assert(set_get(decl_vars, var) == NULL);

	e = set_put(decl_vars, var);
	e->value.init = init;
	e->value.size = size;
	e->value.device_data = ort_alloc((ort->added_devices) * sizeof(void *));

	for (i = 1; i < ort->added_devices; i++)
		e->value.device_data[i] = NULL;
}

static void *get_declvar_entry(void *var, ort_device_t *d)
{;
	setelem(decl) e;

	if (d->id == HOST_ID) /* Run on host */
		return var;

	e = set_get(decl_vars, var);
	assert(e != NULL);

	/* Check if declared variable has been initialized on device */
	if (!e->value.device_data[d->id])
	{
		ee_set_lock((ee_lock_t *) d->lock);
		if (!e->value.device_data[d->id])
		{
			void *shared_address;
			shared_address = d->module->dev_alloc(d->device_info, e->value.size, 0);

			if (e->value.init)
				write_tdvar(var, e->value.size, d, shared_address);

			e->value.device_data[d->id] = shared_address;
		}
		ee_unset_lock((ee_lock_t *) d->lock);
	}

	assert(e->value.device_data[d->id] != NULL);


	return e->value.device_data[d->id];
}

void *ort_get_declvar(void *var, int device_num)
{
	ort_device_t *d = ort_get_device(device_num);

	if (d->id == HOST_ID) /* Run on host */
		return var;

	return d->module->get_dev_address(d->device_info, get_declvar_entry(var, d));
}
