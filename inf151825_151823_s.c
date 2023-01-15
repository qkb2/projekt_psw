#include "inf151825_151823.h"

// global values are OK here bc this file doesn't connect to any other one
int user_pids[MAX_USERS]; // if user_pids[i] == 0 then that user didn't log in
char* user_nicks[MAX_USERS];
char* user_pswds[MAX_USERS];

int user_ipcs[MAX_USERS];

int bad_pids[MAX_BAD_PIDS];
int bad_pid_strikes[MAX_BAD_PIDS];

int groups[MAX_GROUPS];
char* group_names[MAX_GROUPS];
int group_user_matrix[MAX_GROUPS][MAX_USERS];

int server_id;

void signal_handler(int signo) {
    printf("...closing server...\n");
    msgctl(server_id, IPC_RMID, NULL);
    signal(SIGINT,SIG_DFL);
    raise(SIGINT);
}

int options_switch(int my_key) { 
    // switch (expression)
    // {
    // case /* constant-expression */:
    //     /* code */
    //     break;
    
    // default:
    //     break;
    // }

    return 0;
}

int log_user(LBUF client_msg, int usersLoaded, int *last_bad_pid, int *user_id) {
    for (int i = 0; i < usersLoaded; i++) {
        // check if nick is like user nick, if so then check the pswd
        if (strcmp(user_nicks[i], client_msg.nick) != 0) continue;     
        if (strcmp(user_pswds[i], client_msg.pswd) != 0) {
            // Wrong pswd
            // table for PIDs w/ bad pswd
            for (int j = 0; j < *last_bad_pid; j++) {
                if (bad_pids[j] == client_msg.pid) {
                    (bad_pid_strikes[j])++;
                    if (bad_pid_strikes[j] == CHANCES_TO_LOGIN-1) return -3; // user blocked
                    return -1; // bad pswd
                } 
                break;
            }

            bad_pids[(*last_bad_pid)++] = client_msg.pid;
            return -1; // bad pswd
        }
        *user_id = i;
        return 1;  // Logged in
    }
    return -2; // User not found
}

int load_users() {
    int config_fd = open("config.txt", O_RDONLY);
    int iUser = 0;
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
        if (n <= 0) return iUser;  // End of config file

        
        char *token = strtok (nick_pswd_str, "-");
        char *nick_pswd[2];

        int i = 0;
        while (token != NULL) {
            nick_pswd[i++] = token;
            token = strtok(NULL, "-");
        }
        user_nicks[iUser] = malloc(strlen(nick_pswd[0])+1);
        user_pswds[iUser] = malloc(strlen(nick_pswd[1])+1);
        strcpy(user_nicks[iUser], nick_pswd[0]);
        strcpy(user_pswds[iUser], nick_pswd[1]);
        iUser += 1;
    }
}

int main(int argc, char const *argv[]) {
    signal(SIGINT, signal_handler);
    printf("server running...\n");

    int usersLoaded = load_users(); 
    int last_bad_pid = 0;   
    for(int i = 0; i < usersLoaded; i++) {
        printf("Login: %s\tPswd: %s\n", user_nicks[i], user_pswds[i]);
    }

    // create ipc fifo for users to log into (connect to server)
    server_id = msgget(2000, 0666 | IPC_CREAT);
    // printf("server id = %d\n", server_id);

    // write server id to shared.txt for clients to read from
    int file_id = creat("shared.txt", 0666);
    write(file_id, &server_id, sizeof(server_id));
    close(file_id);
    

    LBUF login_msg;
    LCBUF login_to_send;
    MBUF msg_to_client;
    MBUF msg_from_client;

    printf("Setup succesful... server ID:%d\n", server_id);

    while (1) {
        printf("...\n");
        int test = msgrcv(server_id, &login_msg, LMSG_SIZE, 1, IPC_NOWAIT); // 1 - only reads login msgs
        sleep(1);
        if (test > 0) {
            printf("someone's logging in\n");
            int user_id = 0;
            int feedback = log_user(login_msg, usersLoaded, &last_bad_pid, &user_id);
            login_to_send.mtype = login_msg.pid;
            login_to_send.msgCode = feedback; // 1 - OK, -1 - BAD PSWD, -2 - BAD USER -3 - BLOCKED USER
            printf("feedback = %d", feedback);
            if (feedback == 1) {
                int client_ipc_id = msgget(2000+login_msg.pid, 0666 | IPC_CREAT);
                user_ipcs[user_id] = client_ipc_id;
                user_pids[user_id] = login_msg.pid;
                login_to_send.ipcID = client_ipc_id;
            
            }
            msgsnd(server_id, &login_to_send, LCMSG_SIZE, IPC_NOWAIT);
        }
        // printf("feedback: %d\n", feedback);
        // printf("rcvd...\n");

        for (int iUsers = 0; iUsers < usersLoaded; iUsers++) {
            if (user_pids[iUsers] == 0) continue;
            printf("now serving %d\n", user_pids[iUsers]);
            // serving all user IPC FIFOs
            msgrcv(user_ipcs[iUsers], &msg_from_client, MSG_SIZE, -11, IPC_NOWAIT); // -11 is 1 to 11
            options_switch(user_ipcs[iUsers]);
            // msgsnd(user_ipcs[iUsers], &msg_to_client, MSG_SIZE, IPC_NOWAIT);
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
