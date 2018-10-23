/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */
/*
 * ssc_fp_cap_debug.c  
 * Original Author: suyuhuan@ruijie.com.cn, 2018-9-5
 *
 * ssc端fp_cap_debug实现文件
 *
 * History
 *  v1.0    suyuhuan@ruijie.com.cn    2018-9-5
 *          create
 */

#include <stdbool.h>
#include "ssc_fp_cap_debug.h"

bool g_fp_cap_prt_dbg;
bool g_fp_cap_prt_err;
bool g_fp_cap_at_init;
bool g_fp_cap_prt_dbg_enable = false;

static bool fp_cap_debug_file_enable(char *file)
{
    FILE *fp = NULL;

    if (file == NULL) {
        
        return FALSE;
    }

    fp = fopen(file, "r");
    if (fp == NULL) {
        
        return FALSE;
    }
    fclose(fp);

    return TRUE;
}

static void fp_cap_debug_read_conf(void)
{
    /* shell打开
     * mkdir -p /data/fp_cap/
     * touch /data/fp_cap/fp_cap_dbg
     * sync
     */
    if (fp_cap_debug_file_enable("/data/cap/fp_cap_dbg")) {
        g_fp_cap_prt_dbg = true;
        g_fp_cap_prt_err = true;
        g_fp_cap_prt_dbg_enable = true;
    } else {
        g_fp_cap_prt_dbg = false;
        g_fp_cap_prt_err = false;
        g_fp_cap_prt_dbg_enable = false;
    }

    return;
}

static void fp_cap_at_dump_parse_all(void)
{
    ss_fp_cap_dump_parse_all();
    
    return;
}

static void fp_cap_at_dump_parse_single(int device, int slot)
{
    ss_fp_cap_dump_parse_single(device, slot);
    
    return;
}

static void fp_cap_at_get_string_by_mode(int mode)
{
    char *string;
    
    string = ss_fp_cap_get_string_by_mode(mode);
    if (string == NULL) {
        AT_PRINT("Cannot get string from mode %d\n", mode);
        
        return;
    }
    
    AT_PRINT("Mode %d, string %s\n", mode, string);
    
    return;
}

static void fp_cap_at_get_mode_by_string(char *string)
{
    int mode;
    
    mode = ss_fp_cap_get_mode_by_string(string);
    if (mode < CAP_MODE_DEFAULT) {
        AT_PRINT("Cannot get mode from string %s\n", string);
        
        return;
    }
    
    AT_PRINT("String %s, mode %d\n", string, mode);
    
    return;
}

static void fp_cap_at_get_mode_and_string_by_dm_node(int dm_node)
{
    int mode;
    char *string;
    
    mode = ss_fp_cap_get_mode_by_dm_node(dm_node);
    if (mode < CAP_MODE_DEFAULT) {
        AT_PRINT("Cannot get mode from dm_node %u\n", dm_node);
        
        return;
    }
    AT_PRINT("Dm_node %u, mode %d\n", dm_node, mode);
    
    string = ss_fp_cap_get_string_by_dm_node(dm_node);
    if (string == NULL) {
        AT_PRINT("Cannot get string from dm_node %u\n", dm_node);
        
        return;
    }
    AT_PRINT("Dm_node %u, string %s\n", dm_node, string);
    
    return;
}

static void fp_cap_at_set_print_level(int debug, int error, int dbg_prt)
{
    g_fp_cap_prt_dbg = debug != 0 ? true : false;
    g_fp_cap_prt_err = error != 0 ? true : false;
    g_fp_cap_prt_dbg_enable = dbg_prt != 0 ? true : false;
    
    AT_PRINT("Print level: debug - %s, error - %s,dbg_prt=%s\n", debug != 0 ? "TRUE" : "FALSE", 
        error != 0 ? "TRUE" : "FALSE", dbg_prt != 0 ? "TRUE" : "FALSE");
    
    return;
}

static void fp_cap_at_get_print_level(void)
{
    AT_PRINT("g_fp_cap_prt_dbg=%s, g_cap_prt_err=%s,g_cap_prt_dbg_enable=%s\r\n",
        g_fp_cap_prt_dbg ? "true" : "false",
        g_fp_cap_prt_err ? "true" : "false",
        g_fp_cap_prt_dbg_enable ? "true" : "false");

    return;
}

static void fp_cap_at_parse_config_file(char *file)
{
    ss_fp_cap_parse_config(file);

    return;
}

/**
 * ssc_fp_cap_debug_init ssc端fp_cap调试初始化
 * 
 * @return:成功返回SS_E_NONE 其它表示失败见ss_errno.h
 */
int ssc_fp_cap_debug_init(void)
{
    fp_cap_debug_read_conf();
    AT_REG_CMD("show-parse-all", "show parsed info", fp_cap_at_dump_parse_all, "$n");
    AT_REG_CMD("show-parse-single", "show parsed info by devid and slot id",
        fp_cap_at_dump_parse_single, "devid=%d, slotid=%d, $n");
    AT_REG_CMD("get-string", "get fp_cap string by mode", fp_cap_at_get_string_by_mode,
        "mode=%d, $n");
    AT_REG_CMD("get-mode", "get fp_cap mode by string", fp_cap_at_get_mode_by_string, 
        "string=%s, $n");
    AT_REG_CMD("get-mode-string", "get fp_cap mode and string from dm_node", 
        fp_cap_at_get_mode_and_string_by_dm_node, "dm_node=%d, $n");
    AT_REG_CMD("set-print-level", "set print level", fp_cap_at_set_print_level, 
        "debug=%d, error=%d, dbg_prt=%d, $n");
    AT_REG_CMD("get-print-level", "get print level", fp_cap_at_get_print_level,"$n");
    AT_REG_CMD("parse", "parse file", fp_cap_at_parse_config_file, "file=%s, $n");
    
    g_fp_cap_at_init = true;
    
    return SS_E_NONE;
}

