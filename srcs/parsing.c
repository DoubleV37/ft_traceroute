#include "../ft_traceroute.h"

int verify_ttl(int ttl, int max_or_min)
{
	if (ttl < 1 || ttl > 255)
	{
		if (max_or_min == 1)
			printf("ft_traceroute: first hop out of range\n");
		else
			printf("ft_traceroute: max hops cannot be more than 255\n");
		return (1);
	}
	return (0);
}

int parsing(int argc, char **argv)
{
	int i = 1;
	char *cmd;

	if (argc == 1) {
		printf("ft_traceroute: missing host operand\n");
		printf("Try 'ft_traceroute --help' for more information.\n");
		return (1);
	}
	ping ping;
	initial_init_traceroute(&ping);
	while (i < argc) {
		if (argv[i][0] != '-') {
			if (ping.params.raw_dest) {
				printf("ft_traceroute: too many host arguments\n");
				free(ping.params.raw_dest);
				return (1);
			}
			ping.params.raw_dest = strdup(argv[i]);
			i++;
			continue;
		}
		else if (ft_strcmp(argv[i], "--help") == 0) {
			free(ping.params.raw_dest);
			return (cmd_help());
		}
		else if (ft_strcmp(argv[i], "-V") == 0 || ft_strcmp(argv[i], "--version") == 0) {
			free(ping.params.raw_dest);
			return (cmd_version());
		}
		else if (ft_strcmp(argv[i], "-I") == 0 || ft_strcmp(argv[i], "--icmp") == 0) {
			ping.params.type_traceroute = 1;
			if (getgid() != 0) {
				printf("You do not have enough privileges to use this traceroute method.\n");
				free(ping.params.raw_dest);
				return (1);
			}
		}
		else if (ft_strcmp(argv[i], "-f") == 0 || ft_strcmp(argv[i], "--first") == 0) {
			if (i + 1 < argc) {
				ping.params.ttl = atoi(argv[i + 1]);
				i++;
				if (verify_ttl(ping.params.ttl, 1)) {
					free(ping.params.raw_dest);
					return (1);
				}
			}
			else {
				printf("ft_traceroute: option requires an argument -- 'f'\n");
				free(ping.params.raw_dest);
				return (1);
			}
		}
		else if (ft_strcmp(argv[i], "-m") == 0 || ft_strcmp(argv[i], "--max-hops") == 0) {
			if (i + 1 < argc) {
				ping.params.max_ttl = ft_atoi(argv[i + 1]);
				i++;
				if (verify_ttl(ping.params.max_ttl, 0)) {
					free(ping.params.raw_dest);
					return (1);
				}
			}
			else {
				printf("ft_traceroute: option requires an argument -- 'm'\n");
				free(ping.params.raw_dest);
				return (1);
			}
		}
		else if (ft_strcmp(argv[i], "-t") == 0 || ft_strcmp(argv[i], "--tos") == 0) {
			if (i + 1 < argc) {
				ping.params.tos = ft_atoi(argv[i + 1]);
				i++;
			}
			else {
				printf("ft_traceroute: option requires an argument -- 't'\n");
				free(ping.params.raw_dest);
				return (1);
			}
		}
		else {
			cmd = clean_argv(argv[i]);
			printf("ft_traceroute: invalid option -- '%s'\n", cmd);
			free(cmd);
			free(ping.params.raw_dest);
			return (1);
		}
		i++;
	}
	if (ping.params.raw_dest) {
		int return_val = 0;
		if (!get_ip_with_hostname(ping.params.raw_dest, ping.params.ip_addr_dest)) {
			printf("ft_traceroute: unknown host\n");
			free(ping.params.raw_dest);
			return (1);
		}
		if (ping.params.ttl > ping.params.max_ttl) {
			printf("ft_traceroute: invalid first hop value\n");
			free(ping.params.raw_dest);
			return (1);
		}
		if (initial_setup_traceroute(&ping)) {
			printf("ft_traceroute: error while setting up ping\n");
			free(ping.params.raw_dest);
			return (1);
		}
		return_val = cmd_traceroute(&ping);
		end_ping(&ping);
		if (return_val == 0) {
			free(ping.params.raw_dest);
			return (0);
		}
		free(ping.params.raw_dest);
	}
	else {
		printf("ft_ traceroute: missing host operand\n");
		printf("Try 'ft_ping --help' for more information.\n");
		free(ping.params.raw_dest);
		return (1);
	}
	return (0);
}
