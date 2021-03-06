/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: sparcVfpNone.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 09 月 26 日
**
** 描        述: SPARC 体系架构无 VFP 支持.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_FPU_EN > 0
#include "../sparcFpu.h"
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static SPARC_FPU_OP   _G_fpuopVfpNone;
/*********************************************************************************************************
** 函数名称: sparcVfpNoneEnable
** 功能描述: 使能 VFP
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sparcVfpNoneEnable (VOID)
{
}
/*********************************************************************************************************
** 函数名称: sparcVfpNoneDisable
** 功能描述: 禁能 VFP
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sparcVfpNoneDisable (VOID)
{
}
/*********************************************************************************************************
** 函数名称: sparcVfpNoneIsEnable
** 功能描述: 是否使能了 VFP
** 输　入  : NONE
** 输　出  : 是否使能
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static BOOL  sparcVfpNoneIsEnable (VOID)
{
    return  (LW_TRUE);
}
/*********************************************************************************************************
** 函数名称: sparcVfpNoneSave
** 功能描述: 保存 VFP 上下文
** 输　入  : pvFpuCtx  VFP 上下文
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sparcVfpNoneSave (PVOID  pvFpuCtx)
{
}
/*********************************************************************************************************
** 函数名称: sparcVfpNoneRestore
** 功能描述: 恢复 VFP 上下文
** 输　入  : pvFpuCtx  VFP 上下文
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sparcVfpNoneRestore (PVOID  pvFpuCtx)
{
}
/*********************************************************************************************************
** 函数名称: sparcVfpNoneCtxShow
** 功能描述: 显示 VFP 上下文
** 输　入  : iFd       输出文件描述符
**           pvFpuCtx  VFP 上下文
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sparcVfpNoneCtxShow (INT  iFd, PVOID  pvFpuCtx)
{
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
    fdprintf(iFd, "no vfp\n");
#endif
}
/*********************************************************************************************************
** 函数名称: sparcVfpNoneEnableTask
** 功能描述: 系统发生 undef 异常时, 使能任务的 VFP
** 输　入  : ptcbCur    当前任务控制块
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  sparcVfpNoneEnableTask (PLW_CLASS_TCB  ptcbCur)
{
}
/*********************************************************************************************************
** 函数名称: sparcVfpNonePrimaryInit
** 功能描述: 获取 VFP 控制器操作函数集
** 输　入  : pcMachineName 机器名
**           pcFpuName     浮点运算器名
** 输　出  : 操作函数集
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
PSPARC_FPU_OP  sparcVfpNonePrimaryInit (CPCHAR  pcMachineName, CPCHAR  pcFpuName)
{
    _G_fpuopVfpNone.SFPU_pfuncEnable     = sparcVfpNoneEnable;
    _G_fpuopVfpNone.SFPU_pfuncDisable    = sparcVfpNoneDisable;
    _G_fpuopVfpNone.SFPU_pfuncIsEnable   = sparcVfpNoneIsEnable;
    _G_fpuopVfpNone.SFPU_pfuncSave       = sparcVfpNoneSave;
    _G_fpuopVfpNone.SFPU_pfuncRestore    = sparcVfpNoneRestore;
    _G_fpuopVfpNone.SFPU_pfuncCtxShow    = sparcVfpNoneCtxShow;
    _G_fpuopVfpNone.SFPU_pfuncEnableTask = sparcVfpNoneEnableTask;

    return  (&_G_fpuopVfpNone);
}
/*********************************************************************************************************
** 函数名称: sparcVfpNoneSecondaryInit
** 功能描述: 初始化 VFP 控制器
** 输　入  : pcMachineName 机器名
**           pcFpuName     浮点运算器名
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  sparcVfpNoneSecondaryInit (CPCHAR  pcMachineName, CPCHAR  pcFpuName)
{
    (VOID)pcMachineName;
    (VOID)pcFpuName;
}

#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
