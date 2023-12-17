#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#define PORT 8080

void* recievedCommand(void* socket_ptr) {
    int socket_fd = (int)(intptr_t)socket_ptr;
    char buffer[16184];     // 16kb buffer

    while (true)
    {
        ssize_t recieved_amount = recv(socket_fd, buffer, 16184, 0);

        if(recieved_amount == 0) {
            break;
        }        

        if(recieved_amount > 0) {
            buffer[recieved_amount] = 0;
            printf("%s\n", buffer);
        }
    }

    close(socket_fd);
    return NULL;
    
}

void listeningOnThread(int socket_fd) {
    pthread_t id;
    //pthread_create(&id, NULL, recievedCommand, socket_fd);
  pthread_create(&id, NULL, recievedCommand, (void*)(intptr_t)socket_fd);
}

void sendRequestToServer(int socket_fd) {
    // testing sending simple sentences

    char *line;
    size_t line_size = 0;
    printf("type smth to send...\n");

    char buffer[1024];
    
    while (true)
    {
        ssize_t char_count = getline(&line, &line_size, stdin);
        line[char_count-1] = 0;

        sprintf(buffer, "%s", line);

        if(char_count > 0) {
            if (strcmp(line, "exit") == 0) {
                break;
            }

            ssize_t amount_recieved = send(socket_fd, buffer, strlen(buffer), 0);
        } 
    }
    
}

int main(int argc, char const *argv[])
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_fd < 0) {
        perror("Socket creation error on client side\n");
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = inet_addr(argv[1]);

  int connect_result = connect(socket_fd, (struct sockaddr*)&address, sizeof(address));

    if(connect_result == 0){
        printf("Connection was successful.\n");
    } 

    listeningOnThread(socket_fd);  

    sendRequestToServer(socket_fd);

    close(socket_fd);

    return 0;
}

