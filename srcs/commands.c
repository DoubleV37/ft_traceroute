#include "../ft_traceroute.h"

int	cmd_help(void)
{
	printf("Usage: ft_traceroute [OPTION...] host\n");
	printf("\n");
	printf("      --help     give this help list\n");
	printf("  -V, --version  print program version\n");
	printf("  -I, --icmp     use ICMP ECHO for tracerouting\n");
	printf("  -m, --max-hops=N  set the maximum TTL, default value is 30\n");
	printf("  -f, --first=N  set the first TTL, default value is 1\n");
	printf("  -t, --tos=N     set the TOS (type of service)\n");
	return (0);
}

int	cmd_version(void)
{
	printf("ft_traceroute\n");
	printf("Written by Valentin Viovi, 42 Student\n");
	return (0);
}
