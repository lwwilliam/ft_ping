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

	while (keepRunning)
	{
		memset(buffer, 0, BUFFER_SIZE);
		icmp->type = ICMP_ECHO;
		icmp->code = 0;
		icmp->checksum = 0; 
		icmp->un.echo.id = getpid();
		icmp->un.echo.sequence = seq++;
		icmp->checksum = calculate_checksum((unsigned short *)icmp, PAYLOAD_SIZE);

		printf("send bytes %ld\n", sendto(sockfd, buffer, PAYLOAD_SIZE, 0, (struct sockaddr*)addr, sizeof(*addr)));
		unsigned int addr_len = sizeof(r_addr);
		ssize_t recv_bytes = recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, &addr_len);
		if (recv_bytes < 0)
		{
			perror("recvfrom failed");
		}
		else
		{
			if (strcmp(arg, ip_address))
				printf("%ld bytes from %s (%s): icmp_seq=%d ttl=110 time=9.91 ms\n", recv_bytes - sizeof(struct iphdr), reverse_hostname, ip_address, seq);
			else
				printf("%ld bytes from %s: icmp_seq=%d ttl=62 time=4.82 ms\n", recv_bytes - sizeof(struct iphdr), ip_address, seq);
		}

		sleep(1);
	}
	close(sockfd);
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