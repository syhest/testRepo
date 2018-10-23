/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */
/*
 * ssc_fp_cap.c  
 * Original Author: suyuhuan@ruijie.com.cn, 2018-9-5
 *
 * ssc端fp_cap实现文件
 *
 * History
 *  v1.0    suyuhuan@ruijie.com.cn    2018-9-5
 *          create
 */

#include <stdio.h>
#include <string.h>
#include <rg_sys/rg_types.h>
#include <rg_ss/public/ss_errno.h>
#include <rg_ss/public/vdu/policy/ss_vdumsg_policy_cap.h>
#include <rg_ss/public/msgdef/policy/ss_fp_msgdef.h>
#include <rg_ss/lib/libpub/ss_msg_com.h>
#include <rg_dev/dm_lib/rg_dm.h>
#include <rg_ss/lib/libpub/ss_comm_macro.h>
#include <rg_ss/lib/libpub/ss_ds_service.h>
#include <rg_dev/dm_lib/rg_dm_vsu.h>
#include <rg_dev/dm_lib/rg_dm.h>
#include <rg_ss/public/policy/ss_cap_defs.h>
#include <frame/ss_vdu.h>
#include <policy/ssc_fp_frm.h>
#include "ssc_fp_cap_debug.h"

#ifdef NEVER
#include "../../include/ss_uft_cap.h"
#endif /* NEVER */

static ss_cap_data_t ssc_cap_info[SS_MAX_SWITCH_ID + 1][SS_MAX_SLOT_ID + 1];/* id从1开始 */
static ss_cap_conf_t ssc_cap_conf;

static ss_cap_ds_sts_t ssc_fp_cap_get_ds_state(void)
{
    return ssc_cap_conf.state;
}

static void ssc_fp_cap_set_ds_state(ss_cap_ds_sts_t state)
{
    ssc_cap_conf.state = state;
}

static int ssc_fp_cap_get_sn(void)
{
    return ssc_cap_conf.sn;
}

static void ssc_fp_cap_update_sn(void)
{
    ssc_cap_conf.sn++;
}

static void ssc_fp_cap_msg_send(ss_cap_arg_t *arg)
{
    ss_info_t *ss_info;
    ss_cap_arg_t *payload;
    int node;
    int rv;

    ss_info = ss_info_alloc_init(sizeof(ss_cap_arg_t));
    if (ss_info == NULL) {
        
        return;
    }
    /* 填充消息结构和参数 */
    ss_info->hdr.msgid = SPUB_MSGID_FP_CAP;
    ss_info->hdr.msg_ver = 1;       /* 消息版本号 */
    ss_info->hdr.concurrent = 1;    /* 允许并发 */
    ss_info->hdr.sender = MSG_ROLE_SSC;
    ss_info->hdr.reciver = MSG_ROLE_SSD;
    if (arg->slotid == GLOBAL_SET_SLOT) {
        ss_info->hdr.dst_type = SS_MSG_DST_ALL_DIST_NODE;
    } else {
        ss_info->hdr.dst_type = SS_MSG_DST_SPEC_DIST_NODE;
        rv = dm_get_node_from_userslot(arg->switchid, arg->slotid, &node);
        if (rv != SS_E_NONE) {
            ss_info_free(ss_info);
            
            return;
        }
        ss_info->hdr.dst_dm_node = node;
    }
    payload = (ss_cap_arg_t *)ss_info->payload;
    memcpy(payload, arg, sizeof(ss_cap_arg_t));
    /* 发送消息 */
    rv = ss_msg_send(ss_info);
    if (rv != SS_E_NONE) {
        CAP_DBGPRT("ss_msg_send error...\n");    
    }
    /* 释放消息内容 */
    ss_info_free(ss_info);
    
    return;
}

static void ssc_fp_cap_switch_mode_set_out(ss_pthrd_hdl_t *hdl)
{
    ss_cap_arg_t *arg;
    
    arg = (ss_cap_arg_t *)(hdl->arg);

    if (arg == NULL) {
        return;
    }

    if (ssc_fp_cap_get_ds_state() == SS_CAP_DS_UNREADY) {
        free(arg);
        return;
    }

    ssc_cap_info[arg->switchid][arg->slotid].mode = arg->mode;
    ssc_cap_info[arg->switchid][arg->slotid].sn = ssc_fp_cap_get_sn();

    CAP_DBGPRT("vdc_id=%d, switchid=%d, slotid=%d, mode=%d.\n", 
        arg->vduid, arg->switchid, arg->slotid, arg->mode);

    if (ssc_fp_cap_get_ds_state() == SS_CAP_DS_READY) {
        /* 发送消息 */
        ssc_fp_cap_msg_send(arg);
    }

    free(arg);

    return;
}

static void ssc_fp_cap_rcv_ds_start_out(ss_pthrd_hdl_t *hdl)
{
    ss_cap_arg_t *arg;
    
    arg = (ss_cap_arg_t *)(hdl->arg);

    if (arg == NULL) {
        
        return;
    }

    CAP_DBGPRT("ssc_cap_rcv_ds_start_out:vdc_id=%d, switchid=%d, slotid=%d, mode=%d.\n", 
        arg->vduid, arg->switchid, arg->slotid, arg->mode);

    /* 先更新sn的值 */
    ssc_fp_cap_update_sn();
    if (!ss_ds_has_glb_converged(DS_DIR_DOWN)) {
        /* 全局未收敛才需要设置状态 */
        ssc_fp_cap_set_ds_state(SS_CAP_DS_CACHE);
    }

    free(arg);

    return;
}

static void ssc_fp_cap_rcv_ds_end_out(ss_pthrd_hdl_t *hdl)
{
    ss_cap_arg_t *arg;
    
    arg = (ss_cap_arg_t *)(hdl->arg);

    if (arg == NULL) {
        
        return;
    }

    CAP_DBGPRT("ssc_fp_cap_rcv_ds_end_out:vdc_id=%d, switchid=%d, slotid=%d, mode=%d.\n", 
        arg->vduid, arg->switchid, arg->slotid, arg->mode);

    CAP_DBGPRT("ss_ds_has_glb_converged(DS_DIR_DOWN):%d\n", ss_ds_has_glb_converged(DS_DIR_DOWN));
    if (!ss_ds_has_glb_converged(DS_DIR_DOWN)) {
        /* 全局未收敛才需要feedback */
        CAP_DBGPRT("DS_POLICY_CAP --->ss_ds_rcv_feedback.\n");
        ss_ds_rcv_feedback(DS_DIR_DOWN, DS_POLICY_FP_CAP, DS_RESULT_OK, NULL);
    }
    
    free(arg);

    return;
}

static void ssc_fp_cap_ds_all_conf(void)
{
    int i;
    int j;
    int max_switch;
    ss_cap_arg_t arg;
    int phy_slot;

    max_switch = dm_get_max_device_num();

    CAP_DBGPRT("ssc_cap_ds_all_conf:max=%d.\n",max_switch);
    for (i = 0; i <= max_switch; i++) {
        for (j = 0; j < (SS_MAX_SLOT_ID + 1); j++) {
            phy_slot = 0;/* phy_slot清零 */
            if (dm_get_physlot_by_devid_userslot(i, j, &phy_slot) != 0) {
                continue;
            }
            CAP_DBGPRT("devid=%d, user_slot=%d, phy_slot=%d\r\n", i, j, phy_slot);
            if (dm_card_is_online(i, phy_slot, 0)) {
                memset(&arg, 0, sizeof(ss_cap_arg_t));
                arg.is_conf = true;
                arg.switchid = i;
                arg.slotid = j;
                arg.mode = ssc_cap_info[i][j].mode;
                arg.vduid = 0;/* vdu_id = 0 */
                arg.opcode = CAP_OP_MODE_SET;
                ssc_fp_cap_msg_send(&arg);
            }
        }
    }
}

static void ssc_fp_cap_ds_one_node_conf(int dm_node)
{
    int switch_id, user_slot;
    ss_cap_arg_t arg;
    int rv;

    rv = dm_get_userslot_from_node(dm_node, &user_slot);
    if (rv != SS_E_NONE) {
        
        return;
    }

    switch_id = dm_get_switch_id_from_node(dm_node);

    CAP_DBGPRT("ssc_fp_cap_ds_one_node_conf:switch_id=%d, user_slot=%d.\n", switch_id, user_slot);

    memset(&arg, 0, sizeof(ss_cap_arg_t));
    arg.is_conf = true;
    arg.switchid = switch_id;
    arg.slotid = user_slot;
    arg.mode = ssc_cap_info[switch_id][user_slot].mode;
    arg.vduid = 0;/* vdu_id = 0 */
    arg.opcode = CAP_OP_MODE_SET;
    ssc_fp_cap_msg_send(&arg);

    return;
}

static void ssc_fp_cap_ds_fetch_func_out(ss_pthrd_hdl_t *hdl)
{
    ds_fetch_param_t *param;
    char data;
    int rv;

    data = 0;/* data 清零 */
    param = (ds_fetch_param_t *)(hdl->arg);

    if (param == NULL) {
        
        return;
    }

    CAP_DBGPRT("enter ssc_cap_ds_fetch_func_out.\n"); 
    /* 向vdu 0 发送 */
    rv = ss_vdu_msg_snd_to_proxy(0, SS_VDUMSG_CAP_MODE_FETCH, &data, sizeof(data));

    if (rv != SS_E_NONE) {
        CAP_DBGPRT("ss_vdu_msg_snd_to_proxy send error, rv=%d.\n", rv);
    }
    
    free(param);

    return;
}

static void ssc_fp_cap_ds_service_func_out(ss_pthrd_hdl_t *hdl)
{
    ds_param_t *param;
    int node;

    param = (ds_param_t *)(hdl->arg);

    if (param == NULL) {
        
        return;
    }

    CAP_DBGPRT("ssc_fp_cap_ds_service_func_out:ds_dst: 0x%x, dm_node: 0x%x\n", param->ds_dst, 
        param->dm_node);
    
    if (param->ds_dst == DS_TO_ALL_DIST) {
        /* 下发所有节点配置 */
        ssc_fp_cap_ds_all_conf();
    } else { 
        /* 下发特定节点配置 */
        node = param->dm_node;
        ssc_fp_cap_ds_one_node_conf(node);
    }

    ssc_fp_cap_set_ds_state(SS_CAP_DS_READY);
    free(param);
}

static void ssc_fp_cap_rcv_ds_start(int vduid, char *msg, int len)
{
    ss_cap_arg_t *arg;

    arg = malloc(sizeof(ss_cap_arg_t));
    if (arg == NULL) {
        return;
    }
    memset(arg, 0, sizeof(ss_cap_arg_t));
    arg->vduid = vduid;
    arg->opcode = CAP_OP_DS_START;

    CAP_DBGPRT("vdc_id=%d, switchid=%d, slotid=%d, mode=%d, opcode=%d.\n", 
        arg->vduid, arg->switchid, arg->slotid, arg->mode, arg->opcode);

    if (!ss_fp_basic_add_hdl(ssc_fp_cap_rcv_ds_start_out, arg, 0)) {
        free(arg);
        
        return;
    }

    return;
}

static void ssc_fp_cap_rcv_ds_end(int vduid, char *msg, int len)
{    
    ss_cap_arg_t *arg;

    arg = malloc(sizeof(ss_cap_arg_t));
    if (arg == NULL) {
        
        return;
    }
    memset(arg, 0, sizeof(ss_cap_arg_t));
    arg->vduid = vduid;
    arg->opcode = CAP_OP_DS_END;

    CAP_DBGPRT("vdc_id=%d, switchid=%d, slotid=%d, mode=%d, opcode=%d.\n", 
        arg->vduid, arg->switchid, arg->slotid, arg->mode, arg->opcode);

    if (!ss_fp_basic_add_hdl(ssc_fp_cap_rcv_ds_end_out, arg, 0)) {
        free(arg);
        
        return;
    }

    return;
}

static void ssc_fp_cap_switch_mode_set(int vduid, char *msg, int len)
{
    ss_cap_arg_t *arg;

    arg = malloc(sizeof(ss_cap_arg_t));
    if (arg == NULL) {
        
        return;
    }
    memset(arg, 0, sizeof(ss_cap_arg_t));
    memcpy(arg, msg, len);
    arg->vduid = vduid;
    arg->opcode = CAP_OP_MODE_SET;

    CAP_DBGPRT("vdc_id=%d, switchid=%d, slotid=%d, mode=%d, opcode=%d.\n", 
        arg->vduid, arg->switchid, arg->slotid, arg->mode, arg->opcode);

    if (!ss_fp_basic_add_hdl(ssc_fp_cap_switch_mode_set_out, arg, 0)) {
        free(arg);
        
        return;
    }

    return;
}

static int ssc_fp_cap_ds_fetch_func(ds_fetch_param_t *param)
{
    ds_fetch_param_t  *fetch_info;
    
    CAP_DBGPRT("enter ssc_fp_cap_ds_fetch_func\n");
    
    fetch_info = (ds_fetch_param_t *)malloc(sizeof(ds_fetch_param_t));
    if (fetch_info == NULL) {
        
        return -SS_E_PARAM;
    }
    memset(fetch_info, 0, sizeof(ds_fetch_param_t));
    
    if (!ss_fp_basic_add_hdl(ssc_fp_cap_ds_fetch_func_out, fetch_info, 0)) {
        free(fetch_info);
        
        return -SS_E_PARAM;
    }
    
    return SS_E_NONE;
}

static int ssc_fp_cap_ds_service_func(ds_param_t *param)
{
    ds_param_t *ds_info;

    /* 不处理热退出 */
    if (param == NULL) {
        
        return -SS_E_PARAM;
    }
    
    if (param->flag == DS_FLAG_PEER_LEAVE) {
        return SS_E_NONE;
    }

    ds_info = (ds_param_t *)malloc(sizeof(ds_param_t));
    if (ds_info == NULL) {
        
        return -SS_E_PARAM;
    }
    memcpy(ds_info, param, sizeof(ds_param_t));
    
    if (!ss_fp_basic_add_hdl(ssc_fp_cap_ds_service_func_out, ds_info, 0)) {
        free(ds_info);
        
        return -SS_E_PARAM;
    }

    return SS_E_NONE;
}

static int ssc_fp_cap_ds_init(void)
{
    int rv;

    rv = ss_ds_fetch_func_reg(DS_DIR_DOWN, DS_POLICY_FP_CAP, 
            (fetch_func_t)ssc_fp_cap_ds_fetch_func);
    if (rv != SS_E_NONE) {
        
        return rv;
    }
    rv = ss_ds_service_reg(DS_DIR_DOWN, DS_POLICY_FP_CAP, FALSE, ssc_fp_cap_ds_service_func);
    if (rv != SS_E_NONE) {
        
        return rv;
    }

    CAP_DBGPRT("ssc_fp_cap_ds_init sucess...\n");

    return SS_E_NONE;
}

/**
 * ssc_fp_cap_init_indepen ssc端fp_cap独立初始化，不依赖于其它模块
 *
 * @return: void
 */
void ssc_fp_cap_init_indepen(void)
{
    int i;
    int j;
    int rv;
    
    for (i = 0; i < (SS_MAX_SWITCH_ID + 1); i++) {  /* SWITCH_ID 从1开始 */
        for (j = 0; j < (SS_MAX_SLOT_ID + 1); j++) {/* SLOT_ID 从1开始 */
            ssc_cap_info[i][j].mode = CAP_MODE_DEFAULT;
        }
    }

    memset(&ssc_cap_conf, 0, sizeof(ssc_cap_conf));
    ssc_cap_conf.state = SS_CAP_DS_UNREADY;
    
    (void)ss_vdu_msg_reg(SS_VDUMSG_CAP_MODE_SET, ssc_fp_cap_switch_mode_set);
    (void)ss_vdu_msg_reg(SS_VDUMSG_CAP_MODE_DS_BEGIN, ssc_fp_cap_rcv_ds_start);
    (void)ss_vdu_msg_reg(SS_VDUMSG_CAP_MODE_DS_END, ssc_fp_cap_rcv_ds_end);

    rv = ssc_fp_cap_ds_init();
    if (rv != SS_E_NONE) {
        printf("ssc_fp_cap_ds_init err");

        CAP_DBGPRT("ssc_fp_cap_init_indepen error, rv=%d.\n", rv);

    }
    printf("ssc_fp_cap_ds_init success");
    
    return;
}

/**
 * ssc_fp_cap_init_depen ssc端fp_cap依赖初始化
 *
 * @return: void
 */
void ssc_fp_cap_init_depen(void)
{
    int rv;
    ss_cap_ability_t fp_cap_abi;
    int i;

    rv = ssc_fp_cap_debug_init();
    if (rv != SS_E_NONE) {
        AT_PRINT("ssc_fp_cap_debug_init failed rv %d", rv);
    }

    fp_cap_abi.cap_sp = 1;      /* 能力值cap_sp = 1 */
    fp_cap_abi.overlay_sp = 1;  /* 能力值overlay_sp    = 1 */
    for (i = 0; i < CAP_MODE_CNT; i++) {
        fp_cap_abi.mode_sp[i] = 1;/* 能力值fp_cap_abi.mode_sp    = 1 */
    }

    rv = ss_vdu_msg_snd_to_proxy(0, SS_VDUMSG_CAP_MODE_SUPPORT, (char *)&fp_cap_abi, 
            sizeof(fp_cap_abi));
    if (rv != SS_E_NONE) {

        CAP_DBGPRT("ss_vdu_msg_snd_to_proxy send error, rv=%d.\n", rv);
    }

    return;
}

