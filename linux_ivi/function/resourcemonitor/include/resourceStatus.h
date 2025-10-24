#ifndef __RESOURCESTATUS_H
#define __RESOURCESTATUS_H


/*
*	function:getRAMTotalSize
*	input   :void
*	output  :unsigned long long
*	decla	:获取RAM总大小，单位b
*/
extern long long getRAMTotalSize();

/*
*	function:getRAMFreeSize
*	input   :void
*	output  :int
*	decla	:获取可用RAM大小，单位MB
*/

extern long long getRAMFreeSize();

/*
*	function:getRAMUsage
*	input   :void
*	output  :int
*	decla	:获取RAM利用率的百分比，返回int类型，如使用率是45%，则返回45
*/

extern int getRAMUsage();
/*
*	function:getCPUUsage
*	input   :void
*	output  :int
*	decla	:获取cpu利用率的百分比，返回int类型，如使用率是45.6%，则返回46
*/
extern int getCPUUsage();

/*
*	function:getROMTotalSize
*	input   :void
*	output  :long long 
*	decla	:获取ROM总大小,单位b
*/
extern long long getROMTotalSize();

/*
*	function:getROMUsage
*	input   :void
*	output  :int
*	decla	:获取ROM利用率的百分比，返回int类型，如使用率是45.6%，则返回46
*/
extern int  getROMUsage();
/*
*	function:getROMFreeSize
*	input   :void
*	output  :int
*	decla	:获取ROM可用空间大小，单位MB
*/
extern long long  getROMFreeSize();

/*
*	function:getCpuCoreNum
*	input   :void
*	output  :int
*	decla	:获取cpu内核数量
*/
extern int getCpuCoreNum();

#endif 
