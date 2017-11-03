#include "nic/proxy-e2etest/ntls.hpp"

int bytes_recv,bytes_sent;
int port;


pthread_t server_thread;

void *main_server(void*);

int tcp_server_main(int argv, char* argc[]) {

  if (argv != 2) {
    TLOG( "usage: ./hntap <tcp-port-num>\n");
    exit(-1);
  }
  port = atoi(argc[1]);
  TLOG( "Serving port %i\n", port);


  int rc = pthread_create(&server_thread, NULL, main_server, NULL);
  if (rc) {
    TLOG( "Error creating server %i\n", rc);
    exit(-1);
  }

  /*
  while (1) { 
	sleep(2);
  }
  */

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

  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

  if ( bind(sd, (const struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
      TLOG("can't bind port - %s", strerror(errno));
      abort();
    }
  if ( listen(sd, 10) != 0 )
    {
      TLOG("Can't configure listening port - %s", strerror(errno));
      abort();
    }
  TLOG( "Created listener on port %d!!\n", port);
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
      TLOG( "Bytes recv: %i\n", bytes_recv);
    }
    else {
      ERR_print_errors_fp(stderr);
      break;
    }
    for (i = 0; i < bytes; i++) {
      TLOG( " %02x", buf[i]);
    }
    TLOG( "  %s", buf);
    TLOG( "\n");
    send_bytes = send(sd, buf, bytes, 0);
    TLOG( "Bytes sent: %i\n", send_bytes);
    bytes_sent += send_bytes; 
  } while (bytes > 0 );

}

void Servlet(int client)/* Serve the connection -- threadable */
{

  int sd = client;

  test_tcp(sd);

  do {
      sleep(5);
  } while(0);

  //close(sd);/* close connection */

  //exit(0);
}

void *main_server(void* unused)
{

  int server = OpenListener(port);/* create server socket */
  while (1)
    {
      struct sockaddr_in addr;
      unsigned int len = sizeof(addr);
      TLOG( "Waiting to accept!!\n");
      int client = accept(server, (struct sockaddr*) &addr, &len);/* accept connection as usual */
      Servlet(client);/* service connection */
    }
  close(server);/* close server socket */

  return NULL;
}
