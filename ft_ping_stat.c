#include "ft_ping.h"

void ping_print(struct s_ping *ping_struct, int recv_bytes, int seq, float time, struct icmphdr *icmp, int recv_ttl)
{
	if (ping_struct->verbose == 1)
	{
		if (strcmp(ping_struct->ping_arg, ping_struct->ip_addr))
			printf("%ld bytes from %s (%s): icmp_seq=%d ident=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->reverse_hostname, ping_struct->ip_addr, seq, icmp->un.echo.id, recv_ttl, time);
		else
			printf("%ld bytes from %s: icmp_seq=%d ident=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->ip_addr, seq, icmp->un.echo.id, recv_ttl, time);
	}
	else
	{
		if (strcmp(ping_struct->ping_arg, ping_struct->ip_addr))
			printf("%ld bytes from %s (%s): icmp_seq=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->reverse_hostname, ping_struct->ip_addr, seq, recv_ttl, time);
		else
			printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ping_struct->ip_addr, seq, recv_ttl, time);
	}
}

void recv_failed(int seq, struct s_ping *ping_struct, struct s_ping_vars *vars)
{
	if (errno == EAGAIN || errno == EWOULDBLOCK)
		printf("Request Timeout for icmp_seq=%d\n", seq);
	else
		printf("From %s icmp_seq=%d Destination Host Unreachable\n", ping_struct->ip_addr, seq);
	vars->errors++;
}

void print_stats(struct s_ping *ping_struct, struct s_ping_vars *vars, float *rtt_times)
{
	struct timeval prog_end;

	printf("\n--- %s ping statistics ---\n", ping_struct->ping_arg);
	float loss = ((float)(vars->seq - vars->pkt_rec) / vars->seq) * 100;
	gettimeofday(&prog_end, NULL);
	float prog_time = (float)(prog_end.tv_usec - vars->prog_start.tv_usec) / 1000 + (float)(prog_end.tv_sec - vars->prog_start.tv_sec) * 1000;
	if (vars->errors > 0)
		printf("%d packets transmitted, %d received, +%d errors, %.2f%% packet loss, time %.0fms\n", vars->seq, vars->pkt_rec, vars->errors, loss, prog_time);
	else
		printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.0fms\n", vars->seq, vars->pkt_rec, loss, prog_time);
	if (vars->pkt_rec > 0)
	{
		float avg = vars->pkt_rec > 0 ? vars->total_time / vars->pkt_rec : 0;
		float mdev = 0;
		for (int i = 0; i < vars->pkt_rec; i++)
			mdev += fabs(rtt_times[i] - avg);
		mdev /= vars->pkt_rec;
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", vars->min, avg, vars->max, mdev);
	}
}

void update_stats(struct s_ping_vars *vars, float time)
{
	vars->pkt_rec++;
	vars->rtt_times[vars->pkt_rec - 1] = time;
	vars->min = time;
	vars->min = time < vars->min ? time : vars->min;
	vars->max = time > vars->max ? time : vars->max;
	vars->total_time += time;
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