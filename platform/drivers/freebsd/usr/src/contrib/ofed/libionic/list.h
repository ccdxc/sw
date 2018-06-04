#ifndef LIST_H
#define LIST_H

struct list_node {
	struct list_node *next;
	struct list_node *prev;
};
struct list_head {
	struct list_node head;
};

static inline void list_node_init(struct list_node *node)
{
	node->next = node;
	node->prev = node;
}

static inline void list_head_init(struct list_head *head)
{
	list_node_init(&head->head);
}

static inline void list_del(struct list_node *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
}

static inline void list_del_init(struct list_node *node)
{
	list_del(node);
	list_node_init(node);
}

static inline void list_add_tail(struct list_head *head,
				 struct list_node *node)
{
	node->next = &head->head;
	node->prev = head->head.prev;
	head->head.prev->next = node;
	head->head.prev = node;
}

#define list_for_each_safe(hd, pt, nx, member) \
	for ((pt = container_of((hd)->head.next,	\
			       typeof(*(pt)),		\
			       member));		\
	     (nx = container_of(((pt)->member).next,	\
				 typeof(*(pt)),		\
				 member)),		\
	     ((&(pt)->member) != &(hd)->head);		\
	     (pt = (nx)))

#endif
