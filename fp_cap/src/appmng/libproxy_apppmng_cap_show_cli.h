/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * libproxy_apppmng_cap_show_cli.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-9-15
 *
 * 实现fp_cap模块CLI show头文件
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-9-15
 *          根据评审意见修改 
 */

#ifndef _LIBPROXY_APPPMNG_CAP_SHOW_CLI_H_
#define _LIBPROXY_APPPMNG_CAP_SHOW_CLI_H_

#include <mng/cli/cli_transtion.h>

static void fp_cap_switch_mode_status_show(struct_command_data_block *pcdb);
static bool fp_cap_check_conf(void);

EOLWOS(exec_show_policy_cap_mode_status_eol, fp_cap_switch_mode_status_show);

KEYWORD(switch_policy_cap_mode_status, exec_show_policy_cap_mode_status_eol, no_alt,
    "status", "status", PRIVILEGE_CONFIG);

KEYWORD_MINM(exec_show_switch_policy_cap_mode, switch_policy_cap_mode_status, no_alt,
    "switch-policy-cap-mode", "switch-policy-cap-mode", PRIVILEGE_CONFIG, 8);/* 关键字匹配8个字符 */

TEST_EXPRESSION(cap_can_show, exec_show_switch_policy_cap_mode, no_alt, no_alt,
    fp_cap_check_conf());

#undef ALTERNATE
#define ALTERNATE   cap_can_show  

#endif /* _LIBPROXY_APPPMNG_CAP_SHOW_CLI_H_ */

