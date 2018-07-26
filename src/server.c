#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "server.h"
#include "irc.h"
#include "messenger.h"
#include "parser.h"

void server(char* port) 
{
    int serverSocket;
    int clientSocket;

    struct sockaddr_in serverAddr, clientAddr;

    int yes = 1;

    socklen_t sinSize = sizeof(struct sockaddr_in);

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(serverSocket == -1)
    {
        perror("Could not open socket");
        exit(-1);
    }

    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Socket setsockopt() failed");
        close(serverSocket);
        exit(-1);
    }

    if(bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Socket bind() failed");
        close(serverSocket);
        exit(-1);
    }

    if(listen(serverSocket, 5) == -1)
    {
        perror("Socket listen() failed");
        close(serverSocket);
        exit(-1);
    }

    fprintf(stderr, "Waiting for a connection... ");

    if((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &sinSize)) == -1)
    {
        perror("Socket accept() failed");
        close(serverSocket);
        exit(-1);
    }

    char incoming_msg[512];
    char buffer[512];
    memset(buffer, 0, 512);
    irc_wrapper* iw = NULL;
    state* current_state = (state *)malloc(sizeof(state));
    current_state->nick = NULL;
    current_state->user_nick = NULL;

    while(1){

        printf("CUR BUFFER = %s\n", buffer);
        iw = NULL;
        memset(incoming_msg, 0, 512);
        if(read(clientSocket, incoming_msg, 512) <= 0)
        {
            perror("Socket recv() failed");
            close(serverSocket);
            close(clientSocket);
            exit(0);
        }

        iw = parse_messages(incoming_msg, buffer);
        if(iw != NULL){
            print_irc_wrapper(iw);
            memset(incoming_msg, 0, 512);
            process_messages(iw, current_state);
            print_state(current_state);
            if(ready_state(current_state) == 1){
                fill_msg(incoming_msg, current_state, serverAddr, clientAddr);
                if(send(clientSocket, incoming_msg, strlen(incoming_msg), 0) <= 0)
                {
                    perror("Socket send() failed");
                    close(serverSocket);
                    close(clientSocket);
                    exit(-1);
                }
            }
            free_irc_wrapper(iw);
        }

    }
    
    close(clientSocket);
    fprintf(stderr, "Message sent!\n");
    close(serverSocket);
}

void test_loop(char** messages, int num_messages){
    char incoming_msg[512];
    char buffer[512];
    memset(buffer, 0, 512);
    irc_wrapper* iw = NULL;
    state* current_state = (state *)malloc(sizeof(state));
    current_state->nick = NULL;
    current_state->user_nick = NULL;

    for(int i = 0; i < num_messages; i++){
        printf("CUR BUFFER = %s\n", buffer);
        iw = NULL;
        memset(incoming_msg, 0, 512);
        strncpy(incoming_msg, messages[i], strlen(messages[i]));
        iw = parse_messages(incoming_msg, buffer);
        print_irc_wrapper(iw);
        if(iw != NULL){
            memset(incoming_msg, 0, 512);
            process_messages(iw, current_state);
            print_state(current_state);
            if(ready_state(current_state) == 1){
                fill_msg_test(incoming_msg, current_state);
                printf("SENDING MESSAGE: %s\n", incoming_msg);
            }
        }
        free_irc_wrapper(iw);
    }
}
