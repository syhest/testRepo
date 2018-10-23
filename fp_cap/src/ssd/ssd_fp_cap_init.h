/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * ssd_fp_cap_init.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-8-15
 *
 * 实现ssd端fp_cap文件解析的头文件
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-9-15
 *          根据评审意见修改 
 */

#ifndef _SSD_FP_CAP_INIT_H_
#define _SSD_FP_CAP_INIT_H_

/* FP CAP模块独立初始化，不依赖于其它模块 */
extern void ssd_fp_cap_init_indepen(void);
/* FP CAP模块依赖初始化 */
extern void ssd_fp_cap_init_depen(void);

#endif /* _SSD_FP_CAP_INIT_H_ */

