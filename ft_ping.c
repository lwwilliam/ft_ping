#include "ft_ping.h"

int keepRunning = 1;

void intHandler() {
	keepRunning = 0;
}

void ping_error(int sockfd, struct s_ping *ping_struct)
{
	if (sockfd < 0)
		printf("file descriptor for the new socket failed creating\n");
	if (ping_struct->verbose == 1)
	{
		printf("ping: sock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_UNSPEC\n\n", sockfd);
		printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n", ping_struct->ping_arg);
	}
	printf("PING %s (%s) 56(84) bytes of data.\n", ping_struct->ping_arg, ping_struct->ip_addr);

	signal(SIGINT, intHandler);

	if (setsockopt(sockfd, SOL_SOCKET, IP_TTL, &ping_struct->ttl, sizeof(ping_struct->ttl)))
		printf("ttl set error");
}

void icmp_init(struct icmphdr *icmp, int seq)
{
	memset(icmp, 0, PACKET_SIZE);
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = getpid();
	icmp->un.echo.sequence = seq;
	icmp->checksum = calculate_checksum((unsigned short *)icmp, PACKET_SIZE);
}

int init_socket(struct s_ping *ping_struct, struct timeval *timeout)
{
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	ping_struct->ttl = 64;
	timeout->tv_sec = 4;  // 4-second timeout
	timeout->tv_usec = 0;

	ping_error(sockfd, ping_struct);
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0)
		printf("Request Timed out\n");

	return sockfd;
}

void ping_funct(struct sockaddr_in *addr, struct s_ping *ping_struct)
{
	struct s_ping_vars vars = {0};
	struct timeval tv1, tv2;
	float rtt_times[1024];
	struct timeval timeout;
	int sockfd = init_socket(ping_struct, &timeout);
	char buffer[PACKET_SIZE];
	char rbuffer[BUFFER_SIZE];
	struct icmphdr *icmp = (struct icmphdr *)buffer;
	struct sockaddr_in r_addr;

	gettimeofday(&vars.prog_start, NULL);
	while (keepRunning)
	{
		icmp_init(icmp, vars.seq++);
		gettimeofday(&tv1, NULL);
		sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)addr, sizeof(*addr));
		unsigned int addr_len = sizeof(r_addr);
		ssize_t recv_bytes = recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, &addr_len);
		if (recv_bytes > 0)
		{
			vars.pkt_rec++;
			gettimeofday(&tv2, NULL);
			float time = (float)(tv2.tv_usec - tv1.tv_usec) / 1000 + (float)(tv2.tv_sec - tv1.tv_sec) * 1000;
			ping_print(ping_struct, recv_bytes, vars.seq, time, icmp);
			rtt_times[vars.pkt_rec - 1] = time;
			vars.min = time;
			vars.min = time < vars.min ? time : vars.min;
			vars.max = time > vars.max ? time : vars.max;
			vars.total_time += time;
		}
		else
			recv_failed(vars.seq, ping_struct);
		sleep(1);
	}
	close(sockfd);
	print_stats(ping_struct, &vars, rtt_times);
}