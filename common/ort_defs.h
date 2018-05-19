#pragma omp declare target
/* Prototypes of functions used by the compiler in the generated code */
void  ort_execute_serial(void *(*func)(void *), void *shared);
void  ort_execute_parallel(void *(*func)(void *), void *shared, int num_threads,
                           int iscombined, int procbind_type);

void *ort_get_thrpriv(void **key, int size, void *origvar);
void  ort_sglvar_allocate(void **dataptr, int size, void *initer);
void  ort_fence();

/*
 * From tasks.c
 */

void  ort_new_task(void *(*func)(void *arg), void *arg, int final, int untied);
void  ort_taskwait(int waitall);
int   ort_task_throttling(void);
void *ort_task_immediate_start(int final);
void  ort_task_immediate_end(void *tn);
void  ort_entering_taskgroup(void);
void  ort_leaving_taskgroup(void);

/*
 * From pools.c
 */

void *ort_taskenv_alloc(int size, void *(*task_func)(void *));
void  ort_taskenv_free(void *ptr, void *(*task_func)(void *));

/* Atomic, critical, reduction, copyprivate and barrier */
void ort_atomic_begin();
void ort_atomic_end();
void ort_critical_begin(void **cl);
void ort_critical_end(void **cl);
void ort_reduction_begin(void **cl);
void ort_reduction_end(void **cl);
void ort_broadcast_private(int num, ...);
void ort_copy_private(int num, ...);

/*
 * From barrier.c
 */

int ort_barrier_me(void);

/*
 * From ort.c
 */

int ort_enable_cancel(int type);
int ort_check_cancel(int type);

/*
 * From worksharing.c
 */

/* Structure to optimize guided/dynamic schedules.
 * We compute it once and reuse it in every call to ort_get_xxx_chunk().
 * Such things are declared by the compiler (as void *), are initialized
 * during ort_entering_for() and are utilized in every call to
 * ort_get_xxx_chunk() (static schedules ignore this, though).
 */
typedef struct _ort_gdopt_
{
	volatile int  *data;  /* Denotes the current iter of the loop */
	volatile void *lock;  /* Lock to access *data */
	int           nth;    /* # siblings */
	void          *me;    /* my info node */
} ort_gdopt_t;

/* Workshare-related functions */
int  ort_mysingle(int hasnowait);
void ort_leaving_single();
void ort_entering_sections(int hasnowait, int numberofsections);
void ort_leaving_sections();
int  ort_get_section();
void ort_entering_for(int nowait, int hasordered, ort_gdopt_t *t);
int  ort_leaving_for();
void ort_ordered_begin();
void ort_ordered_end();
void ort_thischunk_range(int lb, int ub);

/* For schedules support */
typedef int (*chunky_t)(int niters, int chunksize, int *fiter, int *liter,
                        int *extra, ort_gdopt_t *opt);

int ort_num_iters(int num, long specs[][2], int *itp[]);
void ort_get_runtime_schedule_stuff(chunky_t *func, int *chunksize);
int  ort_get_guided_chunk(int niters, int chunksize, int *fiter, int *liter,
                          int *ignored, ort_gdopt_t *t);
int  ort_get_dynamic_chunk(int niters, int chunksize, int *fiter, int *liter,
                           int *ignored, ort_gdopt_t *t);
int  ort_get_runtimestatic_chunk(int niters, int chunksize,
                                 int *fiter, int *liter, int *chunkid, ort_gdopt_t *t);
int  ort_get_static_default_chunk(int niters, int *from, int *to);

/*
 * From target.c
 */
void *ort_dev_gaddr        (void *local_address);
void *devrt_get_dev_address(void *local_address, unsigned long size);

#pragma omp end declare target
void  ort_offload_kernel   (void *(*host_func)(void *), void *vars, void *declvars, char *kernel_filename_prefix, int device_num, ...);
void *ort_devdata_alloc    (unsigned long size, int device_num);
void  ort_devdata_free     (void *data, int device_num);

void *ort_start_target_data(int tdvars, int device_num);
void  ort_end_target_data  (void *de);
void *ort_alloc_tdvar      (void *var, unsigned long size);
void  ort_init_tdvar       (void *var, unsigned long size);
void  ort_finalize_tdvar   (void *var, unsigned long size);
void  ort_read_tdvar       (void *var, unsigned long size, int device_num);
void  ort_write_tdvar      (void *var, unsigned long size, int device_num);
void *ort_get_vaddress     (void *var);
void  ort_call_decl_reg_func(void (**regfunc)(void));
void  ort_register_declvar (void *var, unsigned long size, const void *init);
void *ort_get_declvar      (void *var, int device_num);
