#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <net/if.h>
# include <arpa/inet.h>
# include <time.h>
# include <signal.h>
# include <errno.h>
# include <ifaddrs.h>
# include <stdbool.h>

extern bool g_run;

typedef struct ping_pckt{
	int seq;
	char *ip_addr;
	struct timeval sent_time;
	struct timeval recv_time;
	struct ping_pckt *next;
} ping_pckt;

typedef struct s_ping_data{
	struct ip ip_hdr;
	struct icmphdr icmp_hdr;
	uint8_t data[56];
} ping_data;

typedef struct s_socks{
	int recv;
	int send;
} socks;

typedef struct s_params{
	uint16_t id;
	char *ip_addr_src;
	char *raw_dest;
	char ip_addr_dest[16];
	int ttl;
	int max_ttl;
	struct s_ping_data *data;
	u_int16_t seq;
	int type_traceroute;
} params;

typedef struct s_ping{
	params params;
	socks socks;
	ping_pckt *pings;
} ping;

// commands.c
int	cmd_help(void);
int	cmd_version(void);

// ping_utils.c
ping_pckt*	add_ping(ping_pckt *head, u_int16_t seq_num);
ping_pckt*	find_ping(ping_pckt *head, u_int16_t seq_num);
void	free_ping(ping_pckt *head);
double	time_diff(struct timeval start, struct timeval end);
float	ft_sqrt(float number);
unsigned	short checksum(void *b, int len);
char *clean_argv(char *argv);
void	initial_init_traceroute(ping *ping);

// parsing.c
int	parsing(int argc, char **argv);

// traceroute.c
int	cmd_traceroute(ping *ping);
int	initial_setup_traceroute(ping *ping);
char	*get_source_ip();
int	get_ip_with_hostname(char *hostname, char final_ip[INET_ADDRSTRLEN]);

// ping_utils.c
int create_socket_send_icmp(void);
ping_data *build_ping_data(ping *ping);
void update_ping_data(ping_data *data, ping *ping);
int send_pings(ping *ping);
void print_ping_delay(ping_pckt *pckt, int cpt);
int recv_pings(ping *ping);
void	end_ping(ping *ping);

#endif
