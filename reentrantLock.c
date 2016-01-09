#include <stdio.h>
#include <pthread.h>

/*互斥锁默认为不可重入的，将同一个锁多次加锁会发生死锁*/

pthread_mutex_t m_mutexA;
pthread_mutexattr_t m_attr;

int main()
{
	//pthread_mutexattr_init(&m_attr);
	//pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
	//pthread_mutex_init(&m_mutexA, &m_attr);
	pthread_mutex_init(&m_mutexA, NULL);

	pthread_mutex_lock(&m_mutexA);
	pthread_mutex_lock(&m_mutexA);
	printf("this is lock area");
	pthread_mutex_unlock(&m_mutexA);
	pthread_mutex_unlock(&m_mutexA);

	return 0;
}
