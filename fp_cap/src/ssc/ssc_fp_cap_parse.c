/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */
/*
 * ssc_fp_cap_parse.c  
 * Original Author: suyuhuan@ruijie.com.cn, 2018-9-5
 *
 * ssc端fp_cap_parse实现文件
 *
 * History
 *  v1.0    suyuhuan@ruijie.com.cn    2018-9-5
 *          create
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <rg_dev/dm_lib/rg_dm.h>
#include <rg_ss/lib/libpub/ss_comm_macro.h>
#include <rg_ss/public/policy/ss_cap_defs.h>
#include "ssc_fp_cap_debug.h"

#define FP_CAP_CONFIG_FILE         "/data/config.text"
#define FP_CAP_LINE_MAX_LEN        256
#define FP_CAP_PARSE_MODE          "switch-policy-cap-mode"
#define FP_CAP_PARSE_SLOT          "slot"
#define FP_CAP_PARSE_DEVICE        "switch"
#define FP_CAP_PARSE_OVERLAY       "overlay"

#define FP_CAP_INVALID_VALUE       (-1)

/* device id 有效检查 */
#define FP_CAP_DEVID_INVALID(id)     ((id) < SS_MIN_SWITCH_ID || (id) > SS_MAX_SWITCH_ID)
/* slot id 有效检查 */
#define FP_CAP_SLOTID_INVALID(id)    ((id) < SS_MIN_SLOT_ID || (id) > SS_MAX_SLOT_ID)

static int g_fp_cap_mode[SS_MAX_SWITCH_NUM][SS_MAX_SLOT_NUM];

static void parse_line(char *cfg)
{
    bool last_mode, last_device, last_slot;
    bool got_mode, got_device, got_slot;
    char *pch;
    char sep[] = " ";
    int fp_cap_mode, fp_cap_slot, fp_cap_device;
    bool overlay_en;

    last_mode = last_device = last_slot = false;
    got_mode = got_device = got_slot = false;
    fp_cap_mode = fp_cap_device = fp_cap_slot = FP_CAP_INVALID_VALUE;
    overlay_en = false;
    pch = strtok(cfg, sep);
    while (pch) {
        if (memcmp(pch, FP_CAP_PARSE_MODE, strlen(FP_CAP_PARSE_MODE)) == 0) {
            last_mode = true;
            got_mode = true;
        } else if (memcmp(pch, FP_CAP_PARSE_DEVICE, strlen(FP_CAP_PARSE_DEVICE)) == 0) {
            last_device = true;
            got_device = true;
        } else if (memcmp(pch, FP_CAP_PARSE_SLOT, strlen(FP_CAP_PARSE_SLOT)) == 0) {
            last_slot = true;
            got_slot = true;
        } else if (memcmp(pch, FP_CAP_PARSE_OVERLAY, strlen(FP_CAP_PARSE_OVERLAY)) == 0) {
            overlay_en = true;
        }
        pch = strtok(NULL, sep);
        if (pch == NULL) {
            break; 
        }

        if (last_mode) {
            fp_cap_mode = ss_fp_cap_get_mode_by_string(pch);
            last_mode = false;

            CAP_PRT_DBG("mode %d %s\n", fp_cap_mode, pch);

            if (FP_CAP_MODE_IS_INVALID(fp_cap_mode)) {
                
                return;
            }

        } else if (last_device) {
            fp_cap_device = atoi(pch);
            last_device = false;

            CAP_PRT_DBG("device %d\n", fp_cap_device);
            if (fp_cap_device < 0 || fp_cap_device > SS_MAX_SWITCH_ID) {
                
                return;
            }
        } else if (last_slot) {
            fp_cap_slot = atoi(pch);
            last_slot = false;

            CAP_PRT_DBG("slot %d\n", fp_cap_slot);
            if (fp_cap_slot < 0 || fp_cap_slot > SS_MAX_SLOT_ID) {
                
                return;
            }
        }
    }/* config.text 字段解析 */

    if (got_mode && got_slot) {
        if (!got_device) {
            fp_cap_device = dm_get_my_devid();
        }

        /* 若为overlay，需要加上偏移 */
        if (overlay_en) {
            fp_cap_mode += CAP_MODE_CNT;
        }
        
        CAP_PRT_DBG("device %d, slot %d, mode %d", fp_cap_device, fp_cap_slot, fp_cap_mode);

        if (FP_CAP_SLOTID_INVALID(fp_cap_slot) || FP_CAP_DEVID_INVALID(fp_cap_device)
                || FP_CAP_MODE_IS_INVALID(fp_cap_mode)) {
            CAP_PRT_ERR("GET Wrong mode %d or device %d or slot %d", fp_cap_mode,
                fp_cap_device, fp_cap_slot);
            
            return;
        }

        g_fp_cap_mode[fp_cap_device][fp_cap_slot] = fp_cap_mode;
        CAP_PRT_DBG("device %d, slot %d, mode %s", fp_cap_device, fp_cap_slot, 
            ss_fp_cap_get_string_by_mode(fp_cap_mode));
    }/* mode与slot有效情况处理 */

    return;
}

/**
 * ss_fp_cap_parse_config ssc端fp_cap解析配置文件
 * 
 * @cfg_file:指向配置后文件指针
 * 
 * @return:成功返回SS_E_NONE 其它表示失败见ss_errno.h
 */
int ss_fp_cap_parse_config(char *cfg_file)
{
    char buf[FP_CAP_LINE_MAX_LEN];
    FILE *fp;
    int len;
    int i;

    fp = fopen(cfg_file, "r");
    if (fp == NULL) {
        
        return -SS_E_FAIL;
    }

    i = 0;/* 计数变量初始化清零 */
    while (fgets(buf, FP_CAP_LINE_MAX_LEN, fp) != NULL) {
        i++;
        len = strlen(buf);
        buf[len - 1] = '\0';/* buf[len - 1]字符串结束符 */
        CAP_PRT_DBG("[LINE %d] %s\n", i, buf);

        parse_line(buf);
    }
    fclose(fp);

    return SS_E_NONE;
}

/**
 * ssc_fp_cap_parse_init ssc端fp_cap解析配置文件初始化
 * 
 * @return:成功返回SS_E_NONE 其它表示失败见ss_errno.h
 */
int ssc_fp_cap_parse_init(void)
{
    int i;
    int j;
    int rv;

    for (i = 0; i < SS_MAX_SWITCH_NUM; i++) {
        for (j = 0; j < SS_MAX_SLOT_NUM; j++) {
            g_fp_cap_mode[i][j] = CAP_MODE_DEFAULT;
        }
    }

    rv = ss_fp_cap_parse_config(FP_CAP_CONFIG_FILE);
    if (rv != SS_E_NONE) {
        CAP_PRT_DBG("file %s not exists or no enough rights", FP_CAP_CONFIG_FILE);
    }

    return SS_E_NONE;
}

/**
 * ss_fp_cap_get_mode_by_dm_node ssc端获得fp_cap模式
 * 
 * @dm_node:dm节点
 * 
 * @return:成功返回fp_cap_mode 其它表示失败见ss_errno.h
 */
int ss_fp_cap_get_mode_by_dm_node(int dm_node)
{
    int rv;
    int dev, phy_slot, cpuid, user_slot;

    dev = phy_slot = cpuid = user_slot= FP_CAP_INVALID_VALUE;
    rv = dm_get_dev_slot_cpuid_from_node(dm_node, &dev, &phy_slot, &cpuid);
    if (rv != SS_E_NONE) {
        CAP_PRT_ERR("get dev error,rv=%d", rv);
        
        return -SS_E_FAIL;
    }

    rv = dm_get_userslot_from_node(dm_node, &user_slot);   
    if (rv != SS_E_NONE) {
        CAP_PRT_ERR("get userslot error,rv=%d", rv);
        
        return -SS_E_FAIL;
    }
    CAP_PRT_DBG("rv %d, dm_node %#x, dev %d, phy_slot %d, user_slot %d,cpuid %d",
        rv, dm_node, dev, phy_slot, user_slot, cpuid);
    
    return g_fp_cap_mode[dev][user_slot];
    
}

/**
 * ss_fp_cap_get_string_by_dm_node ssc端获得fp_cap模式字符串
 * 
 * @dm_node:dm节点
 * 
 * @return:成功返回fp_cap_mode字符串 其它表示失败返回NULL
 */
char *ss_fp_cap_get_string_by_dm_node(int dm_node)
{
    int rv;
    int dev;
    int phy_slot;
    int cpuid;
    int user_slot;

    dev = phy_slot = cpuid = user_slot = FP_CAP_INVALID_VALUE;
    rv = dm_get_dev_slot_cpuid_from_node(dm_node, &dev, &phy_slot, &cpuid);
    if (rv != SS_E_NONE) {
        CAP_PRT_ERR("get dev error,rv=%d", rv);
        
        return NULL;
    }

    rv = dm_get_userslot_from_node(dm_node, &user_slot);
    if (rv != SS_E_NONE) {
        CAP_PRT_ERR("get userslot error,rv=%d", rv);
        
        return NULL;
    }
    CAP_PRT_DBG("rv %d, dm_node %d, dev %d, phy_slot %d, user_slot %d, cpuid %d", rv, dm_node, dev, 
        phy_slot, user_slot, cpuid);

    return ss_fp_cap_get_string_by_mode(g_fp_cap_mode[dev][user_slot]);
}

/**
 * ss_fp_cap_dump_parse_single ssc端fp_cap打印单个模式
 * 
 * @device:设备号
 * 
 * @slot:插槽号
 * 
 * @return:void
 */
void ss_fp_cap_dump_parse_single(int device, int slot)
{
    if (FP_CAP_SLOTID_INVALID(slot) || FP_CAP_DEVID_INVALID(device)) {
        AT_PRINT("Invalid device %d or slot %d\n", device, slot);
        
        return;
    }
    
    AT_PRINT("device %d, slot %d, mode %d %s\n", device, slot, g_fp_cap_mode[device][slot], 
        ss_fp_cap_get_string_by_mode(g_fp_cap_mode[device][slot]));
}

/**
 * ss_fp_cap_dump_parse_all ssc端fp_cap打印所有模式
 * 
 * @return:void
 */
void ss_fp_cap_dump_parse_all(void)
{
   int i;
   int j;

   AT_PRINT("============== FP CAP PARSE RESULT START=============\n");
   for (i = 0; i < SS_MAX_SWITCH_NUM; i++) {
        AT_PRINT("dev %d\n", i);
        for (j = 0; j < SS_MAX_SLOT_NUM; j++) {
            AT_PRINT("    slot %d, mode %d %s\n", j, g_fp_cap_mode[i][j], 
                ss_fp_cap_get_string_by_mode(g_fp_cap_mode[i][j]));
        }
    }
    AT_PRINT("=============== FP CAP PARSE RESULT END =============\n");

    return;
}

