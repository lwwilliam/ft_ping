#include "ft_ping.h"

char *dns_lookup(char *addr_host, struct sockaddr_in *addr)
{
    struct hostent *host_entity;
    char *ip = (char *)malloc(NI_MAXHOST * sizeof(char));
    if ((host_entity = gethostbyname(addr_host)) == NULL)
	{
		free(ip);
        return NULL;
    }
    strcpy(ip, inet_ntoa(*(struct in_addr *)host_entity->h_addr_list[0]));
    (*addr).sin_family = host_entity->h_addrtype;
    (*addr).sin_port = htons(PORT_NO);
    (*addr).sin_addr.s_addr = *(long *)host_entity->h_addr_list[0];

    return ip;
}

char *reverse_dns_lookup(char *ip_addr) {
    struct sockaddr_in temp_addr;
    socklen_t len;
    char buf[NI_MAXHOST], *ret_buf;

    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
    len = sizeof(struct sockaddr_in);

    if (getnameinfo((struct sockaddr *)&temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD))
	{
        printf("Could not resolve reverse lookup of hostname\n");
        return NULL;
    }

    ret_buf = (char *)malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(ret_buf, buf);
    return ret_buf;
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
	printf("PING %s (%s) xx(xx) bytes of data\n", av[1], ip_addr);


	free(ip_addr);
	free(reverse_hostname);
	return (0);
}