#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char** args;
    char* message;
} irc_message;


irc_message* split_message(char* string)
{
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
        stringList[i] = (char*)malloc(strlen(tmp) + 1);
        strcpy(stringList[i], tmp);
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
        args[i] = (char*)malloc(strlen(tmp) + 1);
        strcpy(args[i], tmp);
        i++;
        tmp = strtok(NULL, ":");
    }
    msg->args = args;
    msg->message = stringList[1];
    // free(stringList[0]);
    // free(stringList[1]);
    // free(stringList);
    return msg;
}

void fill_msg(char[] str, char* nick, sockaddr_in server, sockaddr_in client)
{
    bzero(str, 0);
    unsigned long server_address = server.sin_addr.s_addr;
    unsigned long client_address = client.sin_addr.s_addr;
    sprintf(msg, ":%lu 001 %s :Welcome to the Internet Relay Network %s!%s@%lu", server_address, nick, nick, nick, client_address);
    return;

}

int main()
{
    char test_str[] = "NICK omar : omar salemohamed";
    irc_message* res = split_message(test_str);
    printf("%s\n", res->message);
    char* arg = res->args[0];
    int i = 0;
    while(arg != NULL)
    {
        printf("%s\n", arg);
        if(strcmp(arg, "omar") == 0)
        {
            printf("YES");
        }
        i++;
        arg = res->args[i];
    }
}