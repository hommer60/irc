#ifndef CHIRC_IRC_H_
#define CHIRC_IRC_H_

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


#endif /* CHIRC_IRC_H_ */