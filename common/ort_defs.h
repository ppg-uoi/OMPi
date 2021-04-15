/* Prototypes of functions used by the compiler in the generated code */
void ort_execute_serial(void *(*func)(void *),void *shared);
void ort_execute_parallel(void *(*func)(void *),void *shared,int num_threads,
	int iscombined,int procbind_type);
void ort_start_teams(void *(*func)(void *),void *shared,int num_teams,
	int thr_limit);

void *ort_get_thrpriv(void **key,int size,void *origvar);
void ort_sglvar_allocate(void **dataptr,int size,void *initer);
void ort_fence();
void ort_initreqs_add(void (*initfunc)(void));
void *ort_memalloc(int size);
void ort_memfree(void *ptr);

/*
 * From tasks.c
 */

void ort_new_task(void *(*func)(void *arg),void *arg,
                  int now, int final, int untied, int priority,
                  void **deparray, int noutdeps, int nindeps, int ninoutdeps);
void ort_taskwait(int waitall);
int  ort_task_throttling(void);
void *ort_task_immediate_start(int final);
void ort_task_immediate_end(void *tn);
void ort_entering_taskgroup(void);
void ort_leaving_taskgroup(void);

/*
 * From pools.c
 */

void *ort_taskenv_alloc(int size,void *(*task_func)(void *));
void ort_taskenv_free(void *ptr,void *(*task_func)(void *));

/* Atomic,critical,reduction,copyprivate and barrier */
void ort_atomic_begin();
void ort_atomic_end();
void ort_critical_begin(void **cl);
void ort_critical_end(void **cl);
void ort_broadcast_private(int num,...);
void ort_copy_private(int num,...);

/*
 * From barrier.c
 */

int ort_barrier_me(void);

/*
 * From cancel.c
 */

int ort_enable_cancel(int type);
int ort_check_cancel(int type);

/*
 * From worksharing.c
 */

/* Workshare-related functions */
int  ort_mysingle(int hasnowait);
void ort_leaving_single();
void ort_entering_sections(int hasnowait,int numberofsections);
void ort_leaving_sections();
int  ort_get_section();
void ort_entering_for(int nowait,int hasordered);
void ort_entering_doacross(int nowait, int nestdepth, int collapsenum, 
                   int schedtype, int chsize, long params[][3]);
int  ort_leaving_for();
void ort_for_curriter(unsigned long iter);
void ort_ordered_begin();
void ort_ordered_end();
void ort_doacross_post(long params[][3], long *idx);
void ort_doacross_wait(long params[][3], int ndeps, long *deps);

/* For schedules support */
typedef int (*chunky_t)(unsigned long niters, unsigned long chunksize,
    int monotonic, unsigned long *fiter, unsigned long *liter,
    int *extra);

void ort_get_runtime_schedule_stuff(chunky_t *func,unsigned long *chunksize);
int ort_get_guided_chunk(unsigned long niters, unsigned long chunksize, 
               int monotonic, unsigned long *fiter, unsigned long *liter, 
               int *ignored);
int ort_get_dynamic_chunk(unsigned long niters, unsigned long chunksize, 
               int monotonic, unsigned long *fiter, unsigned long *liter, 
               int *ignored);
int ort_get_runtimestatic_chunk(unsigned long niters, unsigned long chunksize, 
               int monotonic, unsigned long *fiter, unsigned long *liter, 
               int *chunkid);
int ort_get_static_default_chunk(unsigned long niters, 
                                 unsigned long *from, unsigned long *to);

/*
 * From target.c
 */

void *ort_dev_gaddr  (void *medaddr);
#pragma omp declare target
char *devpart_med2dev_addr(void *medaddr,unsigned long size);
#pragma omp end declare target

/*
 * From reduction.c
 */

void ort_reduction_begin(void **cl);
void ort_reduction_end(void **cl);

void ort_reduce_add(int, void *, void *, int);
void ort_reduce_subtract(int, void *, void *, int);
void ort_reduce_multiply(int, void *, void *, int);
void ort_reduce_and(int, void *, void *, int);
void ort_reduce_or(int, void *, void *, int);
void ort_reduce_max(int, void *, void *, int);
void ort_reduce_min(int, void *, void *, int);
void ort_reduce_bitand(int, void *, void *, int);
void ort_reduce_bitor(int, void *, void *, int);
void ort_reduce_bitxor(int, void *, void *, int);

void ort_offload_kernel(void *(*host_func)(void *),void *vars,void *declvars,
	char *kernel_filename_prefix,int devnum,...);
void *ort_devdata_alloc(unsigned long size,int devnum);
void ort_devdata_free(void *data,int devnum);

void *ort_start_target_data(int tdvars,int devnum);
void ort_end_target_data(void *de);
void ort_map_tdvar(void *var,unsigned long size,void *varlb,int isptr,int init);
void ort_unmap_tdvar(void *var, int update);
void ort_map_var_dev(void *var,unsigned long size,void *varlb,int isptr,
	int devid, int init);
void ort_unmap_var_dev(void *var, int  devid, int update, int delete);
void ort_read_var_dev(void *var,unsigned long size,void *varlb,int devnum);
void ort_write_var_dev(void *var,unsigned long size,void *varlb,int devnum);
void *ort_unmappedcopy_dev(void *buf, unsigned long size, int devnum);
void ort_unmappedfree_dev(void *umed, int devnum);
void *ort_host2med_addr(void *var, int devnum);
void ort_decltarg_register(void *var,unsigned long size,const void *init,
	int bylink);
void *ort_decltarg_host2med_addr(void *var,int devnum);

/*
 * From rt_common.h
 */

void ort_kerneltable_add(char *name, void *(*kernel_function)(void *));
