#ifndef CHIRC_MESSENGER_H_
#define CHIRC_MESSENGER_H_

int numFullMessages(char* string);
void fill_msg(char* outgoing_msg, state* s, struct sockaddr_in server, struct sockaddr_in client);
void fill_msg_test(char* outgoing_msg, state* s);
int ready_state(state* s);
void free_irc_message(irc_message* msg);
void free_irc_wrapper(irc_wrapper* iw);
void print_state(state* s);
void print_msg(irc_message* msg);
void print_irc(irc_message** messages, int numMessages);
void print_irc_wrapper(irc_wrapper* iw);


#endif /* CHIRC_MESSENGER_H_ */