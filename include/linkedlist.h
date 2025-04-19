#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct node
{
	void		*content;
	struct node	*next;
}				Node;

typedef struct
{
	Node	*head;
}			List;

List	*makelist(void);
Node	*createnode(void *content);
void	add(Node *newNode, List *list);
void	delete(void *content, List *list);
void	display(List * list, void (*print_func)(void *));
void	reverse(List *list);
void	reverse_using_two_pointers(List *list);
void	destroy(List *list, void (*free_func)(void *));
List	*parse(char **tokens, char *delimeter);
List	*map(List *list, void *(*func)(void *));

#endif
