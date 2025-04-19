#include <shell.h>

Node	*createnode(void *content)
{
	Node	*newNode;

	newNode = malloc(sizeof(Node));
	if (!newNode)
		return (NULL);
	newNode->content = content;
	newNode->next = NULL;
	return (newNode);
}

List	*makelist(void)
{
	List	*list;
	
	list = malloc(sizeof(List));
	if (!list)
		return (NULL);
	list->head = NULL;
	return list;
}

void display(List * list, void (*print_func)(void *))
{
	if(!list->head) 
		return ;
	for (Node *current = list->head; current; current = current->next)
		print_func(current->content);
}

void	add(Node *newNode, List *list)
{
	Node *current;

	if(!list->head)
		list->head = newNode;
	else
	{
		current = list->head; 
		while (current->next!=NULL)
			current = current->next;
		current->next = newNode;
	}
}

void	delete(void *content, List *list)
{
	Node	*current;
	Node	*previous;

	current = list->head;
	previous = current;
	while(current)
	{
		if(current->content == content)
		{
			previous->next = current->next;
			if(current == list->head)
				list->head = current->next;
			free(current);
			return ;
		}
		previous = current;
		current = current->next;
	}
}

void reverse(List *list)
{
	Node	*reversed;
	Node	*current;
	Node	*temp;

	reversed = NULL;
	current = list->head;
	temp = NULL;
	while(current != NULL)
	{
		temp = current;
		current = current->next;
		temp->next = reversed;
		reversed = temp;
	}
	list->head = reversed;
}

void	reverse_using_two_pointers(List *list)
{
	Node	*previous;
	
	previous = NULL;
	while (list->head)
	{
		Node *next_node = list->head->next;
		list->head->next = previous;
		previous = list->head;
		list->head = next_node;
	}
	list->head = previous;
}

void	destroy(List *list, void (*free_func)(void *))
{
	Node	*current;
	Node	*next;

	current = list->head;
	next = current;
	while (current != NULL)
	{
		next = current->next;
		free_func(current);
		current = next;
	}
	free(list);
}

List	*parse(char **tokens, char *delimeter)
{
	List	*tokens_list;
	int		start;
	int		len;
	int		c;
	char	**subtokens;

	start = 0;
	tokens_list = makelist();
	for (c = 0; tokens[c]; c++);
	for (int i = 0; i <= c; i++)
	{
		if (i == c || strcmp(tokens[i], delimeter) == 0)
		{
			len = i - start;
			if (len > 0)
			{
				subtokens = malloc((len + 1) * sizeof(char *));
				for (int j = 0; j < len; j++)
					subtokens[j] = strdup(tokens[start + j]);
				subtokens[len] = NULL;
				add(createnode(subtokens), tokens_list);
			}
			start = i + 1;
		}
	}
	return (tokens_list);	
}

List	*map(List *list, void *(*func)(void *))
{
	List	*new_list;
	Node	*current;

	new_list = makelist();
	if (!new_list)
		return (NULL);
	current = list->head;
	while (current)
	{
		add(createnode(func(current->content)), new_list);
		current = current->next;
	}
	return (new_list);
}
