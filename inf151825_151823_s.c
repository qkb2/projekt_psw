#include "inf151825_151823.h"

// global values are OK here bc this file doesn't connect to any other one
int user_pids[MAX_USERS];
char* user_nicks[MAX_USERS];

int bad_pids[MAX_BAD_PIDS];
int bad_pid_strikes[MAX_BAD_PIDS];

int groups[MAX_GROUPS];
int group_user_matrix[MAX_GROUPS][MAX_USERS];

int options_switch() {}

int log_user(LBUF client_msg) {
    int config_fd = open("config.txt", O_RDONLY);
    while (1) {
        char buf;
        char nick_pswd_str[MSG_SIZE] = "";
        int n;
        while((n = read(config_fd, &buf, 1)) > 0) {
            if (buf == ';') {
                lseek(config_fd, 1, SEEK_CUR);
                break;
            }
            strncat(nick_pswd_str, &buf, 1);
        }
        if (n <= 0) return -2;  // End of config file (user not found)

        
        char *token = strtok (nick_pswd_str, "-");
        char *nick_pswd[2];

        int i = 0;
        while (token != NULL) {
            nick_pswd[i++] = token;
            token = strtok(NULL, "-");
        }
        
        // check if nick is like user nick, if so then check the pswd
        if (strcmp(nick_pswd[0], client_msg.nick) != 0) continue;     
        if (strcmp(nick_pswd[1], client_msg.pswd) != 0) {
            // Wrong pswd
            // table for PIDs w/ bad pswd
            return -1;
        }   
        else return 1;  // Logged in
    }
    // return -2; // User not found
}

int main(int argc, char const *argv[]) {
    printf("server running...\n");

    // create ipc fifo for users to log into (connect to server)
    int server_id = msgget(2000, 0600 | IPC_CREAT);
    // printf("server id = %d\n", server_id);

    // write server id to shared.txt for clients to read from
    int file_id = creat("shared.txt", 0666);
    write(file_id, &server_id, sizeof(server_id));
    close(file_id);
    

    LBUF login_msg;
    LCBUF login_to_send;
    MSGBUF msg_to_client;
    MSGBUF msg_from_client;

    while (1) {
        // printf("...\n");
        msgrcv(server_id, &login_msg, LMSG_SIZE, 1, IPC_NOWAIT); // 1 - only reads login msgs
        int feedback = log_user(login_msg);
        login_to_send.mtype = login_msg.pid;
        login_to_send.msgCode = feedback; // 1 - OK, -1 - BAD PSWD, -2 - BAD USER -3 - BLOCKED USER
        msgsnd(server_id, &login_to_send, LCMSG_SIZE, 0);
        printf("feedback: %d\n", feedback);
        // printf("rcvd...\n");

        for (int iUsers = 0; iUsers < MAX_USERS; iUsers++) {
            // serving all user IPC FIFOs
            msgrcv(server_id, &msg_from_client, MSG_SIZE, 1, IPC_NOWAIT); // change 1 to user_id
        }
        
        // switch should be moved to a safe function like options_switch()
        // switch (client_msg.mtype) {
        //     case 1: // login
        //         // printf("feedback...\n");

        //         break;
        //     case 2: // logout

        //         break;
        //     default:
        //         printf("default\n");
        //         continue;
        // }
    }   
        // read msgs from main ipc fifo (create users, log users in)
        // write to main
        // check all the users ipc fifos (act on all requests)
        // write to all user ipc that need writing to

    
    
    // TODO make server close itself automatically w/ std kill signals (9, 15)
    // and also unlink that shared.txt while killing itself
    // also inform clients that server is down

    unlink("shared.txt");
    return 0;
}
