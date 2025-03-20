#include "../ft_traceroute.h"

int send_udp_pckt(ping *ping) {
	setsockopt(ping->socks.send, IPPROTO_IP, IP_TTL, &ping->params.ttl, sizeof(int));
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(ping->params.ip_addr_dest);
	dest_addr.sin_port = htons(ping->params.seq + 33434);
	size_t packet_size = sizeof(ping_data);
	int cnt = 0;
	while (cnt < 3) {
		update_ping_data(ping->params.data, ping);
		if (sendto(ping->socks.send, ping->params.data, packet_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
			perror("Send failed");
			return 1;
		}
		ping->pings = add_pckt(ping->pings, ping->params.seq + 33434);
		ping->params.seq++;
		cnt++;
	}
	return 0;
}

int recv_udp(ping *ping) {
	char recv_buffer[4096];
	struct ip *ip_hdr;
	struct icmphdr *icmp_reply;
	struct sockaddr_in	src_addr;
	struct udphdr *udp_hdr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int cnt = 0;
	struct timeval tv;
	ssize_t recv_len;
	int type_reply;
	char *last_ip = NULL;
	int ip_hdr_len = sizeof(struct ip);
	int icmp_hdr_len = sizeof(struct icmphdr);
	int port;

	while (cnt < 3) {
		if (ping->socks.recv < 0)
			return 1;
		recv_len = recvfrom(ping->socks.recv, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&src_addr, &addr_len);
		if (recv_len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			printf(" *");
			if (cnt == 2)
			{
				printf("\n");
				return 0;
			}
			cnt++;
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
		if (cnt == 0 || (last_ip != NULL && strcmp(last_ip, inet_ntoa(ip_hdr->ip_src)) != 0))
			printf(" %s (%s)", reverse_dns_lookup(inet_ntoa(ip_hdr->ip_src)), inet_ntoa(ip_hdr->ip_src));
		last_ip = inet_ntoa(ip_hdr->ip_src);
		if (type_reply == ICMP_TIME_EXCEEDED || type_reply == ICMP_DEST_UNREACH) {
			ip_hdr = (struct ip *)(recv_buffer + ip_hdr_len + icmp_hdr_len);
			udp_hdr = (struct udphdr *)(recv_buffer + ip_hdr_len + icmp_hdr_len + sizeof(struct ip));
			port = ntohs(udp_hdr->uh_dport);
			ping_pckt *ping_target = find_pckt(ping->pings, port);
			if (!ping_target){
				printf("No matching ping found\n");
				return 0;
			}
			ping_target->ip_addr = inet_ntoa(src_addr.sin_addr);
			ping_target->recv_time = tv;
			print_ping_delay(ping_target, cnt);
			if (cnt == 2 && type_reply == ICMP_DEST_UNREACH)
				return 1;
		}
		cnt++;
	}
	return 0;
}

int create_socket_send_udp(int *tos) {
    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Socket error: %s\n", strerror(errno));
        return -1;
    }
	if (setsockopt(sock, IPPROTO_IP, IP_TOS, tos, sizeof(*tos)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}
