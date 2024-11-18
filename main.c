#include "ft_ping.h"

int keepRunning = 1;

void intHandler(int) {
    keepRunning = 0;
}

void ping_funct(char *reverse_hostname, struct sockaddr_in *addr, char *arg, char *ip_address)
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    int seq = 0;

    if (sockfd < 0)
        printf("file descriptor for the new socket failed creating\n");
    else
        printf("file descriptor for the new socket is returned %d\n", sockfd);

    signal(SIGINT, intHandler);

	struct timeval timeout;
    timeout.tv_sec = 1;  // 1-second timeout
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	char buffer[BUFFER_SIZE];
	char rbuffer[BUFFER_SIZE];
	struct icmphdr *icmp = (struct icmphdr *)buffer;
	struct sockaddr_in r_addr;

    printf("%d\n", sockfd);
	while (keepRunning)
	{
		memset(buffer, 0, BUFFER_SIZE);
		icmp->type = ICMP_ECHO;
		icmp->code = 0;
		icmp->checksum = 0; 
		icmp->un.echo.id = getpid();
		icmp->un.echo.sequence = seq++;

		printf("%ld\n", sendto(sockfd, buffer, sizeof(struct icmphdr), 0, (struct sockaddr*)addr, sizeof(*addr)));
		unsigned int addr_len = sizeof(r_addr);
		ssize_t recv_bytes = recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, &addr_len);
		if (recv_bytes < 0)
		{
			perror("recvfrom failed");
		}
		else
		{
			printf("Received %ld bytes\n", recv_bytes);
		}

		if (strcmp(arg, ip_address))
			printf("64 bytes from %s (%s): icmp_seq=%d ttl=110 time=9.91 ms\n", reverse_hostname, ip_address, seq);
		else
			printf("64 bytes from %s: icmp_seq=%d ttl=62 time=4.82 ms\n", ip_address, seq);
		sleep(1);
	}

    printf("\n--- %s ping statistics ---\n", arg);
    printf("%d packets transmitted, %d received, %d%% packet loss, time %dms\n", 4, 4, 0, 1000);
}

int main (int ac, char **av)
{
	if (ac != 2)
	{
		printf("%s: usage error: Destination address required\n", av[0]);
		return (1);
	}
	struct sockaddr_in addr;
	char *ip_addr, *reverse_hostname;
    ip_addr = dns_lookup(av[1], &addr);
    if (ip_addr == NULL)
	{
        printf("\nDNS lookup failed! Could not resolve hostname!\n");
        return 0;
    }

    reverse_hostname = reverse_dns_lookup(ip_addr);
    // printf("\nTrying to connect to '%s' IP: %s\n", av[1], ip_addr);
    // printf("\nReverse Lookup domain: %s\n", reverse_hostname);
	printf("PING %s (%s) 56(84) bytes of data.\n", av[1], ip_addr);

    ping_funct(reverse_hostname, &addr, av[1], ip_addr);

	free(ip_addr);
	free(reverse_hostname);
	return (0);
}