/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * libproxy_cap.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-9-15
 *
 * 实现fp_cap模块资源头文件
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-9-15
 *          根据评审意见修改 
 */

#ifndef _LIBPROXY_CAP_H_
#define _LIBPROXY_CAP_H_

#include <rg_ss/public/policy/ss_cap_defs.h>

/* CLI命令fp_cap模式设置 */
extern int libproxy_fp_cap_switch_mode_set(ss_cap_arg_t *args);
/* libproxy fp_cap第一阶段初始化 */
extern void libproxy_policy_cap_init_phase1(void);
/* libproxy fp_cap DS过程接收end信号 */
extern void libproxy_fp_cap_cli_recv_end(void);
/* libproxy fp_cap DS过程接收start信号 */
extern void libproxy_fp_cap_cli_recv_start(void);
/* libproxy fp_cap 解析debug配置文件 */
extern void fp_cap_debug_read_conf(void);

#endif /* _LIBPROXY_CAP_H_ */

