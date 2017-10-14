#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

//Struct para representar o bloco + header.
typedef struct header_t {
	size_t size;
	unsigned is_free;
	struct header_t *next;
} Header;

//Cabeça e calda da lista ligada.
Header *head, *tail;

//Retorna o 1º bloco livre (first-fit).
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

	if (!size)		// Se o tamanho for inválido.
		return NULL;
	
	header = get_free_block(size);

	if (header) {	// Se ele possui um bloco do tamanho desejado.
		header->is_free = 0;
		return (void*)(header + 1);
	}

	total_size = sizeof(Header) + size;
	block = sbrk(total_size);		// Caso contrário, aloca-se via syscall um novo pedaço de memória.

	if (block == (void*) -1) { // Teste de retorno da sbrk.
		return NULL;
	}

	header = block;
	header->size = size;
	header->is_free = 0;
	header->next = NULL;

	fprintf(stderr, "%p\t%d\t%d\t%p\n",header, header->size, header->is_free, header->next);
	
	if (!head)		// Teste se for o primeiro a ser inserido, atualiza a cabeça.
		head = header;
	if (tail)		// Teste se for o primeiro a ser inserido, atualiza a calda.
		tail->next = header;
	tail = header;

	return (void*)(header + 1); // Retorna um ponteiro do próximo espaço requisitado.
}



void free(void *block)
{
	Header *header, *aux;
	void *programbreak;

	if (!block)		// Teste valor válido
		return;
	
	header = (Header*)block - 1; // Pega o bloco anterior.

	programbreak = sbrk(0); // Final da heap.
	if ((char*)block + header->size == programbreak) { // Teste se o último bloco é o final da heap.
		if (head == tail) {
			head = tail = NULL;		// Atualiza a cabeça.
		} else {
			aux = head;
			while (aux) {			// Atualização da cauda.
				if(aux->next == tail) {
					aux->next = NULL;
					tail = aux;
				}
				aux = aux->next;
			}
		}
		sbrk(0 - sizeof(Header) - header->size); // Desalocando o tamanho necessário.
		return;
	}
	header->is_free = 1; // Flag de livre/usado.
}