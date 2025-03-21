#include "../ft_traceroute.h"

bool g_run = true;

void	handle_sigint(int sig)
{
	(void)sig;
	g_run = false;
}

int	main(int argc, char **argv)
{
	signal(SIGINT, handle_sigint);
	if (getgid() != 0) {
		printf("You do not have enough privileges.\n");
		return (1);
	}
	if (!parsing(argc, argv))
		return (1);
	return (0);
}
