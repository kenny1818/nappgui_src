/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nrc.c
 *
 */

/* NAppGUI resource compiler - nrc */

#include "nglob.h"
#include "nrclib.h"
#include <core/arrpt.h>
#include <core/core.h>
#include <core/strings.h>
#include <osbs/log.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>
#include <sewer/sewer.h>

static bool_t i_WITH_ASSERTS = FALSE;

/*---------------------------------------------------------------------------*/

static int i_error_in_use(void)
{
    bstd_printf("usage: nrc -v\n");
    bstd_printf("       nrc -dc input_resource_dir output_c_file\n");
    bstd_printf("       nrc -dp input_resource_dir output_c_file\n");
    return ERROR_COMMAND_LINE;
}

/*---------------------------------------------------------------------------*/

static int i_version(void)
{
    String *copyright = nglob_copyright();
    bstd_printf("nrc (NAppGUI Resource Compiler) %s\n", sewer_nappgui_version(TRUE));
    bstd_writef(tc(copyright));
    bstd_writef("\n");
    str_destroy(&copyright);
    return SUCCESS;
}

/*---------------------------------------------------------------------------*/

static int i_result(ArrPt(String) * *warnings, ArrPt(String) * *errors, const bool_t regenerated)
{
    int ret = regenerated ? SUCCESS : SUCCESS_UPTODATE;
    cassert_no_null(warnings);
    cassert_no_null(errors);

    arrpt_foreach(error, *errors, String)
        log_printf("[ERROR] %s", tc(error));
    arrpt_end();

    arrpt_foreach(warning, *warnings, String)
        log_printf("[WARNING] %s", tc(warning));
    arrpt_end();

    if (arrpt_size(*errors, String) > 0)
        ret = WITH_ERRORS;
    else if (arrpt_size(*warnings, String) > 0)
        ret = WITH_WARNINGS;

    arrpt_destroy(errors, str_destroy, String);
    arrpt_destroy(warnings, str_destroy, String);
    return ret;
}

/*---------------------------------------------------------------------------*/

static int i_resdir_to_c_file(const char_t *src, const char_t *dest)
{
    ArrPt(String) *warnings = NULL;
    ArrPt(String) *errors = NULL;
    bool_t regenerated = TRUE;
    nrclib_serial_dir(src, dest, &warnings, &errors, &regenerated);
    return i_result(&warnings, &errors, regenerated);
}

/*---------------------------------------------------------------------------*/

static int i_resdir_to_packed_file(const char_t *src, const char_t *dest)
{
    ArrPt(String) *warnings = NULL;
    ArrPt(String) *errors = NULL;
    bool_t regenerated = TRUE;
    nrclib_pack_dir(src, dest, &warnings, &errors, &regenerated);
    return i_result(&warnings, &errors, regenerated);
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int res = 0;
    core_start();

    if (argc < 2)
    {
        res = i_error_in_use();
    }
    else if (str_equ_c(argv[1], "-v") == TRUE)
    {
        if (argc == 2)
            res = i_version();
        else
            res = i_error_in_use();
    }
    else if (str_equ_c(argv[1], "-dc") == TRUE)
    {
        if (argc == 4)
            res = i_resdir_to_c_file((const char_t *)argv[2], (const char_t *)argv[3]);
        else
            res = i_error_in_use();
    }
    else if (str_equ_c(argv[1], "-dp") == TRUE)
    {
        if (argc == 4)
            res = i_resdir_to_packed_file((const char_t *)argv[2], (const char_t *)argv[3]);
        else
            res = i_error_in_use();
    }
    else
    {
        res = i_error_in_use();
    }

    core_finish();

    if (i_WITH_ASSERTS == TRUE)
        return WITH_ASSERTS;
    else
        return res;
}
