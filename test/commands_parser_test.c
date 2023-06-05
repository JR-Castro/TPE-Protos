#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include <buffer.h>
#include <macros.h>
#include <logger.h>
#include <parser.h>
#include <commands_parser.h>
#include "../src/parsers/commands_parser.c"

START_TEST (test_parse_valid_username) {
    struct command_parser *p = emalloc(sizeof(struct command_parser));
    p->state = CMD_DISPATCH;
    p->commands_queue = newQueue(sizeof(struct command *));
    p->available_commands_queue = newQueue(sizeof(struct command_description));
    ck_assert_ptr_ne(NULL, p);
    ck_assert_ptr_ne(NULL, p->commands_queue);
    ck_assert_ptr_ne(NULL, p->available_commands_queue);

    const struct parser_definition user = parser_utils_strcmpi("USER ");
    commands[CMD_USER].parser = parser_init(parser_no_classes(), &user);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_USER]));

    const struct parser_definition pass = parser_utils_strcmpi("PASS ");
    commands[CMD_PASS].parser = parser_init(parser_no_classes(), &pass);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_PASS]));

    const struct parser_definition stat = parser_utils_strcmpi("STAT\r\n");
    commands[CMD_STAT].parser = parser_init(parser_no_classes(), &stat);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_STAT]));

    const struct parser_definition list = parser_utils_strcmpi("LIST ");
    commands[CMD_LIST].parser = parser_init(parser_no_classes(), &list);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_LIST]));

    const struct parser_definition retr = parser_utils_strcmpi("RETR ");
    commands[CMD_RETR].parser = parser_init(parser_no_classes(), &retr);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_RETR]));

    const struct parser_definition dele = parser_utils_strcmpi("DELE ");
    commands[CMD_DELE].parser = parser_init(parser_no_classes(), &dele);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_DELE]));

    const struct parser_definition noop = parser_utils_strcmpi("NOOP\r\n");
    commands[CMD_NOOP].parser = parser_init(parser_no_classes(), &noop);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_NOOP]));

    const struct parser_definition quit = parser_utils_strcmpi("QUIT\r\n");
    commands[CMD_QUIT].parser = parser_init(parser_no_classes(), &quit);
    p->available_commands_queue = add(p->available_commands_queue, &(commands[CMD_QUIT]));

    ck_assert_int_eq(N(commands), p->available_commands_queue->size);

    struct buffer buf;
    buffer *b = &buf;
    uint8_t direct_buff[512];
    buffer_init(&buf, N(direct_buff), direct_buff);
    size_t wbytes = 0, rbytes = 0;
    uint8_t *ptr = buffer_write_ptr(b, &wbytes);
    ck_assert_uint_eq(512, wbytes);

    uint8_t user_cmd_write [] = {
            'U', 'S', 'E', 'R', ' ', 'p', 'r', '0', 't', '0', 's', '\r', '\n'
    };
    memcpy(ptr, user_cmd_write, sizeof(user_cmd_write));
    buffer_write_adv(b, sizeof(user_cmd_write));

    while(buffer_can_read(b)) {
     do {
         uint8_t c = buffer_read(b);
         log(DEBUG, "Read char: %c", c);
         p->state = command_parser_feed(p, c);
     } while (p->state == CMD_MAYEQ);

     if (p->state == CMD_INVALID) {
        log(DEBUG, "Invalid command");
         struct command *invalid_cmd = (struct command *) peekTail(p->commands_queue);
         const char *errorMsg = "Invalid command";
         invalid_cmd->args = (uint8_t *) errorMsg;
     } else if (p->state == CMD_ARGS) {
         // consume the rest of the buffer until '\r\n'
         struct command *cmd = (struct command *) peekTail(p->commands_queue);
         uint8_t args[cmd->description->args_max];
         size_t *readBytes;
         bool crlf = consume_until_crlf(b, args, readBytes);
         if (crlf) {
             // valid input saves read args in cmd->args
             cmd->args = emalloc(N(args));
             strcpy(cmd->args, args);
             p->state = buffer_can_read(b) ? CMD_DISPATCH : CMD_OK;
         } else {
             // invalid input
             // change command state to invalid, saves error message in args
             memset(args, 0, N(args));
             const char *errorMsg = "Invalid args for command";
             strcpy((char *)args, errorMsg);
             cmd->description->type = CMD_ERROR;
         }
         log(DEBUG, "Command args: %s", args);
     }
    }
    printParsedCommandsNames(p->commands_queue);
    struct command *parsed_cmd = (struct command *) peekTail(p->commands_queue);
    ck_assert_str_eq("USER ",  parsed_cmd->description->name);
    log(DEBUG, "Command args: %s", parsed_cmd->args);
    ck_assert_str_eq("pr0t0s", parsed_cmd->args);

} END_TEST


Suite *
suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("cmd_parser_utils");

    tc = tcase_create("cmd_parser_utils");

    tcase_add_test(tc, test_parse_valid_username);
    suite_add_tcase(s, tc);

    return s;
}

int
main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
