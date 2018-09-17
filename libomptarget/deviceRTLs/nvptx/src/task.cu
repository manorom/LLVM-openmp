//===------------- task.h - NVPTX OpenMP tasks support ----------- CUDA -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Task implementation support.
//
//  explicit task structure uses
//  omptarget_nvptx task
//  kmp_task
//
//  where kmp_task is
//    - klegacy_TaskDescr    <- task pointer
//        shared -> X
//        routine
//        part_id
//        descr
//    -  private (of size given by task_alloc call). Accessed by
//       task+sizeof(klegacy_TaskDescr)
//        * private data *
//    - shared: X. Accessed by shared ptr in klegacy_TaskDescr
//        * pointer table to shared variables *
//    - end
//
//===----------------------------------------------------------------------===//

#include "omptarget-nvptx.h"

EXTERN kmp_TaskDescr *__kmpc_omp_task_alloc(
    kmp_Indent *loc,     // unused
    uint32_t global_tid, // unused
    int32_t flag, // unused (because in our impl, all are immediately exec
    size_t sizeOfTaskInclPrivate, size_t sizeOfSharedTable,
    kmp_TaskFctPtr taskSub) {
  PRINT(LD_IO,
        "call __kmpc_omp_task_alloc(size priv&struct %lld, shared %lld, "
        "fct 0x%llx)\n",
        P64(sizeOfTaskInclPrivate), P64(sizeOfSharedTable), P64(taskSub));
  // want task+priv to be a multiple of 8 bytes
  size_t padForTaskInclPriv = PadBytes(sizeOfTaskInclPrivate, sizeof(void *));
  sizeOfTaskInclPrivate += padForTaskInclPriv;
  size_t kmpSize = sizeOfTaskInclPrivate + sizeOfSharedTable;
  ASSERT(LT_FUSSY, sizeof(omptarget_nvptx_TaskDescr) % sizeof(void *) == 0,
         "need task descr of size %d to be a multiple of %d\n",
         sizeof(omptarget_nvptx_TaskDescr), sizeof(void *));
  size_t totSize = sizeof(omptarget_nvptx_TaskDescr) + kmpSize;
  omptarget_nvptx_ExplicitTaskDescr *newExplicitTaskDescr =
      (omptarget_nvptx_ExplicitTaskDescr *)SafeMalloc(
          totSize, "explicit task descriptor");
  kmp_TaskDescr *newKmpTaskDescr = &newExplicitTaskDescr->kmpTaskDescr;
  ASSERT0(LT_FUSSY,
          (uint64_t)newKmpTaskDescr ==
              (uint64_t)ADD_BYTES(newExplicitTaskDescr,
                                  sizeof(omptarget_nvptx_TaskDescr)),
          "bad size assumptions");
  // init kmp_TaskDescr
  newKmpTaskDescr->sharedPointerTable =
      (void *)((char *)newKmpTaskDescr + sizeOfTaskInclPrivate);
  newKmpTaskDescr->sub = taskSub;
  newKmpTaskDescr->destructors = NULL;
  PRINT(LD_TASK, "return with task descr kmp: 0x%llx, omptarget-nvptx 0x%llx\n",
        P64(newKmpTaskDescr), P64(newExplicitTaskDescr));

  return newKmpTaskDescr;
}

EXTERN int32_t __kmpc_omp_task(kmp_Indent *loc, uint32_t global_tid,
                               kmp_TaskDescr *newKmpTaskDescr) {
  return __kmpc_omp_task_with_deps(loc, global_tid, newKmpTaskDescr, 0, 0, 0,
                                   0);
}

EXTERN int32_t __kmpc_omp_task_with_deps(kmp_Indent *loc, uint32_t global_tid,
                                         kmp_TaskDescr *newKmpTaskDescr,
                                         int32_t depNum, void *depList,
                                         int32_t noAliasDepNum,
                                         void *noAliasDepList) {
  PRINT(LD_IO, "call to __kmpc_omp_task_with_deps(task 0x%llx)\n",
        P64(newKmpTaskDescr));
  // 1. get explict task descr from kmp task descr
  omptarget_nvptx_ExplicitTaskDescr *newExplicitTaskDescr =
      (omptarget_nvptx_ExplicitTaskDescr *)SUB_BYTES(
          newKmpTaskDescr, sizeof(omptarget_nvptx_TaskDescr));
  ASSERT0(LT_FUSSY, &newExplicitTaskDescr->kmpTaskDescr == newKmpTaskDescr,
          "bad assumptions");
  omptarget_nvptx_TaskDescr *newTaskDescr = &newExplicitTaskDescr->taskDescr;
  ASSERT0(LT_FUSSY, (uint64_t)newTaskDescr == (uint64_t)newExplicitTaskDescr,
          "bad assumptions");

  // 2. push new context: update new task descriptor
  int tid = GetLogicalThreadIdInBlock();
  omptarget_nvptx_TaskDescr *parentTaskDescr = getMyTopTaskDescriptor(tid);
  newTaskDescr->CopyForExplicitTask(parentTaskDescr);
  // set new task descriptor as top
  omptarget_nvptx_threadPrivateContext->SetTopLevelTaskDescr(tid, newTaskDescr);
#ifdef OMPD_SUPPORT
  ompd_init_explicit_task((void*)(newKmpTaskDescr->sub));
#endif
  // 3. call sub
  PRINT(LD_TASK, "call task sub 0x%llx(task descr 0x%llx)\n",
        P64(newKmpTaskDescr->sub), P64(newKmpTaskDescr));
  newKmpTaskDescr->sub(0, newKmpTaskDescr);
  PRINT(LD_TASK, "return from call task sub 0x%llx()\n",
        P64(newKmpTaskDescr->sub));

  // 4. pop context
  omptarget_nvptx_threadPrivateContext->SetTopLevelTaskDescr(tid,
                                                             parentTaskDescr);
  // 5. free
  SafeFree(newExplicitTaskDescr, "explicit task descriptor");
  return 0;
}

EXTERN void __kmpc_omp_task_begin_if0(kmp_Indent *loc, uint32_t global_tid,
                                      kmp_TaskDescr *newKmpTaskDescr) {
  PRINT(LD_IO, "call to __kmpc_omp_task_begin_if0(task 0x%llx)\n",
        P64(newKmpTaskDescr));
  // 1. get explict task descr from kmp task descr
  omptarget_nvptx_ExplicitTaskDescr *newExplicitTaskDescr =
      (omptarget_nvptx_ExplicitTaskDescr *)SUB_BYTES(
          newKmpTaskDescr, sizeof(omptarget_nvptx_TaskDescr));
  ASSERT0(LT_FUSSY, &newExplicitTaskDescr->kmpTaskDescr == newKmpTaskDescr,
          "bad assumptions");
  omptarget_nvptx_TaskDescr *newTaskDescr = &newExplicitTaskDescr->taskDescr;
  ASSERT0(LT_FUSSY, (uint64_t)newTaskDescr == (uint64_t)newExplicitTaskDescr,
          "bad assumptions");

  // 2. push new context: update new task descriptor
  int tid = GetLogicalThreadIdInBlock();
  omptarget_nvptx_TaskDescr *parentTaskDescr = getMyTopTaskDescriptor(tid);
  newTaskDescr->CopyForExplicitTask(parentTaskDescr);
  // set new task descriptor as top
  omptarget_nvptx_threadPrivateContext->SetTopLevelTaskDescr(tid, newTaskDescr);
  // 3... noting to call... is inline
  // 4 & 5 ... done in complete
}

EXTERN void __kmpc_omp_task_complete_if0(kmp_Indent *loc, uint32_t global_tid,
                                         kmp_TaskDescr *newKmpTaskDescr) {
  PRINT(LD_IO, "call to __kmpc_omp_task_complete_if0(task 0x%llx)\n",
        P64(newKmpTaskDescr));
  // 1. get explict task descr from kmp task descr
  omptarget_nvptx_ExplicitTaskDescr *newExplicitTaskDescr =
      (omptarget_nvptx_ExplicitTaskDescr *)SUB_BYTES(
          newKmpTaskDescr, sizeof(omptarget_nvptx_TaskDescr));
  ASSERT0(LT_FUSSY, &newExplicitTaskDescr->kmpTaskDescr == newKmpTaskDescr,
          "bad assumptions");
  omptarget_nvptx_TaskDescr *newTaskDescr = &newExplicitTaskDescr->taskDescr;
  ASSERT0(LT_FUSSY, (uint64_t)newTaskDescr == (uint64_t)newExplicitTaskDescr,
          "bad assumptions");
  // 2. get parent
  omptarget_nvptx_TaskDescr *parentTaskDescr = newTaskDescr->GetPrevTaskDescr();
  // 3... noting to call... is inline
  // 4. pop context
  int tid = GetLogicalThreadIdInBlock();
  omptarget_nvptx_threadPrivateContext->SetTopLevelTaskDescr(tid,
                                                             parentTaskDescr);
  // 5. free
  SafeFree(newExplicitTaskDescr, "explicit task descriptor");
}

EXTERN void __kmpc_omp_wait_deps(kmp_Indent *loc, uint32_t global_tid,
                                 int32_t depNum, void *depList,
                                 int32_t noAliasDepNum, void *noAliasDepList) {
  PRINT0(LD_IO, "call to __kmpc_omp_wait_deps(..)\n");
  // nothing to do as all our tasks are executed as final
}

EXTERN void __kmpc_taskgroup(kmp_Indent *loc, uint32_t global_tid) {
  PRINT0(LD_IO, "call to __kmpc_taskgroup(..)\n");
  // nothing to do as all our tasks are executed as final
}

EXTERN void __kmpc_end_taskgroup(kmp_Indent *loc, uint32_t global_tid) {
  PRINT0(LD_IO, "call to __kmpc_end_taskgroup(..)\n");
  // nothing to do as all our tasks are executed as final
}

EXTERN int32_t __kmpc_omp_taskyield(kmp_Indent *loc, uint32_t global_tid,
                                    int end_part) {
  PRINT0(LD_IO, "call to __kmpc_taskyield()\n");
  // do nothing: tasks are executed immediately, no yielding allowed
  return 0;
}

EXTERN int32_t __kmpc_omp_taskwait(kmp_Indent *loc, uint32_t global_tid) {
  PRINT0(LD_IO, "call to __kmpc_taskwait()\n");
  // nothing to do as all our tasks are executed as final
  return 0;
}

EXTERN void __kmpc_taskloop(kmp_Indent *loc, uint32_t global_tid,
                            kmp_TaskDescr *newKmpTaskDescr, int if_val,
                            uint64_t *lb, uint64_t *ub, int64_t st, int nogroup,
                            int32_t sched, uint64_t grainsize, void *task_dup) {

  // skip task entirely if empty iteration space
  if (*lb > *ub)
    return;

  // the compiler has already stored lb and ub in the kmp_TaskDescr structure
  // as we are using a single task to execute the entire loop, we can leave
  // the initial task_t untouched

  __kmpc_omp_task_with_deps(loc, global_tid, newKmpTaskDescr, 0, 0, 0, 0);
}
