#include "../ft_traceroute.h"

int parsing(int argc, char **argv)
{
	int i = 1;
	char *cmd;

	if (argc == 1)
	{
		printf("ft_traceroute: missing host operand\n");
		printf("Try 'ft_traceroute --help' for more information.\n");
		return (1);
	}
	ping ping;
	initial_init_ping(&ping);
	while (i < argc)
	{
		if (argv[i][0] != '-')
		{
			ping.params.raw_dest = strdup(argv[i]);
			i++;
			continue;
		}
		// else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0)
		// {
		// 	free(ping.params.raw_dest);
		// 	return (cmd_help());
		// }
		// else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
		// {
		// 	free(ping.params.raw_dest);
		// 	return (cmd_version());
		// }
		else
		{
			cmd = clean_argv(argv[i]);
			printf("ft_traceroute: invalid option -- '%s'\n", cmd);
			free(cmd);
			free(ping.params.raw_dest);
			return (1);
		}
		i++;
	}
	if (ping.params.raw_dest)
	{
		int return_val = 0;

		if (!get_ip_with_hostname(ping.params.raw_dest, ping.params.ip_addr_dest))
		{
			printf("ft_traceroute: unknown host\n");
			free(ping.params.raw_dest);
			free(ping.params.ip_addr_src);
			return (1);
		}
		if (initial_setup_traceroute(&ping))
		{
			printf("ft_traceroute: error while setting up ping\n");
			free(ping.params.raw_dest);
			free(ping.params.ip_addr_src);
			return (1);
		}
		return_val = cmd_traceroute(&ping);
		end_ping(&ping);
		if (return_val == 0)
		{
			free(ping.params.raw_dest);
			free(ping.params.ip_addr_src);
			return (0);
		}
		free(ping.params.raw_dest);
		// free(ping.params.ip_addr_src);
	}
	else
	{
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping --help' for more information.\n");
		return (1);
	}
	return (0);
}
