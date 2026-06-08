#pragma once

#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include "macros.h"

// Defines some macros
#define ADDRSOCK_STR(a, b, c) addrsock_str(a, b, c, sizeof(c))

// For addrinfo
#define FLAG 0x1
#define FAMILY 0x2
#define SOCKTYPE 0x4
#define PROTOCOL 0x8

// For sockaddr_in* 
#define _FAMILY 0x10
#define _PORT 0x20
#define _ADDR 0x40

// For flags member of addrinfo
const static int addrinfo_flags[] = {
	// Common
	AI_V4MAPPED, AI_ADDRCONFIG, AI_NUMERICHOST,
	AI_PASSIVE, AI_CANONNAME, AI_ALL, AI_NUMERICSERV,
};

const static char *addrinfo_flags_str[] = {
	// Common
	"AI_V4MAPPED", "AI_ADDRCONFIG", "AI_NUMERICHOST",
	"AI_PASSIVE", "AI_CANONNAME", "AI_ALL", "AI_NUMERICSERV",
};

// For family member of addrinfo
const static int addrinfo_families[] = {
	AF_INET, AF_INET6, AF_UNSPEC, AF_UNIX, AF_LOCAL, AF_AX25,
	AF_NETROM, AF_BRIDGE, AF_ATMPVC, AF_ROSE, AF_NETBEUI, AF_SECURITY,
	AF_ECONET, AF_ATMSVC, AF_IRDA, AF_WANPIPE, AF_IUCV, AF_RXRPC, 
	AF_ISDN, AF_PHONET, AF_IEEE802154, AF_CAIF, AF_QIPCRTR, AF_SMC, AF_MCTP, 
	AF_IPX, AF_APPLETALK, AF_X25, AF_DECnet, AF_KEY, AF_NETLINK, 
	AF_PACKET, AF_RDS, AF_PPPOX, AF_LLC, AF_IB, AF_MPLS, AF_CAN, 
	AF_TIPC, AF_BLUETOOTH, AF_ALG, AF_VSOCK, AF_KCM, AF_XDP
};

const static char *addrinfo_families_str[] = {
	"AF_INET", "AF_INET6", "AF_UNSPEC", "AF_UNIX", "AF_LOCAL", "AF_AX25",
	"AF_NETROM", "AF_BRIDGE", "AF_ATMPVC", "AF_ROSE", "AF_NETBEUI", "AF_SECURITY",
	"AF_ECONET", "AF_ATMSVC", "AF_IRDA", "AF_WANPIPE", "AF_IUCV", "AF_RXRPC", 
	"AF_ISDN", "AF_PHONET", "AF_IEEE802154", "AF_CAIF", "AF_QIPCRTR", "AF_SMC", "AF_MCTP", 
	"AF_IPX", "AF_APPLETALK", "AF_X25", "AF_DECnet", "AF_KEY", "AF_NETLINK", 
	"AF_PACKET", "AF_RDS", "AF_PPPOX", "AF_LLC", "AF_IB", "AF_MPLS", "AF_CAN", 
	"AF_TIPC", "AF_BLUETOOTH", "AF_ALG", "AF_VSOCK", "AF_KCM", "AF_XDP"
};

// For socktype member of addrinfo
const static int addrinfo_socktypes[] = {
	SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET, SOCK_RAW,
	SOCK_RDM, SOCK_PACKET, SOCK_NONBLOCK, SOCK_CLOEXEC
};

const static char *addrinfo_socktypes_str[] = {
	"SOCK_STREAM", "SOCK_DGRAM", "SOCK_SEQPACKET", "SOCK_RAW",
	"SOCK_RDM", "SOCK_PACKET", "SOCK_NONBLOCK", "SOCK_CLOEXEC"
};

int addrsock_str(int type, int data, char *buffer, size_t buffer_size) {
	char temp[512] = {0}; // Up to 4 Megabytes to store the represent string data
	
	// Handle only addrinfo FLAGS
	if(type & FLAG) {
		int size = MIN(GET_SIZE(addrinfo_flags), GET_SIZE(addrinfo_flags_str));

		for(int i = 0; i < size; i++) {
			if(data & addrinfo_flags[i]) {
				strcat(temp, addrinfo_flags_str[i]);
				strcat(temp, "|");
			}
		}
	}
	
	// Handle Protocol only 
	else if(type & PROTOCOL) {
		struct protoent *protocol = getprotobynumber(data);
		
		strcat(temp, protocol->p_name);
	}
	
	// Handle _FAMILY and FAMILY
	else if(type & (_FAMILY | FAMILY)) {
		int size = MIN(GET_SIZE(addrinfo_families), GET_SIZE(addrinfo_families_str));

		for(int i = 0; i < size; i++) {
			if(data == addrinfo_families[i]) {
				strcpy(temp, addrinfo_families_str[i]);
				break;
			}
		}
	}

	// Handle SOCKTYPE
	else if(type & SOCKTYPE) {
		int size = MIN(GET_SIZE(addrinfo_socktypes), GET_SIZE(addrinfo_socktypes_str));

		for(int i = 0; i < size; i++) {
			if(data == addrinfo_socktypes[i]) {
				strcpy(temp, addrinfo_socktypes_str[i]);
				break;
			}
		}
	}

	// Handle _PORT
	else if(type & _PORT) {
		sprintf(temp, "%d", ntohs(data));
	}

	// Handle _ADDR
	else if(type & _ADDR) {
		struct in_addr addr = { .s_addr = data };
		
		if(inet_ntop(AF_INET, &addr, temp, sizeof(temp)) != temp)
			return 1;
	}
	
	// None one of them
	else {
		return 1;
	}
	
	// Removed the last '|' charachter
	int last_char = strlen(temp) - 1;
	if(temp[last_char] == '|')
		temp[last_char] = 0;
		
	strncpy(buffer, temp, buffer_size);
	return 0;
}

int send_data(int clientfd, void *data, size_t size, int flags) {
	int bytes = 0, bytes_before = 0;

	while(bytes < size) {
		if(bytes == -1)
			return -1;

		bytes = send(clientfd, data, size, flags);

		if(bytes == 0)
			break;
		
		bytes_before = bytes;
	}

	return bytes_before;
}
