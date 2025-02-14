#include "../ft_traceroute.h"

int	cmd_help(void)
{
	printf("Usage: ft_ping [OPTION...] HOST ...\n");
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("\n");
	printf("  -?, --help     give this help list\n");
	printf("  -V, --version  print program version\n");
	printf("  -v, --verbose  verbose output\n");
	printf("      --ttl      set the time-to-live\n");
	printf("  -c, --count    stop after sending count ECHO_REQUEST packets\n");
	printf("  -w, --timeout  time to wait for a response, in seconds\n");
	printf("  -i, --interval wait interval seconds between sending each packet\n");
	printf("  -T, --tos      set Type Of Service\n");
	return (0);
}

int	cmd_version(void)
{
	printf("ft_ping (take as reference ping from inetutils-2.0)\n");
	printf("Written by Valentin Viovi, 42 Student\n");
	return (0);
}
