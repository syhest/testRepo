/*
 * Copyright(C) 2013 Ruijie Network. All rights reserved.
 */
/*
 * libproxy_apppmng_cap_cli.c  
 * Original Author: suyuhuan@ruijie.com.cn, 2018-9-7
 *
 * policy cap配置命令实现文件
 *
 * History
 *  v1.0    suyuhuan@ruijie.com.cn    2018-9-2
 *          create
 */

#include <rg_sys/rg_types.h>
#include <rg_ss/public/ss_errno.h>
#include <stdio.h>
#include <mng/cli/cli_transtion.h>
#include <libpub/rg_thread/rg_thread.h>
#include <frame/libproxy_vdu.h>
#include <rg_ss/lib/libpub/ss_comm_macro.h>
#include <rg_ss/public/policy/ss_cap_defs.h>
#include <rg_ss/public/vdu/policy/ss_vdumsg_policy_cap.h>
#include <rg_dev/kernel/dm_slot_def.h>

#undef  ALTERNATE
#define ALTERNATE       no_alt
#include "libproxy_apppmng_cap_cfg_cli.h"
APPEND_POINT(cfg_switch_policy_cap_mode_command, ALTERNATE);

#undef  ALTERNATE
#define ALTERNATE       no_alt
#include "libproxy_apppmng_cap_show_cli.h"
APPEND_POINT(exec_show_switch_policy_cap_mode_command, ALTERNATE);

static pthread_mutex_t libp_cap_lock;   /* 互斥锁 */
#define LIBP_CAP_CLI_LOCK()       pthread_mutex_lock(&libp_cap_lock)
#define LIBP_CAP_CLI_UNLOCK()     pthread_mutex_unlock(&libp_cap_lock)
#define LIBP_CAP_CLI_LOCKINIT()   (void)pthread_mutex_init(&libp_cap_lock, NULL)

bool g_cap_prt_dbg_enable = true;
#define CAP_DBGPRT(fmt, args...) do {                               \
    if (g_cap_prt_dbg_enable) {                                     \
        printf("(%s, %d): " fmt, __FILE__, __LINE__, ## args);      \
    }                                                               \
} while (0)

static cap_mode cap_cur_mode[SS_MAX_SWITCH_ID + 1][SS_MAX_SLOT_ID + 1];/* ID从1开始 */

static char *cap_mode_name[CAP_MODE_MAX] = {
    [CAP_MODE_DEFAULT] = "default",             /* 极简X默认模式 */
    [CAP_MODE_TRD] = "tradition",               /* 传统极简模式 */
    [CAP_MODE_NO_QINQ] = "no-qinq",             /* 极简X非QINQ模式 */
    [CAP_MODE_IPV4] = "ipv4",                   /* 极简X IPV4模式 */
    [CAP_MODE_IPV4_NO_QINQ] = "ipv4-no-qinq"    /* 极简X   IPV4非QINQ模式 */
};

static ss_cap_conf_t proxy_cap_conf;

static bool ssc_cap_is_fetch = false;
static bool ssc_cap_is_support = false;

static bool libp_cap_mode_sp[CAP_MODE_CNT];
static bool libp_cap_overlay_sp = false;

static void ssc_fp_cap_mode_sp_init(void)
{
    int i;

    for (i = 0; i < CAP_MODE_CNT; i++) {
        libp_cap_mode_sp[i] = true;
    }
}

bool fp_cap_check_mode_support(cap_mode mode)
{
    bool support;

    if (mode < CAP_MODE_DEFAULT || mode >= CAP_MODE_CNT) {

        return false;
    }

    support = libp_cap_mode_sp[mode];
    CAP_DBGPRT("mode=%d,support=%s\r\n", mode, support ? "true" : "false");

    return support;
}

static bool fp_cap_check_conf(void)
{
    /* 非0域不能配cap */
    if (libproxy_vdu_get_myid() == 0) {

        return true;
    }

    return false;
}

static ss_cap_ds_sts_t libproxy_fp_cap_get_ds_state(void)
{
    return proxy_cap_conf.state;
}

static void libproxy_fp_cap_set_ds_state(ss_cap_ds_sts_t state)
{
    proxy_cap_conf.state = state;
}

static int libproxy_fp_cap_get_sn(void)
{
    return proxy_cap_conf.sn;
}

static void libproxy_fp_cap_update_sn(void)
{
    proxy_cap_conf.sn++;
}

static void libproxy_fp_cap_cli_send_start(void)
{
    int ret;
    char data;
    
    data = 0;/* 空数据 */
    
    CAP_DBGPRT("===libproxy_fp_cap_cli_send_start\n");
    ret = libproxy_vdu_msg_to_ss(SS_VDUMSG_CAP_MODE_DS_BEGIN, &data, sizeof(data));
    if (ret != SS_E_NONE) {
        CAP_DBGPRT("libproxy_vdu_msg_to_ss SS_VDUMSG_CAP_MODE_DS_BEGIN error.\n");
    }
    
    return;
}

static void libproxy_fp_cap_cli_send_end(void)
{
    int ret;
    char data;
    
    data = 0;/* 空数据 */

    CAP_DBGPRT("===libproxy_fp_cap_cli_send_end, id=%d.\n", SS_VDUMSG_CAP_MODE_DS_END);
    ret = libproxy_vdu_msg_to_ss(SS_VDUMSG_CAP_MODE_DS_END, &data, sizeof(data));
    if (ret != SS_E_NONE) {
        CAP_DBGPRT("libproxy_vdu_msg_to_ss SS_VDUMSG_CAP_MODE_DS_BEGIN error.\n");
    }
    
    return;
}

static void libproxy_fp_cap_cli_recv_fetch(int vduid, char *msg, int len)
{
    CAP_DBGPRT("enter libproxy_fp_cap_cli_recv_fetch.\n");
    LIBP_CAP_CLI_LOCK();
    if (libproxy_fp_cap_get_ds_state() == SS_CAP_DS_READY) {
        libproxy_fp_cap_cli_send_end();
    }

    if (!ssc_cap_is_fetch) {
        ssc_cap_is_fetch = true;
    }

    LIBP_CAP_CLI_UNLOCK();

    return;
}

static void libproxy_fp_cap_cli_recv_support(char *msg, int len)
{
    int i;
    ss_cap_ability_t *cap_abi;
    
    CAP_DBGPRT("enter libproxy_fp_cap_cli_recv_support.\n");

    cap_abi = (ss_cap_ability_t *)msg;
    if (cap_abi == NULL) {
        CAP_DBGPRT("param error");

        return;
    }
    
    LIBP_CAP_CLI_LOCK();
    ssc_cap_is_support = cap_abi->cap_sp;

    for (i = 0; i < CAP_MODE_CNT; i++) {
        libp_cap_mode_sp[i] = cap_abi->mode_sp[i];
    }

    libp_cap_overlay_sp = cap_abi->overlay_sp;
    
    LIBP_CAP_CLI_UNLOCK();

    return;
}

static void fp_cap_switch_mode_status_show(struct_command_data_block *pcdb)
{
    int i;
    int j;
    int max_switch;
    int phy_slot;

    max_switch = dm_get_max_device_num();
    CAP_DBGPRT("max_switch=%d\r\n", max_switch);
    
    if (dm_is_vsu()) {
        cli_printf("%-10s %-10s %-15s\n", "Device No", "Slot No", "Switch-Policy-Cap-Mode");
        cli_printf("%-10s %-10s %-15s\n", "---------", "-------", "---------------");
        for (i = 0; i <= max_switch; i++) {
            for (j = 0; j < (SS_MAX_SLOT_ID + 1); j++) {/* SS_MAX_SLOT_ID从1开始 */
                phy_slot = 0;/* phy_slot初始化清零 */
                if (dm_get_physlot_by_devid_userslot(i, j, &phy_slot) != 0) {
                    continue;
                }
                CAP_DBGPRT("devid=%d, user_slot=%d, phy_slot=%d\r\n", i, j, phy_slot);
                if (dm_card_is_online(i, phy_slot, 0) && (!DM_IS_FIXED_SLOT(phy_slot))) {
                    cli_printf("%-10d %-10d %-15s\n", i, j, cap_mode_name[cap_cur_mode[i][j]]);
                }
            }
        }
    } else {
        cli_printf("%-10s %-15s\n", "Slot No", "Switch-Policy-Cap-Mode");
        cli_printf("%-10s %-15s\n", "-------", "---------------");
        for (i = 0; i <= max_switch; i++) {
            for (j = 0; j < (SS_MAX_SLOT_ID + 1); j++) {/* SS_MAX_SLOT_ID从1开始 */
                phy_slot = 0;/* phy_slot初始化清零 */
                if (dm_get_physlot_by_devid_userslot(i, j, &phy_slot) != 0) {
                    continue;
                }
                CAP_DBGPRT("devid=%d, user_slot=%d, phy_slot=%d\r\n",
                    i, j, phy_slot);
                CAP_DBGPRT("dm_card_is_online(%d, %d, 0)=%d,DM_IS_FIXED_SLOT(%d)=%d\r\n",
                    i, phy_slot, dm_card_is_online(i, phy_slot, 0), phy_slot,
                    DM_IS_FIXED_SLOT(phy_slot));
                if (dm_card_is_online(i, phy_slot, 0) && (!DM_IS_FIXED_SLOT(phy_slot))) {
                    cli_printf("%-10d %-15s\n", j, cap_mode_name[cap_cur_mode[i][j]]);
                }
            }
        }
    }

    return;
}

void fp_cap_config_mode_vsu(struct_command_data_block *pcdb)
{
    int slotid;
    int deviceid;
    cap_mode mode;
    int rv;
    ss_cap_arg_t args;
    int overlay_support;

    if (!ssc_cap_is_support) {
        cli_printf("The current command is not supported!\n");

        return;
    }

    memset(&args, 0, sizeof(args));
    mode = GETCDBVAR(int, INPUT_MODE);
    deviceid = GETCDBVAR(int, INPUT_VSU_DEVICE_ID);
    slotid = GETCDBVAR(int, INPUT_VSU_SLOT_ID);
    overlay_support = GETCDBVAR(int, INPUT_OVERLAY_SUPPORT);

    /* overlay处理 */
    if (overlay_support) {
        mode += CAP_MODE_CNT;
    }

    if (pcdb->flag_nd_prefix == NO_PREFIX || pcdb->flag_nd_prefix == DEFAULT_PREFIX) {
        args.mode = CAP_MODE_DEFAULT;
    } else {
        args.mode = mode;
    }

    args.switchid = deviceid;
    args.slotid = slotid;
    args.is_conf = false;

    if (args.mode == cap_cur_mode[args.switchid][args.slotid]) {
        cli_printf("Current mode is your config mode!\n");

        return;
    }

    rv = libproxy_fp_cap_switch_mode_set(&args);
    if (rv != SS_E_NONE) {
        cli_printf("Execute current command incorrectly!\n");    
    }

    if (pcdb->flag_nd_prefix == NO_PREFIX || pcdb->flag_nd_prefix == DEFAULT_PREFIX) {
        cap_cur_mode[args.switchid][args.slotid] = CAP_MODE_DEFAULT;
    } else {
        cap_cur_mode[args.switchid][args.slotid] = args.mode;
    }
    cli_printf("Please save current config and restart your device!\n");

    return;
}

void fp_cap_config_mode_standalone(struct_command_data_block *pcdb)
{
    cap_mode mode;
    int slotid;
    int rv;
    ss_cap_arg_t args;
    int overlay_support;

    if (!ssc_cap_is_support) {
        cli_printf("The current command is not supported!\n");
        
        return;
    }

    if (dm_is_vsu()) {
        
        return;
    }

    memset(&args, 0, sizeof(args));
    mode = GETCDBVAR(int, INPUT_MODE);
    slotid = GETCDBVAR(int, INPUT_STANDALONE_SLOT);
    overlay_support = GETCDBVAR(int, INPUT_OVERLAY_SUPPORT);

    /* overlay处理 */
    if (overlay_support) {
        mode += CAP_MODE_CNT;
    }
    
    if (pcdb->flag_nd_prefix == NO_PREFIX || pcdb->flag_nd_prefix == DEFAULT_PREFIX) {
        args.mode = CAP_MODE_DEFAULT;
    } else {
        args.mode = mode;
    }

    args.switchid = dm_get_my_devid();
    args.slotid = slotid;
    args.is_conf = false;
    CAP_DBGPRT("switchid=%d,slotid=%d,is_conf=%d\r\n", args.switchid, args.slotid, args.is_conf);

    if (args.mode == cap_cur_mode[args.switchid][args.slotid]) {
        cli_printf("Current mode is your config mode!\n");
        
        return;
    }

    rv = libproxy_fp_cap_switch_mode_set(&args);
    
    if (rv != SS_E_NONE) {
        cli_printf("Execute current command incorrectly!\n");    
    }

    if (pcdb->flag_nd_prefix == NO_PREFIX || pcdb->flag_nd_prefix == DEFAULT_PREFIX) {
        cap_cur_mode[args.switchid][args.slotid] = CAP_MODE_DEFAULT;
        CAP_DBGPRT("cur_mode[%d][%d]=%d\r\n", args.switchid, args.slotid,
            cap_cur_mode[args.switchid][args.slotid]);
    } else {
        cap_cur_mode[args.switchid][args.slotid] = args.mode;
        CAP_DBGPRT("cur_mode[%d][%d]=%d\r\n", args.switchid, args.slotid,
            cap_cur_mode[args.switchid][args.slotid]);
    }
    
    cli_printf("Please save current config and restart your device!\n");
    
    return;

}

void fp_cap_config_mode_save(struct_command_data_block *pcdb)
{
    int i;
    int j;
    char cli_str[100];/* cli string 最长不超过100 */

    if (pcdb->parser_status & (SAVE_PARAM | CLEAN_PARAM)) {
        CAP_DBGPRT("save\r\n");
        for (i = 0; i < (SS_MAX_SWITCH_ID + 1); i++) {      /* SS_MAX_SWITCH_ID从1开始 */
            for (j = 0; j < (SS_MAX_SLOT_ID + 1); j++) {    /* SS_MAX_SLOT_ID从1开始 */
                if (cap_cur_mode[i][j] != CAP_MODE_DEFAULT) {
                    CAP_DBGPRT("cap_mode[%d][%d]=%d\r\n", i, j, cap_cur_mode[i][j]);
                    if (dm_is_vsu()) {
                        sprintf(cli_str, "switch %d slot %d", i, j);
                        if (cap_cur_mode[i][j] >= CAP_MODE_CNT) {
                            c2p_printf(true, pcdb, "switch-policy-cap-mode %s overlay %s", 
                                cap_mode_name[cap_cur_mode[i][j] - CAP_MODE_CNT], cli_str);
                        } else {
                            c2p_printf(true, pcdb, "switch-policy-cap-mode %s %s", 
                                cap_mode_name[cap_cur_mode[i][j]], cli_str);
                        }
                    } else {
                        sprintf(cli_str, "%d", j);
                        if (cap_cur_mode[i][j] >= CAP_MODE_CNT) {
                            c2p_printf(true, pcdb, "switch-policy-cap-mode %s overlay slot %s", 
                                cap_mode_name[cap_cur_mode[i][j] - CAP_MODE_CNT], cli_str);
                        } else {
                            c2p_printf(true, pcdb, "switch-policy-cap-mode %s slot %s", 
                                cap_mode_name[cap_cur_mode[i][j]], cli_str);
                        }
                    }
                }
            }
        }
    }

    return;
}

/* 获取可输入的线卡描述,  */
void fp_cap_vsu_lc_slot_getlist(char *pkey, char *phelp, ulong idx)
{
    if (idx >= 0) {
        *pkey = '\0';
        *phelp = '\0';

        return;
    }

    return;
}

/* 获取当前设备的线卡范围, 单机环境 */
void fp_cap_get_lc_slot_num_range(struct_command_data_block *pcdb, long *min, long *max)
{
    int devid;
    int dev_type;
    int min_slot;
    int max_slot;
    int rv;

    if ((pcdb == NULL) || (min == NULL) || (max == NULL)) {
        
        return;
    }

    rv = SS_E_NONE;
    *min = 0;/* 初始值清零 */
    *max = 0;
    min_slot = 0;
    max_slot = 0;
    
    devid = dm_get_my_devid();
    if (devid == 0) {/* device id error */
        CAP_DBGPRT("dm_get_my_devid error, devid=%d.\n", devid);

        return;
    }

    rv = dm_get_device_type(devid, &dev_type);
    if (rv != SS_E_NONE) {
        CAP_DBGPRT("dm_get_device_type error:devid=%d.\n", devid);

        return;
    }
    
    rv = dm_get_lc_slot_range_from_device_type(dev_type, &min_slot, &max_slot);
    if (rv != SS_E_NONE) {
        CAP_DBGPRT("dm_get_user_slot_range_from_device_type error.");
    }

    *min = min_slot;
    *max = max_slot;
    
    return;
}

/* 获取当前设备ID范围, vsu环境 */
void fp_cap_get_vsu_device_id_range(struct_command_data_block *pcdb, long *min, 
        long *max)
{
    if ((pcdb == NULL) || (min == NULL) || (max == NULL)) {
        
        return;
    }

    *min = 0;/* 初始值清零 */
    *max = 0;

    *max = dm_get_max_device_num();
    *min = dm_get_min_device_num();

    return;
}

/* 获取当前slot ID范围, vsu环境 */
void fp_cap_get_vsu_slot_id_range(struct_command_data_block *pcdb, long *min, long *max)
{
    int devid;
    int dev_type;
    int min_slot;
    int max_slot;
    int rv;

    if ((pcdb == NULL) || (min == NULL) || (max == NULL)) {

        return;
    }

    rv = SS_E_NONE;
    *min = 0;/* 初始值清零 */
    *max = 0;
    min_slot = 0;
    max_slot = 0;
    
    devid = GETCDBVAR(int, INPUT_VSU_DEVICE_ID);
    if (devid == 0) {/* devid error */
        CAP_DBGPRT("dm_get_my_devid error, devid=%d.\n", devid);
        
        return;
    }

    rv = dm_get_device_type(devid, &dev_type);
    if (rv != SS_E_NONE) {
        CAP_DBGPRT("dm_get_device_type error:devid=%d.\n", devid);
        
        return;
    }
    
    rv = dm_get_lc_slot_range_from_device_type(dev_type, &min_slot, &max_slot);
    if (rv != SS_E_NONE) {
        CAP_DBGPRT("dm_get_user_slot_range_from_device_type error.");
    }

    *min = min_slot;
    *max = max_slot;
    
    return;
}

/**
 * libproxy_fp_cap_cli_recv_start libproxy fp_cap DS过程接收start信号
 *
 * @return:void
 */
void libproxy_fp_cap_cli_recv_start(void)
{
    LIBP_CAP_CLI_LOCK();
    /* CLI进程开始配置分发 */
    libproxy_fp_cap_set_ds_state(SS_CAP_DS_CACHE);
    
    /* 更新本地sn */
    libproxy_fp_cap_update_sn();

    /* 将start消息发送下去 */
    libproxy_fp_cap_cli_send_start();
    LIBP_CAP_CLI_UNLOCK();

    return;
}

/**
 * libproxy_fp_cap_cli_recv_end libproxy fp_cap DS过程接收end信号
 *
 * @return:void
 */
void libproxy_fp_cap_cli_recv_end(void)
{
    CAP_DBGPRT("enter libproxy_fp_cap_cli_recv_end.\n");
    LIBP_CAP_CLI_LOCK();
    libproxy_fp_cap_set_ds_state(SS_CAP_DS_READY);

    if (ssc_cap_is_fetch) {
        /* 将end消息发送下去 */
        libproxy_fp_cap_cli_send_end();
    }

    LIBP_CAP_CLI_UNLOCK();

    return;
}

/**
 * libproxy_policy_cap_cmd_add libproxy fp_cap添加cli命令
 *
 * @return:void
 */
void libproxy_policy_cap_cmd_add(void)
{
    cli_add_command(PARSE_ADD_SHOW_CMD, &TNAME(exec_show_switch_policy_cap_mode_command), 
        "show switch policy cap mode info");
    cli_add_command(PARSE_ADD_CFG_CMD, &TNAME(cfg_switch_policy_cap_mode_command), 
        "configure switch policy cap mode");

    CAP_DBGPRT("libproxy_policy_cap_cmd_add ok\n");
}

/**
 * libproxy_policy_cap_init_phase1 libproxy fp_cap第一阶段初始化
 *
 * @return:void
 */
void libproxy_policy_cap_init_phase1(void)
{
    int i;
    int j;
    int rv;

    LIBP_CAP_CLI_LOCKINIT();

    fp_cap_debug_read_conf();

    for (i = 0; i < (SS_MAX_SWITCH_ID + 1); i++) {/* switch ID，slot ID 从1开始 */
        for (j = 0; j < (SS_MAX_SLOT_ID + 1); j++) {
            cap_cur_mode[i][j] = CAP_MODE_DEFAULT;
        }
    }

    ssc_fp_cap_mode_sp_init();

    rv = libproxy_vdu_msg_reg(SS_VDUMSG_CAP_MODE_FETCH, 
            (ss_vdu_msg_up_proc)libproxy_fp_cap_cli_recv_fetch);
    if (rv != SS_E_NONE) {
        CAP_DBGPRT("ss_vdu_msg_reg error, rv=%d.\n", rv);
    }

    rv = libproxy_vdu_msg_reg(SS_VDUMSG_CAP_MODE_SUPPORT, 
            (ss_vdu_msg_up_proc)libproxy_fp_cap_cli_recv_support);
    if (rv != SS_E_NONE) {
        CAP_DBGPRT("ss_vdu_msg_reg error, rv=%d.\n", rv);
    }

    CAP_DBGPRT("llibproxy_policy_cap_init_phase1=== ok\n");

    return;
}

