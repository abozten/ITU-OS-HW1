#include <shell.h>

void	display_token_group(void *content)
{
	char	**tokens = content;
	printf("{ ");
	for (int i = 0; tokens[i]; i++)
		printf("\"%s\", ", tokens[i]);
	printf("NULL } --> ");
}

void	display_command(void *content)
{
	command	*cmd;

	cmd = content;
	printf("Command: ");
	printf("fd_in: %d, fd_out: %d, args: {", cmd->fd_in, cmd->fd_out);
	for (int i = 0; cmd->args[i]; i++)
		printf("\"%s\", ", cmd->args[i]);
	printf("NULL }\n");
}

void	display_exit_status(void *content)
{
	command	*cmd;

	cmd = content;
	printf("Exit status: %d\n", cmd->exit_status);
}
