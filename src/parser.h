#include "irc.h"

#ifndef CHIRC_PARSER_H_
#define CHIRC_PARSER_H_

irc_message* parse_message(char *string);
irc_wrapper* parse_messages(char *string, char *buffer);
void process_messages(irc_wrapper* iw, state* current_state);

#endif /* CHIRC_PARSER_H_ */
