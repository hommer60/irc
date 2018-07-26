#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "irc.h"
#include "parser.h"


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
    int at_end = 0;
    printf("NUM FULL MESSAGES = %d\n", numMessages);
    int starting_null = 0;
    int buf_len = strlen(buffer) - 1;
    if(buffer[buf_len] == '\r' && string[0] == '\n')
    {
        numMessages++;
    }
    irc_message** messages = NULL;
    if(numMessages > 0)
        messages = (irc_message**)malloc(sizeof(irc_message*)*numMessages);
    if(strlen(buffer) > 0){
        //incomplete message, fill up buffer
        
        //incoming message starts with null character
        if(strlen(string) >= 2 && string[0] == '\r' && string[1] == '\n'){
            starting_null = 1;
            if(strlen(string) == 2)
                at_end = 1;
            strcat(buffer, "\r\n");
        }
        else{
            tmp = strtok_r(strdup(string), "\r\n", &tmp_buf);
            if(tmp)
                strcat(buffer, strdup(tmp));
            else
                at_end = 1;
        }
        if(numMessages != 0){
            messages[0] = parse_message(buffer);
            //flush buffer
            memset(buffer, 0, 512);
            int i = 1;
            while(i < numMessages){
                if(starting_null == 1){
                    tmp = strtok_r(strdup(string), "\r\n", &tmp_buf);
                    starting_null = 0;
                }
                else
                    tmp = strtok_r(NULL, "\r\n", &tmp_buf);
                messages[i] = parse_message(tmp);
                i++;
            }
            if(at_end == 0)
                tmp = strtok_r(NULL, "\r\n", &tmp_buf);
            if(tmp){
                strcat(buffer, strdup(tmp));
            }
        }
        if(string[strlen(string) - 1] == '\r'){
            int buf_len = strlen(buffer);
            buffer[buf_len] = '\r';
        }
        if(numMessages == 0)
            return NULL;
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

void process_messages(irc_wrapper* iw, state* current_state)
{
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