#include <malloc.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
mybacktrace(void)
{
	int j, nptrs;
#define SIZE 100
	void *buffer[100];
	char **strings;

	nptrs = backtrace(buffer, SIZE);
	printf("backtrace() returned %d addresses\n", nptrs);

	/* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
	 *        would produce similar output to the following: */

	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}

	for (j = 0; j < nptrs; j++)
		printf("%s\n", strings[j]);

	free(strings);
}

/* Prototypes for our hooks.  */
static void my_init_hook (void);
static void *my_malloc_hook (size_t, const void *);
static void my_free_hook (void*, const void *);

/* Override initializing hook from the C library. */
void (*__malloc_initialize_hook) (void) = my_init_hook;
void *(*old_malloc_hook)(size_t size, const void *caller);
void (*old_free_hook)(void *ptr, const void *caller);

static void
my_init_hook (void)
{
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
}

	static void *
my_malloc_hook (size_t size, const void *caller)
{
	void *result;
	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	/* Call recursively */
	result = malloc (size);
	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	/* printf might call malloc, so protect it too. */
	//printf ("malloc (%u) returns %p\n", (unsigned int) size, result);
	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	return result;
}

	static void
my_free_hook (void *ptr, const void *caller)
{
	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	/* Call recursively */
	free (ptr);
	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	/* printf might call free, so protect it too. */
	//printf ("freed pointer %p\n", ptr);
	//mybacktrace();
	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
}
struct test{
	char dummy[80];
};

int main(){
	struct test *a; 
	int i = 0; 
	int count = 135;
	for( i = 0; i < count; ++i){ 
		a = (struct test *)malloc(sizeof(struct test));
		printf("%lu\n", ((unsigned long long *)a)[-1]);
		free(a);
		printf("%lu\n", ((unsigned long long *)a)[-1]);
		a = NULL;
	}
//	printf("the struct's size is %d.\n", sizeof(struct test));
//	printf("the head is %d\n", ((unsigned int*)a)[-1]);
//	printf("the head is %d\n", ((unsigned int*)a)[-2]);
//	printf("the head is %lu\n", ((unsigned long long*)a)[-1]);
	if(a != NULL)
		free(a);
	return 0;
}
