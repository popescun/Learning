/*
 * FreeRTOSTasks.h
 *
 * Contains definitions of FreeRTOS tasks, prioritizing their stack sizes and
 * execution priorities. This file centralizes task configuration for ease of
 * management and clarity in the overall system design.
 */

#ifndef FREERTOSTASKS_H_
#define FREERTOSTASKS_H_

#include "FreeRTOS.h"
#include "task.h"

/**
 *
 */

#define STARTUP_TASK_STACK_SIZE            (512)
#define STARTUP_TASK_PRIORITY              (configMAX_PRIORITIES - 1)



#endif /* FREERTOSTASKS_H_ */
