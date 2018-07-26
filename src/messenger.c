#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>

#include "irc.h"
#include "messenger.h"

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