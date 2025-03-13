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
	if (!parsing(argc, argv))
		return (1);
	return (0);
}
