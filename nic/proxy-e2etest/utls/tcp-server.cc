#include "nic/proxy-e2etest/ntls.hpp"

int bytes_recv,bytes_sent;
int port;


pthread_t server_thread;

void *main_server(void*);

int main(int argv, char* argc[]) {

  if (argv != 2) {
    printf("usage: ./tls port\n");
    exit(-1);
  }
  port = atoi(argc[1]);
  printf("Serving port %i\n", port);


  int rc = pthread_create(&server_thread, NULL, main_server, NULL);
  if (rc) {
    printf("Error creating server %i\n", rc);
    exit(-1);
  }
  while (1) { 
	sleep(2);
  }

  return 0;
}


int OpenListener(int port)
{   int sd;
  struct sockaddr_in addr;

  sd = socket(PF_INET, SOCK_STREAM, 0);
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if ( bind(sd, (const struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
      perror("can't bind port");
      abort();
    }
  if ( listen(sd, 10) != 0 )
    {
      perror("Can't configure listening port");
      abort();
    }
  return sd;
}


void test_tcp(int sd)
{
  int bytes, send_bytes;
  char buf[16384];
  int i;
  do {

    bytes = recv(sd, buf, sizeof(buf), 0);/* get request */
    if ( bytes > 0 ) {
      bytes_recv += bytes;
      printf("Bytes recv: %i\n", bytes_recv);
    }
    else {
      ERR_print_errors_fp(stderr);
      break;
    }
    for (i = 0; i < bytes; i++) {
      printf(" %02x", buf[i]);
    }
    printf("\n");
    send_bytes = send(sd, buf, bytes, 0);
    printf("Bytes sent: %i\n", send_bytes);
    bytes_sent += send_bytes; 
  } while (bytes > 0 );

}

void Servlet(int client)/* Serve the connection -- threadable */
{

  int sd = client;

  test_tcp(sd);

  while (1) {
      sleep(5);
  }

  close(sd);/* close connection */
  exit(0);
}

void *main_server(void* unused)
{

  int server = OpenListener(port);/* create server socket */
  while (1)
    {
      struct sockaddr_in addr;
      unsigned int len = sizeof(addr);

      int client = accept(server, (struct sockaddr*) &addr, &len);/* accept connection as usual */
      Servlet(client);/* service connection */
    }
  close(server);/* close server socket */

  return NULL;
}
