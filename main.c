#include "ft_ping.h"

int keepRunning = 1;

unsigned short calculate_checksum(unsigned short *addr, int length)
{
	unsigned long sum = 0;

	while (length > 1) //processes the input data in 16-bit (2-byte) chunks
	{
		sum += *addr++;
		length -= 2;
	}
	if (length == 1)
		sum += *(unsigned char *)addr;
	while (sum >> 16) // Checks if sum has overflowed beyond 16 bits 
		sum = (sum & 0xFFFF) + (sum >> 16); // Adds the upper 16 bits of sum (sum >> 16) to the lower 16 bits (sum & 0xFFFF).
	return ~sum; // Bitwise not conversion
	// Converts the result into the 1's complement representation
	// https://www.tutorialspoint.com/one-s-complement
}
//http://www.faqs.org/rfcs/rfc1071.html in 4.1

void intHandler(int) {
	keepRunning = 0;
}

void ping_funct(char *reverse_hostname, struct sockaddr_in *addr, char *arg, char *ip_address, int verbose)
{
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	int seq = 0;
	int pkt_rec = 0;
	int ttl = 64;
	struct timeval  tv1, tv2, prog_start, prog_end;

	if (sockfd < 0)
		printf("file descriptor for the new socket failed creating\n");
	if (verbose == 1)
	{
		printf("ping: sock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_UNSPEC\n\n", sockfd);
		printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n", arg);
	}
	printf("PING %s (%s) 56(84) bytes of data.\n", arg, ip_address);

	signal(SIGINT, intHandler);

	if (setsockopt(sockfd, SOL_SOCKET, IP_TTL, &ttl, sizeof(ttl)))
		printf("ttl set error");

	struct timeval timeout;
	timeout.tv_sec = 1;  // 1-second timeout
	timeout.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
		printf("timeout set error");


	char buffer[BUFFER_SIZE];
	char rbuffer[BUFFER_SIZE];
	struct icmphdr *icmp = (struct icmphdr *)buffer;
	struct sockaddr_in r_addr;

	gettimeofday(&prog_start, NULL);
	while (keepRunning)
	{
		memset(buffer, 0, BUFFER_SIZE);
		icmp->type = ICMP_ECHO;
		icmp->code = 0;
		icmp->checksum = 0; 
		icmp->un.echo.id = getpid();
		icmp->un.echo.sequence = seq++;
		icmp->checksum = calculate_checksum((unsigned short *)icmp, PAYLOAD_SIZE);

		gettimeofday(&tv1, NULL);
		sendto(sockfd, buffer, PAYLOAD_SIZE, 0, (struct sockaddr*)addr, sizeof(*addr));
		unsigned int addr_len = sizeof(r_addr);
		ssize_t recv_bytes = recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, &addr_len);
		if (recv_bytes > 0)
		{
			pkt_rec++;
			gettimeofday(&tv2, NULL);
			float time = (float)(tv2.tv_usec - tv1.tv_usec) / 1000 + (float)(tv2.tv_sec - tv1.tv_sec) * 1000;
			if (verbose == 1)
			{
				if (strcmp(arg, ip_address))
					printf("%ld bytes from %s (%s): icmp_seq=%d ident=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), reverse_hostname, ip_address, seq, icmp->un.echo.id,ttl, time);
				else
					printf("%ld bytes from %s: icmp_seq=%d ident=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ip_address, seq, icmp->un.echo.id, ttl, time);
			}
			else
			{
				if (strcmp(arg, ip_address))
					printf("%ld bytes from %s (%s): icmp_seq=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), reverse_hostname, ip_address, seq, ttl, time);
				else
					printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", recv_bytes - sizeof(struct iphdr), ip_address, seq, ttl, time);
			}
		}

		sleep(1);
	}
	close(sockfd);
	printf("\n--- %s ping statistics ---\n", arg);
	float loss = ((float)(seq - pkt_rec) / seq) * 100;
	gettimeofday(&prog_end, NULL);
	float prog_time = (float)(prog_end.tv_usec - prog_start.tv_usec) / 1000 + (float)(prog_end.tv_sec - prog_start.tv_sec) * 1000;
	printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.0fms\n", seq, pkt_rec, loss, prog_time);
}

void help()
{
	printf("\n");
	printf("Usage\n  ping [options] <destination>\n\n");
	printf("Options:\n");
	printf("  <destination>      DNS name or IP address\n");
	printf("  -v                 verbose output\n");
}

int main (int ac, char **av)
{
	int verbose = 0;
	char *dest;
	if (ac < 2)
	{
		printf("%s: usage error: Destination address required\n", av[0]);
		return (1);
	}
	for (int i = 0; av[i]; i++)
	{
		if (strncmp(av[i], "-?", 3) == 0)
			help();
		if (strncmp(av[i], "-v", 3) == 0)
			verbose = 1;
		if (av[i][0] != '-')
			dest = av[i];
	}
	struct sockaddr_in addr;
	char *ip_addr, *reverse_hostname;
	ip_addr = dns_lookup(dest, &addr);
	if (ip_addr == NULL)
		return 0;
	reverse_hostname = reverse_dns_lookup(ip_addr);
	ping_funct(reverse_hostname, &addr, dest, ip_addr, verbose);
	free(ip_addr);
	free(reverse_hostname);
	return (0);
}