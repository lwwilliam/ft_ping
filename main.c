#include "ft_ping.h"

// int keepRunning = 1;

unsigned short calculate_checksum(unsigned short *addr, int count)
{
	unsigned long sum = 0;

	while (count > 1)
	{
		sum += *addr++;
		count -= 2;
	}
	if (count > 0)
		sum += *(unsigned char *)addr;
	while (sum >> 16) // Checks if sum has overflowed beyond 16 bits 
		sum = (sum & 0xFFFF) + (sum >> 16); // Adds the upper 16 bits of sum (sum >> 16) to the lower 16 bits (sum & 0xFFFF).
	return ~sum; // Bitwise not conversion
	// Converts the result into the 1's complement representation
	// https://www.tutorialspoint.com/one-s-complement
}
//http://www.faqs.org/rfcs/rfc1071.html in 4.1

void help()
{
	printf("\n");
	printf("Usage\n  ping [options] <destination>\n\n");
	printf("Options:\n");
	printf("  <destination>      DNS name or IP address\n");
	printf("  -v                 verbose output\n");
}

int parse(int ac, char **av, struct s_ping *ping_struct)
{
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
			ping_struct->verbose = 1;
		if (av[i][0] != '-')
			ping_struct->ping_arg = av[i];
	}
	return (0);
}

int main (int ac, char **av)
{
	struct s_ping ping_struct;
	ping_struct.verbose = 0;
	if (parse(ac, av, &ping_struct) == 1)
		return (1);
	struct sockaddr_in addr;
	ping_struct.ip_addr = dns_lookup(ping_struct.ping_arg, &addr);
	if (ping_struct.ip_addr == NULL)
	{
		free(ping_struct.ip_addr);
		return 0;
	}
	ping_struct.reverse_hostname = reverse_dns_lookup(ping_struct.ip_addr);
	ping_funct(&addr, &ping_struct);
	free(ping_struct.ip_addr);
	free(ping_struct.reverse_hostname);
	return (0);
}