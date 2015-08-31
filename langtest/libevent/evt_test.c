#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/event.h>

#define BUFSIZE 256
#define TIMEOUT_SEC 0

long long count = 0;

void read_handler(int fd, short event, void *arg)
{
  char buffer[BUFSIZE];
  ssize_t read_len;
  read_len = read(fd, buffer, BUFSIZE);
  buffer[read_len] = '\0';
  printf("%s", buffer);
}

void event_handler(int fd, short event, void *arg)
{
  if (event & EV_TIMEOUT)
    count++;
  if (count == 10000) {
    printf("1...\n");
    count = 0;
  }
}

int main(int argc, const char* argv[])
{
  printf("start...\n");
  struct event_base *ev_base;
  struct event *ev;
  struct timeval tv;

  tv.tv_sec = TIMEOUT_SEC;
  tv.tv_usec = 10;

  ev_base = event_base_new();
  ev = event_new(ev_base,
                 fileno(stdin),
                 EV_TIMEOUT | EV_PERSIST,
                 event_handler,
                 NULL);
  event_add(ev, &tv);
  event_base_dispatch(ev_base);

  event_free(ev);
  event_base_free(ev_base);

  return 0;
}