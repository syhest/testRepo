/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * ssc_fpc_cap_debug.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-9-15
 *
 * 实现fp_cap_debug模块资源头文件
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-9-15
 *          根据评审意见修改 
 */

#ifndef _SSC_FP_CAP_DEBUG_H_
#define _SSC_FP_CAP_DEBUG_H_

#include <at/at_srv.h>
#include <rg_ss/public/policy/ss_cap_defs.h>
#include <policy/ssc_fp_cap_parse.h>

AT_MODULE_NAME("ssc_fp_cap");

#define CAP_PRT(fmt, args...) do {                                      \
    if (g_fp_cap_at_init) {                                             \
        (void)AT_PRINT(fmt" (%s,%d)\n", ##args, __func__, __LINE__);    \
    } else {                                                            \
        (void)printf(fmt" (%s,%d)\n", ##args, __func__, __LINE__);      \
    }                                                                   \
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
   if (g_fp_cap_prt_dbg_enable) {                                           \
        printf("(%s, %d): " fmt, __FILE__, __LINE__, ## args);              \
    }                                                                       \
} while (0)

extern bool g_fp_cap_prt_dbg_enable;
extern bool g_fp_cap_prt_dbg;
extern bool g_fp_cap_prt_err;
extern bool g_fp_cap_at_init;

/* ssc端fp_cap解析配置文件 */
extern int ss_fp_cap_parse_config(char *cfg_file);
/* ssc端fp_cap打印所有解析结果 */
extern void ss_fp_cap_dump_parse_all(void);
/* ssc端fp_cap打印单个解析结果 */
extern void ss_fp_cap_dump_parse_single(int device, int slot);
/* ssc端fp_cap打印能力值 */
extern void fp_cap_at_dump_mode_support(void);
/* ssc端fp_cap调试初始化 */
extern int ssc_fp_cap_debug_init(void);

#endif /* _SSC_FP_CAP_DEBUG_H_ */

