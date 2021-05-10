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
** 文   件   名: inet.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2010 年 07 月 27 日
**
** 描        述: include/arpa/inet .
*********************************************************************************************************/

#ifndef __ARPA_INET_H
#define __ARPA_INET_H

#include <lwip/inet.h>

#ifdef __cplusplus
extern "C" {
#endif                                                                  /*  __cplusplus                 */

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int         inet_pton(int af, const char *src, void *dst);

#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  __ARPA_INET_H               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
