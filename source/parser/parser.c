#include <shell.h>

void	*convert_to_command(void *content)
{
	int		i;
	int		j;
	char	**tokens;
	command	*cmd;

	tokens = content;
	cmd = malloc(sizeof(command));
	cmd->fd_in = 0;
	cmd->fd_out = 1;
	cmd->exit_status = 0;
	i = 0;
	j = 0;
	while (tokens[i])
	{
		if (!strcmp(tokens[i], ">"))
		{
			i++;
			cmd->fd_out = open(tokens[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (cmd->fd_out < 0)
			{
				perror("Error opening file");
				exit(EXIT_FAILURE);
			}
		}
		else if (!strcmp(tokens[i], ">>"))
		{
			i++;
			cmd->fd_out = open(tokens[i], O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (cmd->fd_out < 0)
			{
				perror("Error opening file");
				exit(EXIT_FAILURE);
			}
		}
		else if (!strcmp(tokens[i], "<"))
		{
			i++;
			cmd->fd_in = open(tokens[i], O_RDONLY);
			if (cmd->fd_in < 0)
			{
				perror("Error opening file");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			cmd->args[j++] = strdup(tokens[i]);
		}
		i++;
	}
	cmd->args[j] = NULL;
	return (cmd);
}

void	handle_tokens(char **tokens, int token_count)
{
	List	*tokens_list;

	tokens_list = parse(tokens, ";");
	if (!tokens_list)
	{
		fprintf(stderr, "Error: Failed to parse tokens\n");
		return ;
	}
	printf("_____PARSER____\n");
	display(tokens_list, display_token_group);
	printf("NULL\n");
	for (Node *curr = tokens_list->head; curr; curr = curr->next)
	{
		List	*token_group_list;
		List	*command_list;

		token_group_list = parse(curr->content, "|");
		command_list = map(token_group_list, convert_to_command);
		destroy(token_group_list, free_token_group);
		executor(command_list);
		destroy(command_list, free_command);
		check_open_fds();
	}
	destroy(tokens_list, free_token_group);
}
