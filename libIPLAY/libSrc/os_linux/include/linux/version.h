/*
 * =====================================================================================
 *
 *       Filename:  version.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2010/5/27 上午 11:50:58 台北標準時間
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  first_name last_name (fl), fl@my-company.com
 *        Company:  my-company
 *
 * =====================================================================================
 */

#ifndef _LINUX_VERSION_H
#define _LINUX_VERSION_H

#ifndef KERNEL_VERSION
  #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif
#define LINUX_VERSION_CODE  KERNEL_VERSION(2,6,23)

#endif /* _LINUX_VERSION_H */

