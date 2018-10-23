/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 */ 
/*
 * ssc_fp_cap_init.h
 * Original Author:  suyuhuan@ruijie.com.cn, 2018-8-15
 *
 * 实现ssc端fp_cap初始化的头文件
 *
 * History
 * v1.1     suyuhuan@ruijie.com.cn        2018-8-15
 *          根据评审意见修改 
 */

#ifndef _SSC_FP_CAP_INIT_H_
#define _SSC_FP_CAP_INIT_H_

/* fp_cap模块独立初始化，不依赖于其它模块 */
extern void ssc_fp_cap_init_indepen(void);
/* fp_cap模块依赖初始化 */
extern void ssc_fp_cap_init_depen(void);

#endif /* _SSC_FP_CAP_INIT_H_ */

