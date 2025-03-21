#include "../ft_traceroute.h"

ping_pckt* add_pckt(ping_pckt *head, u_int16_t seq_num) {
	ping_pckt *new_node = (ping_pckt *)malloc(sizeof(ping_pckt));
	if (!new_node)
		return NULL;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	new_node->seq = seq_num;
	new_node->sent_time = tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	new_node->recv_time = tv;
	new_node->next = head;
	return new_node;
}

ping_pckt* find_pckt(ping_pckt *head, u_int16_t seq_num) {
	ping_pckt *current = head;
	while (current) {
		if (current->seq == seq_num) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

void free_ping(ping_pckt *head) {
	ping_pckt *current = head;
	ping_pckt *next;
	while (current) {
		next = current->next;
		free(current);
		current = next;
	}
	head = NULL;
}

double time_diff(struct timeval start, struct timeval end) {
	double seconds;
	double microseconds;
	seconds = (double)(end.tv_sec - start.tv_sec);
	microseconds = (double)(end.tv_usec - start.tv_usec);

	return (seconds * 1000.0) + (microseconds / 1000.0);
}

float ft_sqrt(float number) {
	float x = number;
	float y = 1.0;
	float epsilon = 0.0001;

	while (x - y > epsilon) {
		x = (x + y) / 2;
		y = number / x;
	}
	return x;
}

int ft_atoi(const char *str) {
	int sign = 1;
	int result = 0;

	while (*str == ' ' || (*str >= 9 && *str <= 13)) {
		str++;
	}
	if (*str == '-' || *str == '+') {
		if (*str == '-') {
			sign = -1;
		}
		str++;
	}
	while (*str >= '0' && *str <= '9') {
		result = result * 10 + (*str - '0');
		str++;
	}
	return sign * result;
}

int ft_strcmp(const char *s1, const char *s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2) {
			return (unsigned char)*s1 - (unsigned char)*s2;
		}
		s1++;
		s2++;
	}
	return (unsigned char)*s1 - (unsigned char)*s2;
}

unsigned short checksum(void *b, int len) {
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2) {
		sum += *buf++;
	}

	if (len == 1) {
		sum += *(unsigned char *)buf;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

char *clean_argv(char *argv)
{
	int i;
	int cpt;
	char *argvclean;

	argvclean = NULL;

	i = 0;
	cpt = 0;
	while (argv[i])
	{
		if (argv[i] == '-')
			cpt++;
		i++;
	}
	argvclean = strdup(argv + cpt);
	return (argvclean);
}

void initial_init_traceroute(ping *ping)
{
	ping->pings = NULL;
	ping->socks.recv = -1;
	ping->socks.send = -1;
	ping->params.ttl = 1;
	ping->params.max_ttl = 30;
	ping->params.ip_addr_src = NULL;
	ping->params.raw_dest = NULL;
	ping->params.ip_addr_dest[0] = '\0';
	ping->params.type_traceroute = 0;
	ping->params.tos = 0;
}

const char *reverse_dns_lookup(const char *ip) {
    static char host[1024];
    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &sa.sin_addr) <= 0) {
        return ip;
    }
    if (getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0) {
        return host;
    }
    return ip;
}
