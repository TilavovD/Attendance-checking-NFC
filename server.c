#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>

#define PORT 8080

struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

void acceptNewConnectionAndReceiveAndPrintItsData(int serverSocketFD);
void* receiveAndPrintIncomingData(void* socket_ptr);

void startAcceptingIncomingConnections(int serverSocketFD);

void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket);

void sendReceivedMessageToTheOtherClients(char *buffer, int socketFD);

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD);

struct AcceptedSocket acceptedSockets[10] ;
int acceptedSocketsCount = 0;

void startAcceptingIncomingConnections(int serverSocketFD) {

    while(true)
    {
        struct AcceptedSocket* clientSocket  = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketsCount++] = *clientSocket;

        receiveAndPrintIncomingDataOnSeparateThread(clientSocket);
    }
}

void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket) {

    pthread_t id;
    //pthread_create(&id,NULL,receiveAndPrintIncomingData,pSocket->acceptedSocketFD);
    pthread_create(&id, NULL, receiveAndPrintIncomingData, (void*)(intptr_t)pSocket->acceptedSocketFD);
}
void* receiveAndPrintIncomingData(void* socket_ptr) {
    int socketFD = (int)(intptr_t)socket_ptr;
    
    char buffer[16184];

    while (true)
    {
        ssize_t  amountReceived = recv(socketFD,buffer,16184,0);

        if(amountReceived>0)
        {
            buffer[amountReceived] = 0;
            printf("%s\n",buffer);

            sendReceivedMessageToTheOtherClients(buffer,socketFD);
        }

        if(amountReceived==0)
            break;
    }

    close(socketFD);
    
    pthread_exit(NULL);
}


void sendReceivedMessageToTheOtherClients(char *buffer,int socketFD) {

    for(int i = 0 ; i<acceptedSocketsCount ; i++)
        if(acceptedSockets[i].acceptedSocketFD !=socketFD)
        {
            send(acceptedSockets[i].acceptedSocketFD,buffer, strlen(buffer),0);
        }

}

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in  clientAddress ;
    int clientAddressSize = sizeof (struct sockaddr_in);
    //int clientSocketFD = accept(serverSocketFD,&clientAddress,&clientAddressSize);
  int clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &clientAddressSize);
    struct AcceptedSocket* acceptedSocket = malloc(sizeof (struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD>0;

    if(!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = clientSocketFD;

    return acceptedSocket;
}

int main(int argc, char const *argv[])
{
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("Socket creation error.\n");
    }
    else {
        printf("Socket created.\n");
    }
    
    struct sockaddr_in address;;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = inet_addr(argv[1]);
    
    /*int bind_result = bind(server_socket_fd, (struct sockaddr_in*)&address, sizeof(address));*/
    int bind_result = bind(server_socket_fd, (struct sockaddr*)&address, sizeof(address));
    if (bind_result == 0)
    {
        printf("Bind successfull.\n");
    }

    int listen_result = listen(server_socket_fd, 10);

    startAcceptingIncomingConnections(server_socket_fd);

    shutdown(server_socket_fd, SHUT_RDWR);
    
    return 0;
}

