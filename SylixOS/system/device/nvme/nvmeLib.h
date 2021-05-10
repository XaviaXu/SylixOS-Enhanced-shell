/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: nvmeLib.h
**
** 创   建   人: Hui.Kai (惠凯)
**
** 文件创建日期: 2017 年 7 月 17 日
**
** 描        述: NVMe 驱动库.
*********************************************************************************************************/

#ifndef __NVME_LIB_H
#define __NVME_LIB_H

/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_NVME_EN > 0)

#include "nvme.h"

/*********************************************************************************************************
  调试模式
*********************************************************************************************************/
#define NVME_LOG_RUN                        __LOGMESSAGE_LEVEL
#define NVME_LOG_PRT                        __LOGMESSAGE_LEVEL
#define NVME_LOG_ERR                        __ERRORMESSAGE_LEVEL
#define NVME_LOG_BUG                        __BUGMESSAGE_LEVEL
#define NVME_LOG_ALL                        __PRINTMESSAGE_LEVEL

#define NVME_LOG                            _DebugFormat
#define NVME_INT_LOG                        _DebugFormat
#define NVME_CMD_LOG                        _DebugFormat
/*********************************************************************************************************
  标志设置、清除、检测操作
*********************************************************************************************************/
#define NVME_SET(x, y)                      (x) |= (y)
#define NVME_CLR(x, y)                      (x) &= ~(y)
#define NVME_TEST(x, y)                     (x) & (y)
/*********************************************************************************************************
  地址转换
*********************************************************************************************************/
#define NVME_ADDR_LOW32(x)                  ((UINT32)(x))
#define NVME_ADDR_HIGH32(x)                 (0)
/*********************************************************************************************************
  版本字符串格式
*********************************************************************************************************/
#define NVME_DRV_VER_STR_LEN                20
#define NVME_DRV_VER_FORMAT(ver)            "%d.%d.%d-rc%d", (ver >> 24) & 0xff, (ver >> 16) & 0xff,    \
                                            (ver >> 8) & 0xff, ver & 0xff
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
LW_API INT  API_NvmeCtrlIntConnect(NVME_CTRL_HANDLE   hCtrl,
                                   NVME_QUEUE_HANDLE  hQueue,
                                   PINT_SVR_ROUTINE   pfuncIsr,
                                   CPCHAR             cpcName);

#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_NVME_EN > 0)        */
#endif                                                                  /*  __NVME_LIB_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
