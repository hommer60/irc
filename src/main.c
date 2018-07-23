/*
 *
 *  main() code for chirc project
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "log.h"


typedef struct {
    char nick[50];
    char name[100];
    struct user* next;
} user;

typedef struct {
    char** args;
    char* message;
} irc_message;


irc_message* split_message(char* string, char* nick)
{
    printf("NICK BEFORE\n");
    if(nick != NULL){
        printf("%s", nick);
    }
    irc_message *msg = (irc_message*)malloc(sizeof(irc_message));
    char* tmp = NULL;
    char** stringList = (char**)malloc(2 * sizeof(char*));
    for(int i = 0; i < 2; i++)
    {
        stringList[i] = NULL;
    }
    tmp = strtok(string, ":");
    int i = 0;
    while(tmp != NULL)
    {
        printf("i = %d and tmp = %s", i, tmp);
        stringList[i] = strdup(tmp);
        i++;
        tmp = strtok(NULL, ":");
    }
    char** args = (char**)malloc(4*sizeof(char*));
    char* first_part = stringList[0];
    tmp = NULL;
    for(int i = 0; i < 4; i++)
    {
        args[i] = NULL;
    }
    tmp = strtok(first_part, " ");
    i = 0;
    while(tmp != NULL)
    {
        args[i] = strdup(tmp);
        i++;
        tmp = strtok(NULL, " ");
    }
    msg->args = args;
    if(stringList[1] != NULL)
        msg->message = strdup(stringList[1]);
    // free(stringList[0]);
    // free(stringList[1]);
    free(tmp);
    free(stringList[0]);
    free(stringList[1]);
    free(stringList);

    printf("NICK AFTER\n");
    printf("%s", nick);
    return msg;
}

void fill_msg(char str[], char* nick, struct sockaddr_in server, struct sockaddr_in client)
{
    char servAdr[INET_ADDRSTRLEN];
    char cliAdr[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(server.sin_addr), servAdr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(client.sin_addr), cliAdr, INET_ADDRSTRLEN);
    sprintf(str, ":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n", servAdr, nick, nick, nick, cliAdr);
    return;

}

int main(int argc, char *argv[])
{
    int opt;
    char *port = "6667", *passwd = NULL;
    int verbosity = 0;

    while ((opt = getopt(argc, argv, "p:o:vqh")) != -1)
        switch (opt)
        {
        case 'p':
            port = strdup(optarg);
            break;
        case 'o':
            passwd = strdup(optarg);
            break;
        case 'v':
            verbosity++;
            break;
        case 'q':
            verbosity = -1;
            break;
        case 'h':
            fprintf(stderr, "Usage: chirc -o PASSWD [-p PORT] [(-q|-v|-vv)]\n");
            exit(0);
            break;
        default:
            fprintf(stderr, "ERROR: Unknown option -%c\n", opt);
            exit(-1);
        }

    if (!passwd)
    {
        fprintf(stderr, "ERROR: You must specify an operator password\n");
        exit(-1);
    }

    /* Set logging level based on verbosity */
    switch(verbosity)
    {
    case -1:
        chirc_setloglevel(QUIET);
        break;
    case 0:
        chirc_setloglevel(INFO);
        break;
    case 1:
        chirc_setloglevel(DEBUG);
        break;
    case 2:
        chirc_setloglevel(TRACE);
        break;
    default:
        chirc_setloglevel(INFO);
        break;
    }

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

    char str[512];
    char* nick = NULL;
    char* name = NULL;
    irc_message* msg = NULL;
    int recv_nick = 0;
    while(1){

        memset(str, 0, 512);
        if(read(clientSocket, str, 512) <= 0)
        {
            perror("Socket recv() failed");
            close(serverSocket);
            close(clientSocket);
            exit(-1);
        }
        msg = split_message(str, nick);
        printf("HERE AND MESSAGE : %s\n", msg->message);
        memset(str, 0, 512);
        if(msg->args != NULL)
        {
            if(nick != NULL){
                printf("the name is : %s\n", nick);
            }
            char** args = msg->args;
            if(args[0] != NULL)
            {
                if(strcmp(args[0], "NICK") == 0)
                {
                    nick = strdup(args[1]);
                    printf("::NAME IS ::%s", nick);
                    printf("::LENGTH IS ::%d", strlen(nick));
                    nick[strlen(nick) - 2] = '\0';
                    recv_nick = 1;

                }
                else if(strcmp(args[0], "USER") == 0)
                {
                    if(msg->message != NULL)
                    {
                        name = strdup(msg->message);
                        printf("OOMAMR");
                        fill_msg(str, nick, serverAddr, clientAddr);
                        printf("BUT GETTING HERE");
                        printf("message is %s", msg->message);
                        fflush(stdout);
                        if((msg->message[strlen(msg->message) -2] == '\r') && (msg->message[strlen(msg->message) - 1] == '\n'))
                        {
                            printf("NOT GETTING HERE");
                            fflush(stdout);
                            if(recv_nick == 1){
                                printf("SENDING");
                                fflush(stdout);
                                if(send(clientSocket, str, strlen(str), 0) <= 0)
                                {
                                    perror("Socket send() failed");
                                    close(serverSocket);
                                    close(clientSocket);
                                    exit(-1);
                                }
                            }
                        }
                    }
                }
            }
        }
        free(msg);
        msg = NULL;

    }


    close(clientSocket);
    fprintf(stderr, "Message sendt!\n");
    close(serverSocket);
    return 0;
}

