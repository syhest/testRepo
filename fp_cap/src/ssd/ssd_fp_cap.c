/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */
/*
 * ssd_fp_cap.c  
 * Original Author: suyuhuan@ruijie.com.cn, 2018-9-15
 *
 * ssd端fp_cap实现文件
 *
 * History
 *  v1.0    suyuhuan@ruijie.com.cn    2018-9-15
 *          create
 */

#include <stdio.h>
#include <string.h>
#include <rg_sys/rg_types.h>
#include <rg_ss/public/ss_errno.h>
#include <rg_ss/public/policy/ss_cap_defs.h>
#include <frame/ss_vdu.h>
#include <rg_ss/public/msgdef/policy/ss_fp_msgdef.h>
#include <rg_ss/lib/libpub/ss_msg_com.h>
#include <rg_dev/dm_lib/rg_dm.h>
#include "ssd_fp_cap_debug.h"

static void ssd_fp_cap_msg_send(ss_cap_arg_t *arg)
{
    ss_info_t *ss_info;
    ss_cap_arg_t *payload;
    int rv;

    ss_info = ss_info_alloc_init(sizeof(ss_cap_arg_t));
    if (ss_info == NULL) {
        
        return;
    }
    /* 填充消息结构和参数 */
    ss_info->hdr.msgid = SPUB_MSGID_FP_CAP; 
    ss_info->hdr.msg_ver = 1;       /* 消息版本号为 1 */
    ss_info->hdr.concurrent = 1;    /* 允许并发 */
    ss_info->hdr.sender = MSG_ROLE_SSD;
    ss_info->hdr.reciver = MSG_ROLE_SSA;
    ss_info->hdr.dst_type = SS_MSG_DST_SELF_NODE;
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

static void ssd_fp_cap_switch_mode_set(ss_cap_arg_t *arg)
{
    CAP_DBGPRT("vdc_id=%d, slotid=%d, mode=%d.\n", arg->vduid, arg->slotid, arg->mode);

    /* 发送消息 */
    ssd_fp_cap_msg_send(arg);

    return;
}

static ss_msg_func_ret ssd_fp_cap_recv_msg_server(ss_rcv_msg_t *rcv_msg, int *ret)
{
    ss_info_t *msg_info;
    ss_cap_arg_t *arg;
    
    if (rcv_msg == NULL) {
        *ret = -SS_E_FAIL;
        
        return FALSE;
    }
    
    msg_info = ss_rcv_msg_get_ss_info(rcv_msg);
    if (msg_info == NULL) {
        *ret = -SS_E_FAIL;
        
        return FALSE;
    }

    arg = (ss_cap_arg_t *)msg_info->payload;
    if (arg == NULL) {
        
        return  FALSE;
    }
    ssd_fp_cap_switch_mode_set(arg);
    
    return  FALSE;
} 

/**
 * ssd_fp_cap_init_indepen ssd端fp_cap独立初始化，不依赖于其它模块
 *
 * @return:void
 */
void ssd_fp_cap_init_indepen(void)
{
    (void)ss_msg_register_handler(SPUB_MSGID_FP_CAP, ssd_fp_cap_recv_msg_server); 
    CAP_DBGPRT("ssd_fp_cap_init_indepen end.\n");
}

/**
 * ssd_fp_cap_init_depen ssd端fp_cap依赖初始化
 *
 * @return:void
 */
void ssd_fp_cap_init_depen(void)
{
    int rv;

    rv = ssd_fp_cap_debug_init();
    if (rv != SS_E_NONE) {
        AT_PRINT("ssc_fp_cap_debug_init failed rv %d", rv);
    }
    
    return;
}

