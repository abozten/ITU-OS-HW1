#include <shell.h>

void	free_token_group(void *content)
{
	Node	*curr;
	char	**subtokens;

	if (!content)
		return ;
	curr = content;
	subtokens = curr->content;
	for (int i = 0; subtokens[i]; i++)
		free(subtokens[i]);
	free(subtokens);
	free(curr);
}

void	free_tokens(void *content)
{
	char	**tokens;

	tokens = content;
	for (int i = 0; tokens[i]; i++)
		free(tokens[i]);
	free(tokens);
}

void	free_command(void *content)
{
	Node	*curr;
	command	*cmd;

	curr = content;
	cmd = curr->content;
	if (!cmd)
		return ;
	for (int i = 0; cmd->args[i]; i++)
		free(cmd->args[i]);
	if (cmd->fd_in > 2)
		close(cmd->fd_in);
	if (cmd->fd_out > 2)
		close(cmd->fd_out);
	free(cmd);
	free(curr);
}
