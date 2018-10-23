/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * libproxy_cap.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-9-15
 *
 * ʵ��fp_capģ����Դͷ�ļ�
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-9-15
 *          ������������޸� 
 */

#ifndef _LIBPROXY_CAP_H_
#define _LIBPROXY_CAP_H_

#include <rg_ss/public/policy/ss_cap_defs.h>

/* CLI����fp_capģʽ���� */
extern int libproxy_fp_cap_switch_mode_set(ss_cap_arg_t *args);
/* libproxy fp_cap��һ�׶γ�ʼ�� */
extern void libproxy_policy_cap_init_phase1(void);
/* libproxy fp_cap DS���̽���end�ź� */
extern void libproxy_fp_cap_cli_recv_end(void);
/* libproxy fp_cap DS���̽���start�ź� */
extern void libproxy_fp_cap_cli_recv_start(void);
/* libproxy fp_cap ����debug�����ļ� */
extern void fp_cap_debug_read_conf(void);

#endif /* _LIBPROXY_CAP_H_ */

