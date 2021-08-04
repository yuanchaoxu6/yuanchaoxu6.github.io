#include <mcontainer.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>

int stride = 256;

struct node
{
		int val;
		struct node * next;
};

int main()
{
		int devfd;
		devfd = open("/dev/mcontainer", O_RDWR);
		char *mapped_data, *data;
		struct timeval current_time;
        if (devfd < 0)
		{
				fprintf(stderr, "Device open failed");
				exit(1);
		}
		struct node * head = (struct node*)malloc(16384*stride*sizeof(struct node));
		int i, j;
		struct node *it;
		for (it = head, i =0; i < 16384; i++, it = head+i*stride)
		{
				it->val = i;
				if (i != 16383) it->next = head+(i+1)*stride;
				else it->next = NULL;
		}
//  for (it = head; it != NULL; it = it->next)
//  {
//      printf("%d\n", it->val);
//  }
		int sum = 0;
		gettimeofday(&current_time, NULL);
		printf("S\t%d\t%ld\t%d\n", getpid(),  current_time.tv_sec * 1000000 + current_time.tv_usec, i);
		for (i = 0; i < 500; ++i)
		{
				mapped_data = (char *)mcontainer_alloc(devfd, 0, 16384*stride*sizeof(struct node));
				printf("S\t%d\t%ld\t%d\t%s\n", getpid(),  current_time.tv_sec * 1000000 + current_time.tv_usec, i, mapped_data);
				int t = rand()%16384;
				for (it =head; it != NULL; it = it->next)
				{
						if (it->val == t) {sum++; break;}
				}
		}
		gettimeofday(&current_time, NULL);
		printf("S\t%d\t%ld\t%d\n", getpid(),  current_time.tv_sec * 1000000 + current_time.tv_usec, i);
		printf("found:%d\n",sum);
		return 0;
}
