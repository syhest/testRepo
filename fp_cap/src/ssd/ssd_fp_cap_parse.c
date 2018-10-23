/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */
/*
 * ssd_fp_cap_parse.c  
 * Original Author: suyuhuan@ruijie.com.cn, 2018-9-15
 *
 * ssd端fp_cap 文件解析实现文件
 *
 * History
 *  v1.0    suyuhuan@ruijie.com.cn    2018-9-15
 *          create
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <rg_ss/public/policy/ss_cap_defs.h>
#include "ssd_fp_cap_debug.h"

#define FP_CAP_CONFIG_FILE         "/data/fp_cap_config/config_path.bcm"
#define FP_CAP_LINE_MAX_LEN        (128U)
#define FP_CAP_INVALID_VALUE       (-1)

static int g_fp_cap_mode;

/**
 * ss_fp_cap_parse_config ssd端fp_cap 配置文件解析
 *
 * @cfg_file: 指向文件路径字符串指针
 *
 * @return: 成功返回SS_E_NONE 其它表示失败见ss_errno.h
 */
int ss_fp_cap_parse_config(char *cfg_file)
{
    char buf[FP_CAP_LINE_MAX_LEN];
    FILE *fp;
    int len, mode;
    char *pch;

    fp = fopen(cfg_file, "r");
    if (fp == NULL) {
        
        return -SS_E_FAIL;
    }

    while (fgets(buf, FP_CAP_LINE_MAX_LEN, fp) != NULL) {
        len = strlen(buf);
        buf[len - 1] = '\0';/* buf[len - 1] 字符串结束符 */
        CAP_PRT_DBG("get mode %s\n", buf);
        
        /* buf is "xxx.bcm", mode string is "xxx" */
        pch = strtok(buf, ".");
        if (pch == NULL) {
            
            return SS_E_NOT_FOUND;
        }
        mode = ss_fp_cap_get_mode_by_string(pch);
        if (FP_CAP_MODE_IS_INVALID(mode)) {
            mode = CAP_MODE_DEFAULT;
        }
        g_fp_cap_mode = mode;
        /* The config file has only one line */
        break;
    }
    fclose(fp);

    return SS_E_NONE;
}

/**
 * ss_fp_cap_get_mode_string ssd端获取当前模式的字符串
 *
 * @return:当前模式的字符串
 */
char *ss_fp_cap_get_mode_string(void)
{
    return ss_fp_cap_get_string_by_mode(g_fp_cap_mode);
}

/**
 * ssd_fp_cap_parse_init ssd端fp_cap配置解析初始化
 *
 * @return: 成功返回SS_E_NONE 其它表示失败见ss_errno.h
 */
int ssd_fp_cap_parse_init(void)
{
    int rv;

    g_fp_cap_mode = CAP_MODE_DEFAULT;

    rv = ss_fp_cap_parse_config(FP_CAP_CONFIG_FILE);
    if (rv != SS_E_NONE) {
        CAP_PRT_DBG("file %s not exists or no enough rights", FP_CAP_CONFIG_FILE);

        return rv;
    }

    return SS_E_NONE;
}

