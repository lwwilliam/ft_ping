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
#include <errno.h>
#include <math.h>

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef NI_NAMEREQD
#define NI_NAMEREQD 0x00000001
#endif

#define PORT_NO 0
#define BUFFER_SIZE 1024
#define PAYLOAD_SIZE 56
#define PACKET_SIZE (sizeof(struct icmphdr) + PAYLOAD_SIZE)
#define TTL 64

typedef struct s_ping
{
	int verbose;
	char *ip_addr;
	char *reverse_hostname;
	int ttl;
	char *ping_arg;
} t_ping;

typedef struct s_ping_vars {
	struct timeval prog_start;
	float total_time;
	float min;
	float max;
	int seq;
	int pkt_rec;
	float rtt_times[1024];
	int errors;
} t_ping_vars;

char *dns_lookup(char *addr_host, struct sockaddr_in *addr);
char *reverse_dns_lookup(char *ip_addr);
void ping_funct(struct sockaddr_in *addr, struct s_ping *ping_struct);
unsigned short calculate_checksum(unsigned short *addr, int count);
void print_stats(struct s_ping *ping_struct, struct s_ping_vars *vars, float *rtt_times);
void recv_failed(int seq, struct s_ping *ping_struct, struct s_ping_vars *vars);
void ping_print(struct s_ping *ping_struct, int recv_bytes, int seq, float time, struct icmphdr *icmp, int recv_ttl);
void update_stats(struct s_ping_vars *vars, float time);
int ping_init(int sockfd, struct s_ping *ping_struct);
void intHandler();

#endif