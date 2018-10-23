/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * libproxy_apppmng_cap_cfg_cli.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-9-15
 *
 * 实现fp_cap模块CLI配置头文件
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-9-15
 *          根据评审意见修改 
 */

#ifndef _LIBPROXY_APPPMNG_CAP_CFG_CLI_H_
#define _LIBPROXY_APPPMNG_CAP_CFG_CLI_H_

#include <rg_ss/public/policy/ss_cap_defs.h>
#include <mng/cli/cli_transtion.h>
#include <rg_dev/dm_lib/rg_dm_vsu.h>
#include <rg_dev/dm_lib/rg_dm.h>

#define INPUT_MODE                  (1U)
#define INPUT_VSU_SLOT              (2U)
#define INPUT_STANDALONE_SLOT       (3U)
#define INPUT_VSU_DEVICE_ID         (4U)
#define INPUT_VSU_SLOT_ID           (5U)
#define INPUT_OVERLAY_SUPPORT       (6U)

extern void fp_cap_config_mode_vsu(struct_command_data_block *pcdb);
extern void fp_cap_config_mode_standalone(struct_command_data_block *pcdb);
extern void fp_cap_get_lc_slot_num_range(struct_command_data_block *pcdb, long *min, 
                long *max);
extern void fp_cap_config_mode_save(struct_command_data_block *pcdb);
extern void fp_cap_vsu_lc_slot_getlist(char *pkey, char *phelp, ulong idx);
extern void fp_cap_get_vsu_slot_id_range(struct_command_data_block *pcdb, long *min, 
                long *max);
extern void fp_cap_get_vsu_device_id_range(struct_command_data_block *pcdb, long *min, 
                long *max);
static bool fp_cap_check_conf(void);
extern bool fp_cap_check_mode_support(cap_mode mode);

EOLWOS(exec_fp_cap_config_mode_vsu_eol, fp_cap_config_mode_vsu);
EOLWOS(exec_fp_cap_config_mode_standalone_eol, fp_cap_config_mode_standalone);

/* vsu环境 swich-mode xx slot xx */
NUM_FUNC_SET_RANGE(cap_vsu_slot_id, exec_fp_cap_config_mode_vsu_eol, no_alt, 
    "slot id range", CDBVAR(int, INPUT_VSU_SLOT_ID), 1, 16, fp_cap_get_vsu_slot_id_range);

KEYWORD(cap_vsu_slot, cap_vsu_slot_id, no_alt, "slot", "slot id", PRIVILEGE_CONFIG);

/* vsu环境 swich-mode xx device xx */
NUM_FUNC_SET_RANGE(cap_vsu_switch_id, cap_vsu_slot, no_alt, "switch id range", 
    CDBVAR(int, INPUT_VSU_DEVICE_ID), 1, 16, fp_cap_get_vsu_device_id_range);

KEYWORD(cap_vsu_mode, cap_vsu_switch_id, no_alt, "switch", "switch id", PRIVILEGE_CONFIG);

/* 单机情况 swich-mode xx slot xx */
NUM_FUNC_SET_RANGE(cap_standalone_slot, exec_fp_cap_config_mode_standalone_eol,\
    no_alt, "Slot number range", CDBVAR(int, INPUT_STANDALONE_SLOT), 1, 16,\
    fp_cap_get_lc_slot_num_range);

/* swich-mode xx slot */
KEYWORD(cap_config_mode_slot, cap_standalone_slot, no_alt, "slot", "slot id", PRIVILEGE_CONFIG);

/* 根据工作模式选择命令的输入方式 */
TEST_EXPRESSION(fp_cap_config_mode_vsu_test, cap_vsu_mode, cap_config_mode_slot, ALTERNATE, 
    dm_is_vsu());

/* swich-mode xx */
KEYWORD_SET_CDBVAR(trd_mode, fp_cap_config_mode_vsu_test, no_alt, "tradition",
    "tradition mode", PRIVILEGE_CONFIG, CDBVAR(int, INPUT_MODE), CAP_MODE_TRD);

TEST_EXPRESSION(trd_mode_can_set, trd_mode, no_alt, no_alt, 
    fp_cap_check_mode_support(CAP_MODE_TRD));

KEYWORD_SET_CDBVAR(no_qinq_mode, fp_cap_config_mode_vsu_test, trd_mode_can_set, "no-qinq",
    "no-qinq mode", PRIVILEGE_CONFIG, CDBVAR(int, INPUT_MODE), CAP_MODE_NO_QINQ);

TEST_EXPRESSION(no_qinq_mode_can_set, no_qinq_mode, trd_mode_can_set, no_alt, 
    fp_cap_check_mode_support(CAP_MODE_NO_QINQ));

KEYWORD_SET_CDBVAR(ipv4_mode, fp_cap_config_mode_vsu_test, no_qinq_mode_can_set, "ipv4", 
    "ipv4 mode", PRIVILEGE_CONFIG, CDBVAR(int, INPUT_MODE), CAP_MODE_IPV4);

TEST_EXPRESSION(ipv4_mode_can_set, ipv4_mode, no_qinq_mode_can_set, no_alt, 
    fp_cap_check_mode_support(CAP_MODE_IPV4));

KEYWORD_MINM_SET_CDBVAR(ipv4_no_qinq_mode, fp_cap_config_mode_vsu_test, ipv4_mode_can_set,
    "ipv4-no-qinq", "ipv4-no-qinq mode", PRIVILEGE_CONFIG, CDBVAR(int, INPUT_MODE),
    CAP_MODE_IPV4_NO_QINQ, 10);/* 匹配长度为10 */

TEST_EXPRESSION(ipv4_no_qinq_mode_can_set, ipv4_no_qinq_mode, ipv4_mode_can_set, no_alt, 
    fp_cap_check_mode_support(CAP_MODE_IPV4_NO_QINQ));

KEYWORD_SET_CDBVAR(default_mode, fp_cap_config_mode_vsu_test, ipv4_no_qinq_mode_can_set, 
    "default", "default mode", PRIVILEGE_CONFIG, CDBVAR(int, INPUT_MODE), CAP_MODE_DEFAULT);

NOPREFIX(config_cap_mode_no, default_mode, default_mode);

C2PWOS(config_cap_mode_c2p, config_cap_mode_no, fp_cap_config_mode_save);

KEYWORD_MINM(exec_cap_config_cap_mode, config_cap_mode_c2p, no_alt,
    "switch-policy-cap-mode", "switch-policy-cap-mode", PRIVILEGE_CONFIG, 8);

TEST_EXPRESSION(cap_can_set, exec_cap_config_cap_mode, no_alt, no_alt, fp_cap_check_conf());

#undef ALTERNATE
#define ALTERNATE   cap_can_set

#endif /* _LIBPROXY_APPPMNG_CAP_CFG_CLI_H_ */

