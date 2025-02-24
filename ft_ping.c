#include "ft_ping.h"

int keepRunning = 1;

void intHandler(int) {
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

void ping_print(struct s_ping *ping_struct, int recv_bytes, int seq, float time, struct icmphdr *icmp)
{
	if (ping_struct->verbose == 1)
	{
		if (strcmp(ping_struct->ping_arg, ping_struct->ip_addr))
			printf("%ld bytes from %s (%s): icmp_seq=%d ident=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->reverse_hostname, ping_struct->ip_addr, seq, icmp->un.echo.id, ping_struct->ttl, time);
		else
			printf("%ld bytes from %s: icmp_seq=%d ident=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->ip_addr, seq, icmp->un.echo.id, ping_struct->ttl, time);
	}
	else
	{
		if (strcmp(ping_struct->ping_arg, ping_struct->ip_addr))
			printf("%ld bytes from %s (%s): icmp_seq=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->reverse_hostname, ping_struct->ip_addr, seq, ping_struct->ttl, time);
		else
			printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->ip_addr, seq, ping_struct->ttl, time);
	}
}

void icmp_init(struct icmphdr *icmp, int seq)
{
    memset(icmp, 0, sizeof(struct icmphdr));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0; 
    icmp->un.echo.id = getpid();
    icmp->un.echo.sequence = seq;
    icmp->checksum = calculate_checksum((unsigned short *)icmp, sizeof(struct icmphdr));
}

int init_socket(struct s_ping *ping_struct, struct timeval *timeout)
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
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
	int seq = 0;
	int pkt_rec = 0;
	struct timeval tv1, tv2, prog_start, prog_end;
	struct timeval timeout;
	int sockfd = init_socket(ping_struct, &timeout);
	char buffer[BUFFER_SIZE];
	char rbuffer[BUFFER_SIZE];
	struct icmphdr *icmp = (struct icmphdr *)buffer;
	struct sockaddr_in r_addr;
	float min = 0;
	float total_time = 0;
	float max = 0;
	float rtt_times[1024];

	gettimeofday(&prog_start, NULL);
	while (keepRunning)
	{
		icmp_init(icmp, seq++);
		gettimeofday(&tv1, NULL);
		sendto(sockfd, buffer, PAYLOAD_SIZE, 0, (struct sockaddr*)addr, sizeof(*addr));
		unsigned int addr_len = sizeof(r_addr);
		ssize_t recv_bytes = recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, &addr_len);
		if (recv_bytes > 0)
		{
			pkt_rec++;
			gettimeofday(&tv2, NULL);
			float time = (float)(tv2.tv_usec - tv1.tv_usec) / 1000 + (float)(tv2.tv_sec - tv1.tv_sec) * 1000;
			rtt_times[pkt_rec - 1] = time;
			ping_print(ping_struct, recv_bytes, seq, time, icmp);
			min = time;
			min = time < min ? time : min;
			max = time > max ? time : max;
			total_time += time;
		}
		else
        {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				printf("Request Timeout for icmp_seq=%d\n", seq);
			else
				printf("From %s icmp_seq=%d Destination Host Unreachable\n", ping_struct->ip_addr, seq);
        }
		sleep(1);
	}
	close(sockfd);
	printf("\n--- %s ping statistics ---\n", ping_struct->ping_arg);
	float loss = ((float)(seq - pkt_rec) / seq) * 100;
	gettimeofday(&prog_end, NULL);
	float prog_time = (float)(prog_end.tv_usec - prog_start.tv_usec) / 1000 + (float)(prog_end.tv_sec - prog_start.tv_sec) * 1000;
	printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.0fms\n", seq, pkt_rec, loss, prog_time);
	if (pkt_rec > 0)
	{
    	float avg = pkt_rec > 0 ? total_time / pkt_rec : 0;
		float mdev = 0;
		for (int i = 0; i < pkt_rec; i++)
			mdev += fabs(rtt_times[i] - avg);
		mdev /= pkt_rec;
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", min, avg, max, mdev);
	}
}