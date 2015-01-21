#include <stdio.h>
#include <sys/select.h>

void your_callback()
{
	printf("%s\n", __FUNCTION__);
}

int main()
{
	struct timeval t;

	while (1) {
		t.tv_sec = 1;
		t.tv_usec = 0;

		select(0, NULL, NULL, NULL, &t);

		your_callback();
	}

	return 0;
}
