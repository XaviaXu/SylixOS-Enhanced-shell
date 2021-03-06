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
** 文   件   名: dlfcn.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2011 年 02 月 20 日
**
** 描        述: posix 动态链接库兼容库.

** BUG:
2012.05.10  加入 dladdr api. (补丁里也有相应函数, 内核中调用仅对内核起效)
2013.01.15  相应 api 已经提供进程控制块接口, 不需要再有补丁参与.
2013.06.07  加入 dlrefresh() 用于更新正在运行的程序和动态链接库. 
            dlerror 直接从 strerror 获取错误字串.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../include/px_dlfcn.h"                                        /*  已包含操作系统头文件        */
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0 && LW_CFG_MODULELOADER_EN > 0
#include "../SylixOS/loader/include/loader_vppatch.h"
/*********************************************************************************************************
** 函数名称: dladdr
** 功能描述: 获得指定地址的符号信息
** 输　入  : pvAddr        地址
**           pdlinfo       地址对应的信息
** 输　出  : > 0 表示正确
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
int  dladdr (void *pvAddr, Dl_info *pdlinfo)
{
    PVOID   pvVProc = (PVOID)__LW_VP_GET_CUR_PROC();
    INT     iError;
    
    iError = API_ModuleAddr(pvAddr, (PVOID)pdlinfo, pvVProc);
    if (iError < 0) {
        return  (0);
    
    } else {
        return  (1);
    }
}
/*********************************************************************************************************
** 函数名称: dlclose
** 功能描述: 关闭指定句柄的动态链接库
** 输　入  : pvHandle      动态库句柄
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
int  dlclose (void  *pvHandle)
{
    INT     iError;
    PVOID   pvVProc = (PVOID)__LW_VP_GET_CUR_PROC();

    if (pvHandle == pvVProc) {
        iError = ERROR_NONE;

    } else {
        iError = API_ModuleUnload(pvHandle);
    }

    /*
     *  这里使用 loader 中的错误检查机制, 以确保 dlerror() 的正确性.
     */
    if (iError >= 0) {
        errno = ERROR_NONE;
    }
     
    return  (iError);
}
/*********************************************************************************************************
** 函数名称: dlerror
** 功能描述: 当动态链接库操作函数执行失败时，dlerror可以返回出错信息，返回值为NULL时表示操作函数执行成功。
** 输　入  : NONE
** 输　出  : 出错信息
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
char  *dlerror (void)
{
    if (!errno) {
        return  (LW_NULL);
    }

    return  (lib_strerror(errno));
}
/*********************************************************************************************************
** 函数名称: dlopen
** 功能描述: 函数以指定模式打开指定的动态连接库文件，并返回一个句柄给调用程序.
             这里装载的动态链接库没有进程信息, 外部编译的进程将重定向此函数, 并提供进程信息!
             所以, 在内核代码中慎用 RTLD_GLOBAL 属性, 如果使用则此动态库将变成多进程共享的动态链接库.
** 输　入  : pcFile        动态链接库文件
**           iMode         打开方式
** 输　出  : 动态链接库句柄
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
void  *dlopen (const char *pcFile, int  iMode)
{
    PVOID   pvVProc = (PVOID)__LW_VP_GET_CUR_PROC();
    PVOID   pvHandle;
    INT     iLoaderMode;
    
    if (iMode & RTLD_GLOBAL) {
        iLoaderMode = LW_OPTION_LOADER_SYM_GLOBAL;
    } else {
        iLoaderMode = LW_OPTION_LOADER_SYM_LOCAL;
    }
    
    if (pcFile == LW_NULL) {                                            /*  返回进程句柄                */
        return  (pvVProc);
    }

    if (iMode & RTLD_NOLOAD) {
        pvHandle = API_ModuleGlobal(pcFile, iLoaderMode, pvVProc);
    
    } else {
        pvHandle = API_ModuleLoad(pcFile, iLoaderMode, LW_NULL, LW_NULL, pvVProc);
    }
    
    if (pvHandle) {
        errno = ERROR_NONE;
    }
    
    return  (pvHandle);
}
/*********************************************************************************************************
** 函数名称: dlsym
** 功能描述: 从装载的动态库中, 获取的函数的运行地址
** 输　入  : pvHandle      动态库句柄
**           pcName        需要查找的函数名
** 输　出  : 函数入口
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
void  *dlsym (void  *pvHandle, const char *pcName)
{
    PVOID   pvVProc = (PVOID)__LW_VP_GET_CUR_PROC();
    PVOID   pvFunc  = LW_NULL;

    if ((pvVProc == pvHandle) || (pvHandle == RTLD_DEFAULT)) {
        pvFunc = API_ModuleProcSym(pvVProc, LW_NULL, pcName);           /*  在整个进程空间查找符号      */

#ifdef __GNUC__
    } else if (pvHandle == RTLD_NEXT) {
        pvFunc = API_ModuleProcSym(pvVProc, __builtin_return_address(0), pcName);
#endif                                                                  /*  __GNUC__                    */

    } else {
        pvFunc = API_ModuleSym(pvHandle, pcName);
    }

    if (pvFunc) {
        errno = ERROR_NONE;
    }
    
    return  (pvFunc);
}
/*********************************************************************************************************
** 函数名称: dlrefresh
** 功能描述: 清除操作系统共享库信息, 主用用于更新正在运行的应用程序和动态链接库, 在更新动态库前, 必须运行
             此函数
** 输　入  : pcName        需要更新的动态库 (NULL 表示清除或者刷新所有)
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
int  dlrefresh (const char *pcName)
{
    return  (API_ModuleShareRefresh(pcName));
}

#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
                                                                        /*  LW_CFG_MODULELOADER_EN > 0  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
