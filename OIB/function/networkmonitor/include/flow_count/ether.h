/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 2021-01-21 22:38:49
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-01-21 22:59:36
 */
#ifndef __ETHER_H_
#define __ETHER_H_

#define	ETHERTYPE_PUP		0x0200   
#define	ETHERTYPE_IP		0x0800
#define	ETHERTYPE_ARP		0x0806
#define	ETHERTYPE_REVARP	0x8035

#define	ETHER_ADDR_LEN		6

struct	ether_header {
	unsigned char	ether_dhost[ETHER_ADDR_LEN];
	unsigned char	ether_shost[ETHER_ADDR_LEN];
	unsigned short	ether_type;
};

struct vlan_8021q_header {
	unsigned short	priority_cfi_vid;
	unsigned short	ether_type;
};

#endif 
