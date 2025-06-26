#include "ft_ping.h"

int keepRunning = 1;

void intHandler() {
	keepRunning = 0;
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

int init_socket(struct s_ping *ping_struct)
{
	struct timeval timeout;
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	ping_struct->ttl = TTL;
	timeout.tv_sec = 4;  // 4-second timeout
	timeout.tv_usec = 0;
	if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ping_struct->ttl, sizeof(ping_struct->ttl)) != 0)
	{
		perror("setsockopt(IP_TTL)");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
		printf("Request Timed out\n");
	if (ping_init(sockfd, ping_struct) == -1)
		return -1;
	return sockfd;
}

void icmp_err(int type, struct sockaddr_in r_addr, struct s_ping_vars *vars)
{
	char addr_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &r_addr.sin_addr, addr_str, sizeof(addr_str));
	printf("From %s icmp_seq=%d ", addr_str, vars->seq);
	vars->errors++;
	switch (type)
	{
		case ICMP_DEST_UNREACH:
			printf("Destination host unreachable\n");
			break;
		case ICMP_SOURCE_QUENCH:
			printf("Source quench\n");
			break;
		case ICMP_REDIRECT:
			printf("Redirect message\n");
			break;
		case ICMP_TIME_EXCEEDED:
			printf("Time to live exceeded\n");
			break;
		case ICMP_PARAMETERPROB:
			printf("Parameter problem\n");
			break;
		default:
			printf("Unknown ICMP type\n");
	}
}

void ping_recv(struct s_ping_vars *vars, struct s_ping *ping_struct, int sockfd, struct timeval tv1)
{
	struct timeval tv2;
	char rbuffer[BUFFER_SIZE];
	struct sockaddr_in r_addr;
	unsigned int addr_len = sizeof(r_addr);

	ssize_t recv_bytes = recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, &addr_len);
	if (recv_bytes > 0)
	{
		gettimeofday(&tv2, NULL);
		float time = (float)(tv2.tv_usec - tv1.tv_usec) / 1000 + (float)(tv2.tv_sec - tv1.tv_sec) * 1000;
		struct iphdr *ip_hdr = (struct iphdr *)rbuffer;
		int ip_header_len = ip_hdr->ihl * 4;
		struct icmphdr *icmp_hdr = (struct icmphdr *)(rbuffer + ip_header_len);

		if (icmp_hdr->type == ICMP_ECHOREPLY)
		{
			update_stats(vars, time);
			ping_print(ping_struct, recv_bytes, vars->seq, time, icmp_hdr, ip_hdr->ttl);
		}
		else
			icmp_err(icmp_hdr->type, r_addr, vars);
	}
	else
		recv_failed(vars->seq, ping_struct, vars);
}

void ping_funct(struct sockaddr_in *addr, struct s_ping *ping_struct)
{
	struct s_ping_vars vars = {0};
	struct timeval tv1;
	int sockfd = init_socket(ping_struct);
	if (sockfd == -1)
	{
		close(sockfd);
		return;
	}
	char buffer[PACKET_SIZE];
	struct icmphdr *icmp = (struct icmphdr *)buffer;

	gettimeofday(&vars.prog_start, NULL);
	while (keepRunning)
	{
		icmp_init(icmp, vars.seq++);
		gettimeofday(&tv1, NULL);
		sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)addr, sizeof(*addr));
		ping_recv(&vars, ping_struct, sockfd, tv1);
		sleep(1);
	}
	close(sockfd);
	print_stats(ping_struct, &vars, vars.rtt_times);
}