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

int user_blocks_user[MAX_USERS][MAX_USERS];
int user_blocks_group[MAX_USERS][MAX_GROUPS];

int server_id;

void signal_handler(int signo) {
    printf("...closing server...\n");
    msgctl(server_id, IPC_RMID, NULL);
    signal(SIGINT,SIG_DFL);
    raise(SIGINT);
}

int get_user_id() {
    return 0;
}

int get_group_id() {
    return 0;
}

int list_active_users(char* list) {
    return 0;
}

int list_groups(char* list) {
    return 0;
}

int list_group_users(char* list, char* group) {
    return 0;
}

int atJoin_group(int opt, int my_id, char* group) {
    return 0;
}

int atMute_user(int opt, int my_id, char* user) {
    return 0;
}

int atMute_group(int opt, int my_id, char* group) {
    return 0;
}

int logout_user(int my_id) {
    return 0;
}

int long_msg_sender(int my_key, MBUF msg) {
    if (msg.code == 2) {
        
    }
    else if (msg.code == 3) {

    }

    return -1;
}

int options_switch(int my_key, SMBUF msg, int my_id) {
    // list msgs need MBUF
    if (msg.mtype < 5) {
        MBUF lmsg;
        switch (msg.mtype) {
            case 2:
                // list all users
                list_active_users(lmsg.longMsg);
                break;
                case 3:
                    // list all groups
                    list_groups(lmsg.longMsg);
                    break;
                case 4:
                    // list users by group X
                    list_group_users(lmsg.longMsg, msg.name);
                    break;
                default:
                    return -1;
        }
    }

    switch (msg.mtype) {
    case 5:
        // join group X
        atJoin_group(1, my_id, msg.name);
        break;
    case 6:
        // leave group X
        atJoin_group(0, my_id, msg.name);
        break;

    // mute NAME:
    // if opt==2 then the user is muted (not default, default is 0 - not muted)
    case 7:
        // mute user X
        atMute_user(2, my_id, msg.name);
        break;
    case 8:
        // mute group X
        atMute_group(2, my_id, msg.name);
        break;
    case 9:
        // unmute user X
        atMute_user(0, my_id, msg.name);
        break;
    case 10:
        // unmute group X
        atMute_group(0, my_id, msg.name);
        break;
    case 11:
        // logout
        logout_user(my_id);
        break;
    default:
        return -1;
    }

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
    printf("Users loaded (%d):\n", usersLoaded);
    int last_bad_pid = 0;   
    for(int i = 0; i < usersLoaded; i++) {
        printf("Login: %s\tPswd: %s\n", user_nicks[i], user_pswds[i]);
    }

    // create ipc fifo for users to log into (connect to server)
    server_id = msgget(PROGRAM_KEY, 0666 | IPC_CREAT);
    // printf("server id = %d\n", server_id);

    // write server id to shared.txt for clients to read from
    // int file_id = creat("shared.txt", 0666);
    // write(file_id, &server_id, sizeof(server_id));
    // close(file_id);
    

    LBUF login_rcvd;
    LCBUF login_to_send;
    SMBUF short_msg;
    MBUF long_msg;

    printf("Setup succesful... server ID:%d\n", server_id);

    while (1) {
        printf("...\n");
        int test = msgrcv(server_id, &login_rcvd, LMSG_SIZE, 1, IPC_NOWAIT); // 1 - only reads login msgs
        sleep(1);
        if (test > 0) {
            printf("someone's logging in\n");
            int user_id = 0;
            int feedback = log_user(login_rcvd, usersLoaded, &last_bad_pid, &user_id);
            login_to_send.mtype = login_rcvd.pid;
            login_to_send.msgCode = feedback; // 1 - OK, -1 - BAD PSWD, -2 - BAD USER -3 - BLOCKED USER
            printf("feedback = %d", feedback);
            if (feedback == 1) {
                int client_ipc_id = msgget(2000+login_rcvd.pid, 0666 | IPC_CREAT);
                user_ipcs[user_id] = client_ipc_id;
                user_pids[user_id] = login_rcvd.pid;
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
            int ibSMSG = msgrcv(user_ipcs[iUsers], &short_msg, SMSG_SIZE, -11, IPC_NOWAIT);
            // -11 is 1 to 11
            if (ibSMSG > 0) options_switch(user_ipcs[iUsers], short_msg, iUsers);

            // code 12 (send msg to user/group) gets its own options bc of its size
            int ibMSG = msgrcv(user_ipcs[iUsers], &long_msg, MSG_SIZE, 12, IPC_NOWAIT);
            if (ibMSG > 0) long_msg_sender(user_ipcs[iUsers], long_msg);

            // msgsnd(user_ipcs[iUsers], &msg_to_client, MSG_SIZE, IPC_NOWAIT);
        }
        // switch case in function_switch()
    }   
        // read msgs from main ipc fifo (create users, log users in)
        // write to main
        // check all the users ipc fifos (act on all requests)
        // write to all user ipc that need writing to

    
    
    // DONE make server close itself automatically w/ std kill signals (9, 15)
    // also inform clients that server is down (idk if it's necessary)
    return 0;
}
