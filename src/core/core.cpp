/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: core.cpp
 *
 */

/* Core library */

#include "osbs.h"
#include "core.h"
#include "core.inl"
#include "heap.inl"
#include "dbind.inl"
#include "stream.inl"
#include "bmem.h"
#include "bproc.h"
#include "bstd.h"
#include "cassert.h"
#include "heap.h"
#include "log.h"

/*---------------------------------------------------------------------------*/

class Core
{
public:
    static uint32_t NUM_USERS;

    //Core()
    //{
    //    log_printf("Starting Core");        
    //}
    
    ~Core()
    {
        if (NUM_USERS != 0)
        {
            core_finish();
            //log_printf("Error! core is not properly closed (%d)\n", NUM_USERS);
        }
    }
};

static Core i_CORE;
uint32_t Core::NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

static void i_assert_to_log(void *item, const uint32_t group, const char_t *caption, const char_t *detail, const char_t *file, const uint32_t line)
{
    unref(item);
    unref(group);
    if (group == 0)
    {
        log_printf("FATAL ASSERT: %s (%s:%d): %s", caption, file, line, detail);
        bproc_exit(1000);
    }
    else
    {
        log_printf("ASSERT: %s (%s:%d): %s", caption, file, line, detail);
    }
}

/*---------------------------------------------------------------------------*/

void core_start(void)
{
    if (i_CORE.NUM_USERS == 0)
    {
        osbs_start();
        _heap_start();
        _stm_start();
        _dbind_start();
        cassert_set_func(NULL, i_assert_to_log);
        i_CORE.NUM_USERS = 1;
        #if defined (__APPLE__) || defined (__LINUX__)
        cassert(sizeof(EventHandler) == 2 * sizeof(void*));
        #else
        cassert(sizeof(EventHandler) == sizeof(void*));
        #endif
    }
    else
    {
        i_CORE.NUM_USERS += 1;
    }
}

/*---------------------------------------------------------------------------*/

void core_finish(void)
{
    cassert(i_CORE.NUM_USERS > 0);
    if (i_CORE.NUM_USERS == 1)
    {
        i_CORE.NUM_USERS = 0;
        _dbind_finish();
        _stm_finish();
        _heap_finish();
        osbs_finish();
    }
    else
    {
        i_CORE.NUM_USERS -= 1;
    }
}
