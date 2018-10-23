/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * ssd_fp_cap_debug.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-8-15
 *
 * 实现ssd端fp_cap调试的头文件
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-9-15
 *          根据评审意见修改 
 */

#ifndef _SSD_CAP_DEBUG_H_
#define _SSD_CAP_DEBUG_H_

#include <at/at_srv.h>
#include <rg_ss/public/policy/ss_cap_defs.h>
#include <policy/ssd_fp_cap_parse.h>

AT_MODULE_NAME("ssd_fp_cap");

#define CAP_PRT(fmt, args...) do {                                        \
    if (g_fp_cap_at_init) {                                               \
        (void)AT_PRINT(fmt" (%s,%d)\n", ##args, __func__, __LINE__);      \
    } else {                                                              \
        (void)printf(fmt" (%s,%d)\n", ##args, __func__, __LINE__);        \
    }                                                                     \
} while (0)

#define CAP_PRT_DBG(fmt, args...) do {                                      \
    if (g_fp_cap_prt_dbg) {                                                 \
        if (g_fp_cap_at_init) {                                             \
            (void)AT_PRINT(fmt" (%s,%d)\n", ##args, __func__, __LINE__);    \
        } else {                                                            \
            (void)printf(fmt" (%s,%d)\n", ##args, __func__, __LINE__);      \
        }                                                                   \
    }                                                                       \
} while (0)

#define CAP_PRT_ERR(fmt, args...) do {                                      \
    if (g_fp_cap_prt_err) {                                                 \
        if (g_fp_cap_at_init) {                                             \
            (void)AT_PRINT(fmt" (%s,%d)\n", ##args, __func__, __LINE__);    \
        } else {                                                            \
            (void)printf(fmt" (%s,%d)\n", ##args, __func__, __LINE__);      \
        }                                                                   \
    }                                                                       \
} while (0)

#define CAP_DBGPRT(fmt, args...) do {                                       \
    if (g_fp_cap_prt_dbg_enable) {                                          \
        printf("(%s, %d): " fmt,  __FILE__, __LINE__, ## args);             \
    }                                                                       \
} while (0)

extern bool g_fp_cap_prt_dbg_enable;
extern bool g_fp_cap_prt_dbg;
extern bool g_fp_cap_prt_err;
extern bool g_fp_cap_at_init;

/* ssd端fp_cap 文件解析 */
extern int ss_fp_cap_parse_config(char *cfg_file);
/* ssd端fp_cap debug初始化 */
extern int ssd_fp_cap_debug_init(void);

#endif /* _SSD_FP_CAP_DEBUG_H_ */

