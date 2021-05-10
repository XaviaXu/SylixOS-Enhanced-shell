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
** 文   件   名: inlResourceRecord.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2006 年 12 月 20 日
**
** 描        述: 这是系统占领资源线程登记函数

** BUG
2007.09.12  加入可裁剪宏限制。
2007.11.21  修改注释.
*********************************************************************************************************/

#ifndef __INLRESOURCERECORD_H
#define __INLRESOURCERECORD_H

/*********************************************************************************************************
  Long Wing 内核通过资源按照资源分配号的顺序，顺序分配以避免死锁的发生
*********************************************************************************************************/

#if (LW_CFG_EVENT_EN > 0) && (LW_CFG_MAX_EVENTS > 0)

static LW_INLINE VOID  _ResourceRecord (PLW_CLASS_EVENT  pevent)
{
    LW_TCB_GET_CUR(pevent->EVENT_pvTcbOwn);                             /*  记录使用线程                */
}

#endif                                                                  /*  (LW_CFG_EVENT_EN > 0)       */
                                                                        /*  (LW_CFG_MAX_EVENTS > 0)     */
#endif                                                                  /*  __INLRESOURCERECORD_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
