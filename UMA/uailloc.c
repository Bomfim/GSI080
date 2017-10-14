#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

typedef struct header_t {
	size_t size;
	unsigned is_free;
	struct header_t *next;
} Header;

Header *head, *tail;

Header *get_free_block(size_t size)
{
	Header *current = head;
	while(current) {
		if (current->is_free && current->size >= size)
			return current;
		current = current->next;
	}
	return NULL;
}

void *malloc(size_t size)
{
	size_t total_size;
	void *block;
	Header *header;

	if (!size)
		return NULL;
	header = get_free_block(size);

	if (header) {
		header->is_free = 0;
		return (void*)(header + 1);
	}

	total_size = sizeof(Header) + size;
	block = sbrk(total_size);

	if (block == (void*) -1) {
		return NULL;
	}

	header = block;
	header->size = size;
	header->is_free = 0;
	header->next = NULL;

	fprintf(stderr, "%p\t%d\t%d\t%p\n",header, header->size, header->is_free, header->next);
	
	if (!head)
		head = header;
	if (tail)
		tail->next = header;
	tail = header;

	return (void*)(header + 1);
}



void free(void *block)
{
	Header *header, *tmp;
	void *programbreak;

	if (!block)
		return;
	
	header = (Header*)block - 1;

	programbreak = sbrk(0);
	if ((char*)block + header->size == programbreak) {
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				if(tmp->next == tail) {
					tmp->next = NULL;
					tail = tmp;
				}
				tmp = tmp->next;
			}
		}
		sbrk(0 - sizeof(Header) - header->size);
		return;
	}
	header->is_free = 1;
}