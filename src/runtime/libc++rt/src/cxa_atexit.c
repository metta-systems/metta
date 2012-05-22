#ifdef __sun__
#include <pthread.h>
#include <stdlib.h>

static struct atexit_handler {
  void (*f)(void *);
  void *p;
  void *d;
  struct atexit_handler *next;
} *head;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int __cxa_atexit( void (*f)(void *), void *p, void *d) {
  pthread_mutex_lock(&lock);
  struct atexit_handler *h = malloc(sizeof(*h));
  if (!h) {
    pthread_mutex_unlock(&lock);
    return 1;
  }
  h->f = f;
  h->p = p;
  h->d = d;
  h->next = head;
  head = h;
  pthread_mutex_unlock(&lock);
  return 0;
}

void __cxa_finalize(void *d ) {
  pthread_mutex_lock(&lock);
  struct atexit_handler **last = &head;
  for (struct atexit_handler *h = head ; h ; h = h->next) {
    if ((h->d == d) || (d == 0)) {
      *last = h->next;
      h->f(h->p);
      free(h);
    } else {
      last = &h->next;
    }
  }
  pthread_mutex_unlock(&lock);
}
#endif
