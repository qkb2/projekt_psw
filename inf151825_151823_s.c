#include "inf151825_151823.h"

int log_user(MSGBUF client_msg) {
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
        if (strcmp(nick_pswd[0], client_msg.shortMsg) != 0) continue;
        if (strcmp(nick_pswd[1], client_msg.longMsg) != 0) return -1;   // Wrong pswd
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
    

    MSGBUF client_msg;
    while (1) {
        // printf("...\n");
        msgrcv(server_id, &client_msg, MSG_SIZE, 0, 0);
        // printf("rcvd...\n");
        switch (client_msg.mtype) {
            case 1: // login
                // printf("feedback...\n");
                int feedback = log_user(client_msg);
                // TODO case feedback
                printf("feedback: %d\n", feedback);
                break;
            case 2: // logout

                break;
            default:
                printf("default\n");
                continue;
        }
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
