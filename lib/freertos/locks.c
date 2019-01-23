#include <sys/lock.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro.h"
#include "task.h"

typedef long _lock_t;
static void lock_init_generic(_lock_t *lock, uint8_t mutex_type)
{
    portENTER_CRITICAL();
    if (*lock)
    {
    }
    else
    {
        xSemaphoreHandle new_sem = xQueueCreateMutex(mutex_type);
        if (!new_sem)
        {
            abort();
        }
        *lock = (_lock_t)new_sem;
    }
    portEXIT_CRITICAL();
}

void _lock_init(_lock_t *lock)
{
    *lock = 0;
    lock_init_generic(lock, queueQUEUE_TYPE_MUTEX);
}

void _lock_init_recursive(_lock_t *lock)
{
    *lock = 0;
    lock_init_generic(lock, queueQUEUE_TYPE_RECURSIVE_MUTEX);
}

void _lock_close(_lock_t *lock)
{
    portENTER_CRITICAL();
    if (*lock)
    {
        xSemaphoreHandle h = (xSemaphoreHandle)(*lock);
#if (INCLUDE_xSemaphoreGetMutexHolder == 1)
        configASSERT(xSemaphoreGetMutexHolder(h) == NULL);
#endif
        vSemaphoreDelete(h);
        *lock = 0;
    }
    portEXIT_CRITICAL();
}

void _lock_close_recursive(_lock_t *lock) __attribute__((alias("_lock_close")));

static int lock_acquire_generic(_lock_t *lock, uint32_t delay, uint8_t mutex_type)
{
    xSemaphoreHandle h = (xSemaphoreHandle)(*lock);
    if (!h)
    {
        lock_init_generic(lock, mutex_type);
        h = (xSemaphoreHandle)(*lock);
        configASSERT(h != NULL);
    }

    BaseType_t success;
    if (mutex_type == queueQUEUE_TYPE_RECURSIVE_MUTEX)
    {
        success = xSemaphoreTakeRecursive(h, delay);
    }
    else
    {
        success = xSemaphoreTake(h, delay);
    }
    return (success == pdTRUE) ? 0 : -1;
}

void _lock_acquire(_lock_t *lock)
{
    lock_acquire_generic(lock, portMAX_DELAY, queueQUEUE_TYPE_MUTEX);
}

void _lock_acquire_recursive(_lock_t *lock)
{
    lock_acquire_generic(lock, portMAX_DELAY, queueQUEUE_TYPE_RECURSIVE_MUTEX);
}

int _lock_try_acquire(_lock_t *lock)
{
    return lock_acquire_generic(lock, 0, queueQUEUE_TYPE_MUTEX);
}

int _lock_try_acquire_recursive(_lock_t *lock)
{
    return lock_acquire_generic(lock, 0, queueQUEUE_TYPE_RECURSIVE_MUTEX);
}

static void lock_release_generic(_lock_t *lock, uint8_t mutex_type)
{
    xSemaphoreHandle h = (xSemaphoreHandle)(*lock);
    if (h == NULL)
    {
        return;
    }

    if (mutex_type == queueQUEUE_TYPE_RECURSIVE_MUTEX)
    {
        xSemaphoreGiveRecursive(h);
    }
    else
    {
        xSemaphoreGive(h);
    }
    
}

void _lock_release(_lock_t *lock)
{
    lock_release_generic(lock, queueQUEUE_TYPE_MUTEX);
}

void _lock_release_recursive(_lock_t *lock)
{
    lock_release_generic(lock, queueQUEUE_TYPE_RECURSIVE_MUTEX);
}
