#ifndef FT_PING
# define FT_PING

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef NI_NAMEREQD
#define NI_NAMEREQD 0x00000001
#endif

#define PORT_NO 0
#define BUFFER_SIZE 1024
#define PAYLOAD_SIZE 64

char *dns_lookup(char *addr_host, struct sockaddr_in *addr);
char *reverse_dns_lookup(char *ip_addr);

#endif