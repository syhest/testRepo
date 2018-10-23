/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */
/*
 * libproxy_cap.c
 * Original Author: suyuhuan@ruijie.com.cn, 2018-9-9
 * �����capʵ���ļ�
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
#include <frame/libproxy_vdu.h>

extern bool g_cap_prt_dbg_enable;
static bool fp_cap_debug_file_enable(char *file)
{
    FILE *fp = NULL;
        
    if (file == NULL) {
        
        return FALSE;
    }
        
    fp = fopen(file, "r");
    if (fp == NULL) {
        
        return FALSE;
    }
    fclose(fp);
        
    return TRUE;
}

/**
 * fp_cap_debug_read_conf ����debug�����ļ�
 *
 * @return:void
 */
void fp_cap_debug_read_conf(void)
{
    /* shell��
     * mkdir -p /data/cap/
     * touch /data/cap/cap_dbg
     * sync
     */
    if (fp_cap_debug_file_enable("/data/cap/cap_dbg")) {
        g_cap_prt_dbg_enable = true;
    } else {
        g_cap_prt_dbg_enable = false;
    }
    
    return;
}

/**
 * libproxy_fp_cap_switch_mode_set CLI����fp_capģʽ����
 *
 * @args:ָ��fp_ap��ģʽ�Ľṹ��ָ��
 *
 * @return:�ɹ�����SS_E_NONE ������ʾʧ�ܼ�ss_errno.h
 */
int libproxy_fp_cap_switch_mode_set(ss_cap_arg_t *args)
{
    int rv;

    rv = SS_E_NONE;
    
    if (args == NULL) {
        
        return -SS_E_PARAM;
    }
    
    printf("libproxy: switchid=%d, slotid=%d, mode=%d.\n", args->switchid, args->slotid,
        args->mode);

    rv = libproxy_vdu_msg_to_ss(SS_VDUMSG_CAP_MODE_SET, (char *)args, sizeof(ss_cap_arg_t));
    if (rv != SS_E_NONE) {
        printf("vdu msg to ss fail,rv=%d.\n", rv);

        return rv;
    }

    return rv;
}

