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

int	create_socket_recv()
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
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt error");
		close(sock);
		return -1;
	}

	return (sock);
}

int create_socket_send(void) {
	int sock;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sock < 0) {
		perror("Socket error");
		return -1;
	}
	int disable = 1;
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &disable, sizeof(disable)) < 0) {
		perror("setsockopt error");
		close(sock);
		return -1;
	}
	return sock;
}

ping_data *build_ping_data(ping *ping) {
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(ping->params.ip_addr_dest);
	size_t packet_size = sizeof(ping_data);
	ping_data *data = (ping_data *)malloc(packet_size);
	if (!data) {
		perror("Malloc failed");
		exit(1);
	}

	memset(data, 0, packet_size);
    data->ip_hdr.ip_hl = 5;
    data->ip_hdr.ip_v = 4;
    data->ip_hdr.ip_tos = 0;
    data->ip_hdr.ip_len = htons(packet_size);
    data->ip_hdr.ip_id = ping->params.id;
    data->ip_hdr.ip_off = 0;
    data->ip_hdr.ip_ttl = ping->params.ttl;
    data->ip_hdr.ip_p = IPPROTO_ICMP;
    data->ip_hdr.ip_src.s_addr = inet_addr(ping->params.ip_addr_src);
    data->ip_hdr.ip_dst.s_addr = dest_addr.sin_addr.s_addr;
    data->ip_hdr.ip_sum = 0;
    data->ip_hdr.ip_sum = checksum((uint16_t *)&data->ip_hdr, sizeof(data->ip_hdr));

	data->icmp_hdr.type = ICMP_ECHO;
	data->icmp_hdr.code = 0;
	data->icmp_hdr.un.echo.id = htons(ping->params.id);
	data->icmp_hdr.un.echo.sequence = 0;
	data->icmp_hdr.checksum = 0;
	data->icmp_hdr.checksum = checksum((uint16_t *)&data->icmp_hdr, sizeof(data->icmp_hdr));

	return data;
}

void update_ping_data(ping_data *data, ping *ping) {
	u_int16_t rand_id = rand() & 0xFFFF;
	data->ip_hdr.ip_id = rand_id;
	data->ip_hdr.ip_sum = 0;
	data->ip_hdr.ip_sum = checksum((uint16_t *)&data->ip_hdr, sizeof(data->ip_hdr));
	data->ip_hdr.ip_ttl = ping->params.ttl;

	data->icmp_hdr.un.echo.sequence = ping->params.seq;
	data->icmp_hdr.checksum = 0;
	data->icmp_hdr.checksum = checksum((uint16_t *)&data->icmp_hdr, sizeof(data->icmp_hdr));
}

int send_pings(ping *ping) {
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(ping->params.ip_addr_dest);
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

void	print_ping_delay(ping_pckt *pckt, int cpt) {
		printf(" %.2f ms", time_diff(pckt->sent_time, pckt->recv_time));
	if (cpt == 2)
		printf("\n");
}

int recv_pings(ping *ping) {
	char recv_buffer[4096];
	struct ip *ip_hdr;
	struct icmphdr *icmp_reply;
	struct sockaddr_in	src_addr;
	socklen_t		addr_len = sizeof(struct sockaddr_in);
	int cpt = 0;
	struct timeval tv;
	ssize_t recv_len;
	int type_reply;
	int ip_hdr_len = sizeof(struct ip);
	int icmp_hdr_len = sizeof(struct icmphdr);

	while (cpt < 3) {
		recv_len = recvfrom(ping->socks.recv, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&src_addr, &addr_len);
		if (recv_len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			printf(" *");
			if (cpt == 2)
			{
				printf("\n");
				return 0;
			}
			cpt++;
			continue;
		}
		if (recv_len <= 0) {
			free_ping(ping->pings);
			ping->pings = NULL;
			perror("Recvfrom failed");
			return 1;
		}
		ip_hdr = (struct ip *)recv_buffer;
		icmp_reply = (struct icmphdr *)(recv_buffer + ip_hdr_len);
		type_reply = icmp_reply->type;
		gettimeofday(&tv, NULL);
		if (cpt == 0)
			printf(" %s (%s)", inet_ntoa(ip_hdr->ip_src), inet_ntoa(ip_hdr->ip_src));
		if (type_reply == ICMP_TIME_EXCEEDED) {
			ip_hdr = (struct ip *)(recv_buffer + ip_hdr_len + icmp_hdr_len);
			icmp_reply = (struct icmphdr *)(recv_buffer + ip_hdr_len * 2 + icmp_hdr_len);
			ping_pckt *ping_target = find_ping(ping->pings, icmp_reply->un.echo.sequence);
			if (!ping_target){
				printf("No matching ping found\n");
				return 0;
			}
			ping_target->ip_addr = inet_ntoa(src_addr.sin_addr);
			ping_target->recv_time = tv;
			print_ping_delay(ping_target, cpt);
		}
		else if (type_reply == ICMP_ECHOREPLY) {
			ping_pckt *ping_target = find_ping(ping->pings, icmp_reply->un.echo.sequence);
			if (!ping_target){
				printf("No matching ping found\n");
				return 0;
			}
			ping_target->ip_addr = inet_ntoa(src_addr.sin_addr);
			ping_target->recv_time = tv;
			print_ping_delay(ping_target, cpt);
			if (cpt == 2)
				return 1;
		}
		cpt++;
	}
	return 0;
}

void print_first_line(ping *ping) {
	printf("traceroute to %s (%s), %d hops max, %ld byte packets\n", ping->params.raw_dest, ping->params.ip_addr_dest, ping->params.max_ttl, sizeof(ping_data));
}

int initial_setup_traceroute(ping *ping) {
	ping->socks.recv = create_socket_recv();
	if (ping->socks.recv < 0) {
		return 1;
	}
	ping->socks.send = create_socket_send();
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

void end_ping(ping *ping) {
	close(ping->socks.recv);
	ping->socks.recv = -1;
	close(ping->socks.send);
	ping->socks.send = -1;
	free(ping->params.data);
	ping->params.data = NULL;
	free_ping(ping->pings);
	ping->pings = NULL;
}

int cmd_traceroute(ping *ping) {
	while (g_run && ping->params.ttl <= ping->params.max_ttl) {
		printf("%d : ", ping->params.ttl);
		if (!g_run || send_pings(ping) != 0)
			return 0;
		if (recv_pings(ping) == 1)
			return 0;
		ping->params.ttl++;
	}
	return 1;
}
