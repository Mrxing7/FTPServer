/***************************************************************************************************
* 版权信息:
* 文件名称: basic_type.h
* 摘要: 定义基本的数据类型
* 
* 版本: 0.1.0
* 日期: 2023-03-31
* 备注: 创建并完成草案
***************************************************************************************************/

#ifndef _HKA_BASIC_TYPE_H_
#define _HKA_BASIC_TYPE_H_

/***************************************************************************************************
* 基本数据类型定义 
***************************************************************************************************/
typedef signed char     HKA_S08;
typedef unsigned char   HKA_U08;
typedef signed short    HKA_S16;
typedef unsigned short  HKA_U16;
typedef signed int      HKA_S32;
typedef unsigned int    HKA_U32;
typedef float           HKA_F32;
typedef double          HKA_F64;

typedef const signed char       HKA_CON_S08;
typedef const unsigned char     HKA_CON_U08;
typedef const signed short      HKA_CON_S16;
typedef const unsigned short    HKA_CON_U16;
typedef const signed int        HKA_CON_S32;
typedef const unsigned int      HKA_CON_U32;
typedef const float             HKA_CON_F32;
typedef const double            HKA_CON_F64;

#ifndef HKA_VOID
#define HKA_VOID    void
#endif

#ifndef HKA_BOOL
#define HKA_BOOL    bool
#endif

/***************************************************************************************************
* 空指针、true、false宏定义
***************************************************************************************************/
#ifndef HKA_NULL
#define HKA_NULL    0
#endif

#ifndef HKA_TRUE
#define HKA_TRUE    1
#endif

#ifndef HKA_FALSE
#define HKA_FALSE   0
#endif

// 状态码数据类型
typedef int HKA_STATUS;     // 函数返回值都定义为该类型

// 函数返回状态类型
// 服务器自定义状态类型，自定义状态类型值 < -1000
typedef enum _HKA_STATUS_CODE
{
    HKA_STS_ERR     = -1,           // 处理错误
    HKA_STS_OK      = 0,            // 处理正确

}_HKA_STATUS_CODE;

#endif // end of _HKA_BASIC_TYPE_H_