#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#define PORT_NO 0

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef NI_NAMEREQD
#define NI_NAMEREQD 0x00000001
#endif