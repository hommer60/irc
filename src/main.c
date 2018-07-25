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
    char* command;
    char** params;
    char* payload;
} irc_message;

typedef struct {
    irc_message** messages;
    int num_messages;
} irc_wrapper;

typedef struct {
    char* nick;
    char* user_nick;
} state;

char* strtok_r(
    char *str, 
    const char *delim, 
    char **nextp)
{
    char *ret;
    if (str == NULL)
    {
        str = *nextp;
    }

    str += strspn(str, delim);
    if (*str == '\0')
    {
        return NULL;
    }

    ret = str;
    str += strcspn(str, delim);

    if (*str)
    {
        *str++ = '\0';
    }
    *nextp = str;
    return ret;
}

int numFullMessages(char* string)
{
    int numFullMessages = 0;
    for(int j = 0; j < strlen(string) - 1; j++)
    {
        if(string[j] == '\r' && string[j + 1] == '\n')
        {
            numFullMessages++;
        }
    }
    return numFullMessages;
}

irc_message* parse_message(char *string)
{
    irc_message* irc = (irc_message*)malloc(sizeof(irc_message));
    char* args = NULL;
    char* cmd = NULL;
    char* payload = NULL;
    char* tmp = NULL;
    tmp = strtok(string, ":");
    args = strdup(tmp);
    tmp = strtok(NULL, ":");
    if(tmp != NULL){
        payload = strdup(tmp);
        payload = strtok(payload, "\r\n");
    }
    else{
        args = strtok(args, "\r\n");
    }
    tmp = NULL;
    tmp = strtok(args, " ");
    cmd = strdup(tmp);
    char** params = (char**)malloc(sizeof(char*)*3);
    for(int i = 0; i < 3; i++){
        params[i] = NULL;
    }
    int counter = 0;
    tmp = strtok(NULL, " ");
    while(tmp != NULL){
        params[counter] = strdup(tmp);
        counter++;
        tmp = strtok(NULL, " ");
    }

    irc->command = cmd;
    irc->payload = payload;
    irc->params = params;
    return irc;
}

irc_wrapper* parse_messages(char *string, char *buffer)
{
    char* tmp = NULL;
    char* tmp_buf;
    int numMessages = numFullMessages(string);
    printf("NUM FULL MESSAGES = %d\n", numMessages);
    int starting_null = 0;
    irc_message** messages = NULL;
    if(numFullMessages > 0)
        messages = (irc_message**)malloc(sizeof(irc_message*)*numMessages);
    if(strlen(buffer) > 1){
        //incomplete message, fill up buffer
        
        //incoming message starts with null character
        if(strlen(string) >= 2 && string[0] == '\r' && string[1] == '\n'){
            starting_null = 1;
            strcat(buffer, "\r\n");
        }
        else{
            tmp = strtok_r(string, "\r\n", &tmp_buf);
            strcat(buffer, strdup(tmp));
        }
        if(numMessages == 0)
            return NULL;
        messages[0] = parse_message(buffer);
        //flush buffer
        memset(buffer, 0, 512);
        int i = 1;
        while(i < numMessages){
            if(starting_null == 1){
                tmp = strtok_r(string, "\r\n", &tmp_buf);
                starting_null = 0;
            }
            else
                tmp = strtok_r(NULL, "\r\n", &tmp_buf);
            messages[i] = parse_message(tmp);
            i++;
        }
        tmp = strtok_r(NULL, "\r\n", &tmp_buf);
        if(tmp)
            strcat(buffer, strdup(tmp));
    }
    else{
        tmp = strtok_r(string, "\r\n", &tmp_buf);
        int i = 0;
        while(i < numMessages){
            messages[i] = parse_message(tmp);
            i++;
            tmp = strtok_r(NULL, "\r\n", &tmp_buf);
        }
        if(tmp)
            strcat(buffer, strdup(tmp));
    }
    //free(tmp);
    //free(tmp_buf);
    irc_wrapper* iw = (irc_wrapper*)malloc(sizeof(irc_wrapper));
    iw->messages = messages;
    iw->num_messages = numMessages;
    return iw;
}

void fill_msg(char* outgoing_msg, state* s, struct sockaddr_in server, struct sockaddr_in client)
{
    char servAdr[INET_ADDRSTRLEN];
    char cliAdr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server.sin_addr), servAdr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(client.sin_addr), cliAdr, INET_ADDRSTRLEN);
    printf("IN FILL MESG\n");
    printf("outgoing msg should be empty... it is %s\n", outgoing_msg);
    printf("NICK: %s, USER NICK: %s\n", s->nick, s->user_nick);
    sprintf(outgoing_msg, ":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n", servAdr, s->nick, s->nick, s->user_nick, cliAdr);
    return;
}

void fill_msg_test(char* outgoing_msg, state* s)
{
    printf("IN FILL MESG\n");
    printf("outgoing msg should be empty... it is %s\n", outgoing_msg);
    printf("NICK: %s, USER NICK: %s\n", s->nick, s->user_nick);
    sprintf(outgoing_msg, ":001 %s :Welcome to the Internet Relay Network %s!%s@\r\n", s->nick, s->nick, s->user_nick);
    return;
}

int ready_state(state* s)
{
    if(s->nick != NULL && s-> user_nick != NULL){
        return 1;
    }
    else{
        return 0;
    }
}

void free_irc_message(irc_message* msg)
{
    free(msg->command);
    free(msg->payload);
    for(int i = 0; i < 3; i++){
        free(msg->params[i]);
    }
    free(msg);
    return;
}

void free_irc_wrapper(irc_wrapper* iw)
{
    for(int i = 0; i < iw->num_messages; i++)
    {
        free_irc_message(iw->messages[i]);
    }
    free(iw->messages);
    free(iw);
    return;
}

void process_messages(irc_wrapper* iw, state* current_state)
{
    // char new_nick[512];
    // char new_user_nick[512];
    int found_nick = 0;
    int found_user_nick = 0;

    for(int i = iw->num_messages - 1; i >= 0; i--)
    {
        irc_message* msg = iw->messages[i];
        if(found_nick == 1 && found_user_nick == 1){
            break;
        }
        else if((strcmp(msg->command,"NICK") == 0) && found_nick == 0){
            free(current_state->nick);
            current_state->nick = strdup(msg->params[0]);
            found_nick = 1;
        }
        else if((strcmp(msg->command, "USER") == 0) && found_user_nick == 0){
            free(current_state->user_nick);
            current_state->user_nick = strdup(msg->params[0]);
            found_user_nick = 1;
        }
    }
}


void print_state(state* s)
{
    printf("CURRENT NICK: %s\n", s->nick);
    printf("CURRENT USER NICK: %s\n", s->user_nick);
    return;
}
void print_msg(irc_message* msg)
{
    if(msg->command)
        printf("Command = %s\n", msg->command);
    if(msg->payload)
        printf("Payload = %s\n", msg->payload);
    for(int i = 0; i < 3; i++){
        printf("Arg %d = %s\n", i, msg->params[i]);
    }
}

void print_irc(irc_message** messages, int numMessages)
{
    for(int i = 0; i < numMessages; i++){
        print_msg(messages[i]);
    }
    return;
}

void print_irc_wrapper(irc_wrapper* iw)
{
    printf("NUM MESSAGES: %d\n", iw->num_messages);
    print_irc(iw->messages, iw->num_messages);
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

// int main(int argc, char *argv[])
// {
//     char** test_messages = (char**)malloc(sizeof(char*)*4);
//     char msg1[512] = "NICK nick4";
//     char msg2[512] = "2\r\n";
//     char msg3[512] = "USER user4";
//     char msg4[512] = "2 * * :User Forty Two\r\n";
//     test_messages[0] = msg1;
//     test_messages[1] = msg2;
//     test_messages[2] = msg3;
//     test_messages[3] = msg4;
//     test_loop(test_messages, 4);
//     return 0;
// }

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
            exit(-1);
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
    return 0;
}

