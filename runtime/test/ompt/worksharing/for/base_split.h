#include "callback.h"
#include <omp.h>

/* With the combined parallel-for construct (base.h), the return-addresses are hard to compare.
   With the separate parallel and for-nowait construct, the addresses become more predictable,
   but the begin of the for-loop still generates additional code, so the offset of loop-begin 
   to the label is >4 Byte.
   TODO: use this file to test codeptr_ra */

int main()
{
  unsigned int i;

  #pragma omp parallel num_threads(4) 
  {
    //__attribute__((aligned(256)))
    print_current_address(0);
    #pragma omp for schedule(SCHEDULE) nowait
    for (i = 0; i < 4; i++) {
      print_current_address(1);
    }
    print_current_address(2);
  }
  print_current_address(3);

  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_begin'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_end'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_implicit_task'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_work'


  // CHECK: 0: NULL_POINTER=[[NULL:.*$]]
  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_parallel_begin: parent_task_id={{[0-9]+}}, parent_task_frame.exit=[[NULL]], parent_task_frame.reenter={{0x[0-f]+}}, parallel_id=[[PARALLEL_ID:[0-9]+]], requested_team_size=4, codeptr_ra=[[PARALLEL_RETURN_ADDRESS:0x[0-f]+]], invoker={{[0-9]+}}
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_loop_begin: parallel_id=[[PARALLEL_ID]], parent_task_id={{[0-9]+}}, codeptr_ra=0x{{[0-f]+}}
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_loop_end: parallel_id=[[PARALLEL_ID]], task_id={{[0-9]+}}, codeptr_ra=[[LOOP_END_RETURN_ADDRESS:0x[0-f]+]]
  // CHECK: {{^}}[[MASTER_ID]]: current_address=[[LOOP_END_RETURN_ADDRESS]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_parallel_end: parallel_id=[[PARALLEL_ID]], task_id={{[0-9]+}}, invoker={{[0-9]+}}, codeptr_ra=[[PARALLEL_RETURN_ADDRESS]]
  // CHECK: {{^}}[[MASTER_ID]]: current_address=[[PARALLEL_RETURN_ADDRESS]]

  // CHECK: {{^}}[[THREAD_ID:[0-9]+]]: ompt_event_loop_begin: parallel_id=[[PARALLEL_ID]], parent_task_id={{[0-9]+}}, codeptr_ra=0x{{[0-f]+}}
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_loop_end: parallel_id=[[PARALLEL_ID]], task_id={{[0-9]+}}, codeptr_ra=[[LOOP_END_RETURN_ADDRESS:0x[0-f]+]]
  // CHECK: {{^}}[[THREAD_ID]]: current_address=[[LOOP_END_RETURN_ADDRESS]]

  // CHECK: {{^}}[[THREAD_ID:[0-9]+]]: ompt_event_loop_begin: parallel_id=[[PARALLEL_ID]], parent_task_id={{[0-9]+}}, codeptr_ra=0x{{[0-f]+}}
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_loop_end: parallel_id=[[PARALLEL_ID]], task_id={{[0-9]+}}, codeptr_ra=[[LOOP_END_RETURN_ADDRESS:0x[0-f]+]]
  // CHECK: {{^}}[[THREAD_ID]]: current_address=[[LOOP_END_RETURN_ADDRESS]]

  // CHECK: {{^}}[[THREAD_ID:[0-9]+]]: ompt_event_loop_begin: parallel_id=[[PARALLEL_ID]], parent_task_id={{[0-9]+}}, codeptr_ra=0x{{[0-f]+}}
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_loop_end: parallel_id=[[PARALLEL_ID]], task_id={{[0-9]+}}, codeptr_ra=[[LOOP_END_RETURN_ADDRESS:0x[0-f]+]]
  // CHECK: {{^}}[[THREAD_ID]]: current_address=[[LOOP_END_RETURN_ADDRESS]]

  return 0;
}