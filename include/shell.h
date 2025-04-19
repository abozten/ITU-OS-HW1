#ifndef SHELL_H
# define SHELL_H
# include "linkedlist.h"
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>

typedef struct
{
	char	*args[100];
	int		fd_in;
	int		fd_out;
	int		exit_status;
}			command;

void	executor(List *tokens_list);
void	display_token_group(void *content);
void	display_command(void *content);
void	display_exit_status(void *content);
void	free_token_group(void *content);
void	free_tokens(void *content);
void	free_command(void *content);
void    check_open_fds(void);

#endif