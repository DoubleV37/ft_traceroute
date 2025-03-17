#include "../ft_traceroute.h"

int	get_ip_with_hostname(char *hostname, char final_ip[INET_ADDRSTRLEN]) {
	struct addrinfo hints, *info, *p;
	int gai_result;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
		return 0;
	}

	for(p = info; p != NULL; p = p->ai_next) {
		void *addr;
		if (p->ai_family == AF_INET) {
			addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
			inet_ntop(p->ai_family, addr, final_ip, INET_ADDRSTRLEN);
			freeaddrinfo(info);
			return 1;
		}
	}
	freeaddrinfo(info);
	return 0;
}

char	*get_source_ip() {
	char *ip = NULL;
	struct ifaddrs *ifaddr, *ifa;
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return (NULL);
	}
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		if (ifa->ifa_addr->sa_family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
			struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
			ip = inet_ntoa(sa->sin_addr);
			freeifaddrs(ifaddr);
			return (ip);
		}
	}
	freeifaddrs(ifaddr);
	return (NULL);
}


int create_socket_send_udp(void) {
    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Socket error: %s\n", strerror(errno));
        return -1;
    }
    return sock;
}

int create_socket_recv_udp(int port) {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket error");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        close(sock);
        return -1;
    }
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt error");
        close(sock);
        return -1;
    }
    return sock;
}

int	create_socket_recv_icmp()
{
	int	sock;
	struct timeval timeout;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock)
	if (sock < 0)
	{
		perror("socket error");
		return (-1);
	}
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt error");
		close(sock);
		return -1;
	}

	return (sock);
}

void print_first_line(ping *ping) {
	printf("traceroute to %s (%s), %d hops max, %ld byte packets\n", ping->params.raw_dest, ping->params.ip_addr_dest, ping->params.max_ttl, sizeof(ping_data));
}

int initial_setup_traceroute(ping *ping) {
	if (ping->params.type_traceroute == 1) {
		ping->socks.recv = create_socket_recv_icmp();
		if (ping->socks.recv < 0)
			return 1;
		ping->socks.send = create_socket_send_icmp();
	}
	else {
		ping->socks.recv = create_socket_recv_udp(33434);
		if (ping->socks.recv < 0)
			return 1;
		ping->socks.send = create_socket_send_udp();
	}
	if (ping->socks.send < 0) {
		close(ping->socks.recv);
		return 1;
	}
	ping->params.id = getpid() & 0xFFFF;
	ping->params.seq = 0;
	ping->params.ip_addr_src = get_source_ip();
	if (!ping->params.ip_addr_src) {
		perror("Failed to get source ip");
		return 1;
	}
	ping->params.data = build_ping_data(ping);

	print_first_line(ping);
	return 0;
}

int send_udp_pckt(ping *ping) {
	setsockopt(ping->socks.send, IPPROTO_IP, IP_TTL, &ping->params.ttl, sizeof(int));
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(ping->params.ip_addr_dest);
	dest_addr.sin_port = htons(33434);
	size_t packet_size = sizeof(ping_data);
	int cpt = 0;
	while (cpt < 3) {
		update_ping_data(ping->params.data, ping);
		if (sendto(ping->socks.send, ping->params.data, packet_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
			perror("Send failed");
			return 1;
		}
		ping->pings = add_ping(ping->pings, ping->params.seq);
		ping->params.seq++;
		cpt++;
	}
	return 0;
}

int cmd_traceroute(ping *ping) {
	while (g_run && ping->params.ttl <= ping->params.max_ttl) {
		printf("%d : ", ping->params.ttl);
		if (ping->params.type_traceroute == 0) {
			if (send_udp_pckt(ping) != 0)
				return 0;
		}
		else if (ping->params.type_traceroute == 1) {
			if (!g_run || send_pings(ping) != 0)
				return 0;
		}
		if (recv_pings(ping) == 1)
			return 0;
		ping->params.ttl++;
	}
	return 1;
}
