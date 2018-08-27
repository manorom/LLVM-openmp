#include "kmp.h"
#include <stdint.h>

#ifndef __OMPD_SPECIFIC_H__
#define __OMPD_SPECIFIC_H__

#ifdef OMPD_SUPPORT

void ompd_init();
extern volatile const char * * ompd_dll_locations;
extern int ompd_rtl_version;

#ifdef  __cplusplus
extern "C" {
#endif
void __attribute__ ((noinline)) ompd_dll_locations_valid ( void );
void __attribute__ ((noinline)) ompd_bp_parallel_begin ( void );
void __attribute__ ((noinline)) ompd_bp_parallel_end ( void );
void __attribute__ ((noinline)) ompd_bp_task_begin ( void );
void __attribute__ ((noinline)) ompd_bp_task_end ( void );
void __attribute__ ((noinline)) ompd_bp_thread_begin ( void );
void __attribute__ ((noinline)) ompd_bp_thread_end ( void );
#ifdef  __cplusplus
} /* extern "C" */
#endif

extern uint64_t ompd_state;
#define OMPD_ENABLE_BP 0x1

#define OMPD_FOREACH_ACCESS(OMPD_ACCESS) \
OMPD_ACCESS(kmp_base_info_t,      th_current_task) \
OMPD_ACCESS(kmp_base_info_t,      th_team) \
OMPD_ACCESS(kmp_base_info_t,      th_info) \
OMPD_ACCESS(kmp_base_info_t,      ompt_thread_info) \
\
OMPD_ACCESS(kmp_base_root_t,      r_in_parallel) \
\
OMPD_ACCESS(kmp_base_team_t,      ompt_team_info) \
OMPD_ACCESS(kmp_base_team_t,      ompt_serialized_team_info) \
OMPD_ACCESS(kmp_base_team_t,      t_active_level) \
OMPD_ACCESS(kmp_base_team_t,      t_implicit_task_taskdata) \
OMPD_ACCESS(kmp_base_team_t,      t_master_tid) \
OMPD_ACCESS(kmp_base_team_t,      t_nproc) \
OMPD_ACCESS(kmp_base_team_t,      t_level) \
OMPD_ACCESS(kmp_base_team_t,      t_parent) \
OMPD_ACCESS(kmp_base_team_t,      t_pkfn) \
OMPD_ACCESS(kmp_base_team_t,      t_threads) \
\
OMPD_ACCESS(kmp_desc_t,           ds) \
\
OMPD_ACCESS(kmp_desc_base_t,      ds_thread) \
OMPD_ACCESS(kmp_desc_base_t,      ds_tid) \
\
OMPD_ACCESS(kmp_info_t,           th) \
\
OMPD_ACCESS(kmp_r_sched_t,        r_sched_type) \
OMPD_ACCESS(kmp_r_sched_t,        chunk) \
\
OMPD_ACCESS(kmp_root_t,           r) \
\
OMPD_ACCESS(kmp_internal_control_t, dynamic) \
OMPD_ACCESS(kmp_internal_control_t, max_active_levels) \
OMPD_ACCESS(kmp_internal_control_t, nested) \
OMPD_ACCESS(kmp_internal_control_t, nproc) \
OMPD_ACCESS(kmp_internal_control_t, proc_bind) \
OMPD_ACCESS(kmp_internal_control_t, sched) \
\
OMPD_ACCESS(kmp_taskdata_t,       ompt_task_info) \
OMPD_ACCESS(kmp_taskdata_t,       td_flags) \
OMPD_ACCESS(kmp_taskdata_t,       td_icvs) \
OMPD_ACCESS(kmp_taskdata_t,       td_parent) \
OMPD_ACCESS(kmp_taskdata_t,       td_team) \
\
OMPD_ACCESS(kmp_task_t,           routine) \
\
OMPD_ACCESS(kmp_team_p,           t) \
\
OMPD_ACCESS(ompt_task_info_t,     frame) \
OMPD_ACCESS(ompt_task_info_t,     scheduling_parent) \
OMPD_ACCESS(ompt_task_info_t,     task_data) \
/*OMPD_ACCESS(ompt_task_info_t,     function)*/ \
\
OMPD_ACCESS(ompt_team_info_t,     parallel_data) \
/*OMPD_ACCESS(ompt_team_info_t,     microtask)*/ \
\
OMPD_ACCESS(ompt_thread_info_t,   state) \
OMPD_ACCESS(ompt_thread_info_t,   wait_id) \
\
OMPD_ACCESS(ompt_data_t,   value) \
OMPD_ACCESS(ompt_data_t,   ptr) \
\
OMPD_ACCESS(omp_frame_t,         exit_frame) \
OMPD_ACCESS(omp_frame_t,         enter_frame) \
\
OMPD_ACCESS(ompt_lw_taskteam_t,   parent) \
OMPD_ACCESS(ompt_lw_taskteam_t,   ompt_team_info) \
OMPD_ACCESS(ompt_lw_taskteam_t,   ompt_task_info)


#define OMPD_FOREACH_BITFIELD(OMPD_BITFIELD) \
OMPD_BITFIELD(kmp_tasking_flags_t,  final) \
OMPD_BITFIELD(kmp_tasking_flags_t,  tiedness) \
OMPD_BITFIELD(kmp_tasking_flags_t,  tasktype) \
OMPD_BITFIELD(kmp_tasking_flags_t,  task_serial) \
OMPD_BITFIELD(kmp_tasking_flags_t,  tasking_ser) \
OMPD_BITFIELD(kmp_tasking_flags_t,  team_serial) \
OMPD_BITFIELD(kmp_tasking_flags_t,  started) \
OMPD_BITFIELD(kmp_tasking_flags_t,  executing) \
OMPD_BITFIELD(kmp_tasking_flags_t,  complete) \
OMPD_BITFIELD(kmp_tasking_flags_t,  freed) \
OMPD_BITFIELD(kmp_tasking_flags_t,  native) \


// TODO: (mr) this is a hack to cast cuda contexts to 64 bit values
typedef uint64_t ompd_cuda_context_ptr_t;

#define OMPD_FOREACH_SIZEOF(OMPD_SIZEOF) \
OMPD_SIZEOF(kmp_info_t) \
OMPD_SIZEOF(kmp_taskdata_t) \
OMPD_SIZEOF(kmp_task_t) \
OMPD_SIZEOF(kmp_tasking_flags_t) \
OMPD_SIZEOF(kmp_thread_t) \
OMPD_SIZEOF(ompt_data_t) \
OMPD_SIZEOF(ompt_id_t) \
OMPD_SIZEOF(__kmp_avail_proc) \
OMPD_SIZEOF(__kmp_max_nth) \
OMPD_SIZEOF(__kmp_gtid) \
OMPD_SIZEOF(__kmp_nth) \
OMPD_SIZEOF(ompd_cuda_context_ptr_t) \

#endif /* OMPD_SUPPORT */
#endif
