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

int user_blocks_user[MAX_USERS][MAX_USERS]; // if user is blocked then u_block_u[i,j] == 2
int user_blocks_group[MAX_USERS][MAX_GROUPS]; // same as above

// server setup vars
int server_id;
int users_loaded;
int groups_loaded;

// msgbufs
LBUF login_rcvd;
LCBUF login_to_send;
SMBUF short_msg;
MBUF long_msg;

void signal_handler(int signo) {
    printf("...closing server...\n");
    msgctl(server_id, IPC_RMID, NULL);
    signal(SIGINT,SIG_DFL);
    raise(SIGINT);
}

int get_user_ipc(char* name) {
    int ipc = 0;
    for (int i = 0; i < users_loaded; i++) {
        if (strcmp(name, user_nicks[i]) == 0) {
            ipc = user_ipcs[i];
            return ipc;
        }
    }
    return -1; // NOT FOUND
}

int get_user_id(char* name) {
    for (int i = 0; i < users_loaded; i++) {
        if (strcmp(name, user_nicks[i]) == 0) {
            return i;
        }
    }
    return -1; // NOT FOUND
}

int get_group_id(char* name) {
    for (int i = 0; i < groups_loaded; i++) {
        if (strcmp(name, group_names[i]) == 0) 
            return i;
    }
    return -1; // NOT FOUND
}

void list_active_users(char* list) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (user_pids[i] > 0) {
            strcat(list, user_nicks[i]);
            printf("%s\n", user_nicks[i]);
            strcat(list, "\n");
        }
    }
}

int list_groups(char* list) {
    for (int i = 0; i < groups_loaded; i++) {
            strcat(list, group_names[i]);
            strcat(list, "\n");
    }
    return 0;
}

int list_group_users(char* list, char* group) {
    int group_id = get_group_id(group);
    printf("Past group_id\n");
    if (group_id == -1) return -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (group_user_matrix[group_id][i] == 1) {
            strcat(list, user_nicks[i]);
            strcat(list, "\n");
        }
    } 
    return 0;
}

int atJoin_group(int opt, int my_id, char* group) {
    int group_id = get_group_id(group);
    if (group_id == -1) return -1;
    group_user_matrix[group_id][my_id] = opt;
    return 0;
}

int atMute_user(int opt, int my_id, char* user) {
    int user_id = get_user_id(user);
    if (user_id == -1) return -1;
    user_blocks_user[my_id][user_id] = opt;
    return 0;
}

int atMute_group(int opt, int my_id, char* group) {
    int group_id = get_group_id(group);
    if (group_id == -1) return -1;
    user_blocks_group[my_id][group_id] = opt;
    return 0;
}

int logout_user(int my_id) {
    user_ipcs[my_id] = 0;
    user_pids[my_id] = 0;
    return 0;
}

int long_msg_sender(int my_key, int my_id) {
    printf("sending msg\n");
    short_msg.mtype = 101;
    if (long_msg.code == 2) {
        int user_id = get_user_id(long_msg.shortMsg);
        int user_ipc = get_user_ipc(long_msg.shortMsg);
        if (user_ipc > 0) {
            if (user_blocks_user[user_id][my_id] == 2) {
                // user blocked
                // code = block
                short_msg.code = -5; // blocked
            } else {
                // send msg to user
                // code = ok
                short_msg.code = 1; // OK
                long_msg.mtype = 99;
                strncpy(long_msg.shortMsg, user_nicks[my_id], 16);
                printf("sending msg to user %s \n", user_nicks[user_id]);
                msgsnd(user_ipc, &long_msg, MSG_SIZE, IPC_NOWAIT);
                printf("sent \n");           
            }
        } else {
            short_msg.code = -1; // user not found
        }
    }
    else if (long_msg.code == 3) {
        int group_id = get_group_id(long_msg.shortMsg);
        if (group_id != -1) {
            // check if the sender is in group
            // send w/ for all users in group, make for all in v=group_id
            short_msg.code = 1;
            long_msg.mtype = 99;
            strncpy(long_msg.shortMsgB, long_msg.shortMsg, 16);
            strncpy(long_msg.shortMsg, user_nicks[my_id], 16);
            for (int i = 0; i < MAX_USERS; i++) {
                if (group_user_matrix[group_id][i] == 1 && user_ipcs[i] > 0) {
                    msgsnd(user_ipcs[i], &long_msg, MSG_SIZE, 0);
                }
            }
        } else {
            short_msg.code = -1; // group not found
        }
    }
    short_msg.mtype = 101;
    printf("sending feedback \n");
    msgsnd(my_key, &short_msg, SMSG_SIZE, IPC_NOWAIT);

    return -1;
}

int options_switch(int my_key, int my_id) {
    // list msgs need MBUF
    int err = 0;
    if (short_msg.mtype < 5) {
        long_msg.mtype = 100;
        switch (short_msg.mtype) {
            case 2:
                // list all users
                list_active_users(long_msg.longMsg);
                break;
                case 3:
                    // list all groups
                    list_groups(long_msg.longMsg);
                    break;
                case 4:
                    // list users by group X
                    err = list_group_users(long_msg.longMsg, short_msg.name);
                    break;
                default:
                    return -1;
        }
        if (err == -1) long_msg.code = -1;
        else long_msg.code = 1;
        msgsnd(my_key, &long_msg, MSG_SIZE, IPC_NOWAIT);
        return 0;
    }
    err = 0;
    switch (short_msg.mtype) {
    case 5:
        // join group X
        err = atJoin_group(1, my_id, short_msg.name);
        break;
    case 6:
        // leave group X
        err = atJoin_group(0, my_id, short_msg.name);
        break;

    // mute NAME:
    // if msg.opt==2 then the user is muted (not default, default is 0 - not muted)
    case 7:
        // mute user X
        err = atMute_user(2, my_id, short_msg.name);
        break;
    case 8:
        // mute group X
        err = atMute_group(2, my_id, short_msg.name);
        break;
    case 9:
        // unmute user X
        err = atMute_user(0, my_id, short_msg.name);
        break;
    case 10:
        // unmute group X
        err = atMute_group(0, my_id, short_msg.name);
        break;
    case 11:
        // logout
        logout_user(my_id);
        return 0;
    default:
        return -1;
    }
    if (err == -1) short_msg.code = -1;
    else short_msg.code = 1;
    short_msg.mtype = 101;
    msgsnd(my_key, &short_msg, SMSG_SIZE, IPC_NOWAIT);
    return 0;
}

int log_user(int *last_bad_pid, int *user_id) {
    for (int i = 0; i < users_loaded; i++) {
        // check if nick is like user nick, if so then check the pswd
        if (strcmp(user_nicks[i], login_rcvd.nick) != 0) continue;     
        if (strcmp(user_pswds[i], login_rcvd.pswd) != 0) {
            // Wrong pswd
            // table for PIDs w/ bad pswd
            for (int j = 0; j < *last_bad_pid; j++) {
                if (bad_pids[j] == login_rcvd.pid) {
                    (bad_pid_strikes[j])++;
                    if (bad_pid_strikes[j] == CHANCES_TO_LOGIN-1) return -3; // user blocked
                    return -1; // bad pswd
                } 
                break;
            }

            bad_pids[(*last_bad_pid)++] = login_rcvd.pid;
            return -1; // bad pswd
        }

        if (user_pids[i] > 0) return -4; // user already logged in
        *user_id = i;
        return 1;  // Logged in
    }
    return -2; // User not found
}

void load_users_and_groups() {
    int config_fd = open("config.txt", O_RDONLY);
    int iUser = 0;
    while (1) {
        char buf;
        char nick_pswd_str[MSG_SIZE] = "";
        int n;
        while((n = read(config_fd, &buf, 1)) > 0) {
            if (buf == ';' || buf == '#') {
                lseek(config_fd, 1, SEEK_CUR);
                break;
            }
            strncat(nick_pswd_str, &buf, 1);
        } 

        if (buf == '#') {
            printf("here \n");
            users_loaded = iUser;
            break;
            // End of user config file
        } 

        char *token = strtok(nick_pswd_str, "-");
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

    // load groups
    int iGroup = 0;
    while (1) {
        char buf;
        char name[MSG_SIZE] = "";
        int n;
        while((n = read(config_fd, &buf, 1)) > 0) {
            if (buf == ';') {
                lseek(config_fd, 1, SEEK_CUR);
                break;
            }
            strncat(name, &buf, 1);
        }
        if (n <= 0) {
            groups_loaded = iGroup;
            break;
            // End of config file
        }

        group_names[iGroup] = malloc(strlen(name)+1);
        strcpy(group_names[iGroup], name);
        iGroup += 1;
    }    
    close(config_fd);
    return;
}

// int load_groups() {
//     int config_fd = open("group_config.txt", O_RDONLY);
//     close(config_fd);
// }

int main(int argc, char const *argv[]) {
    signal(SIGINT, signal_handler);
    printf("server running...\n");

    load_users_and_groups();
    // users_loaded = load_users(); 
    printf("Users loaded (%d):\n", users_loaded);
    // groups_loaded = load_groups();
    printf("Groups loaded (%d):\n", groups_loaded);
    int last_bad_pid = 0;   

    // for(int i = 0; i < users_loaded; i++) {
    //     printf("Login: %s\tPswd: %s\n", user_nicks[i], user_pswds[i]);
    // }

    // create ipc fifo for users to log into (connect to server)
    server_id = msgget(PROGRAM_KEY, 0666 | IPC_CREAT);
    printf("Setup succesful... server ID:%d\n", server_id);

    while (1) {
        printf("...\n");
        // make all bits in structs 0 (struct clean) - OK in C99 standard
        login_rcvd = (LBUF) {0};
        login_to_send = (LCBUF) {0};
        int test = msgrcv(server_id, &login_rcvd, LMSG_SIZE, 1, IPC_NOWAIT); // 1 - only reads login msgs
        sleep(1);

        if (test > 0) {
            printf("someone's logging in\n");
            int user_id = 0;
            int feedback = log_user(&last_bad_pid, &user_id);
            login_to_send.mtype = login_rcvd.pid;
            login_to_send.msgCode = feedback; // 1 - OK, -1 - BAD PSWD, -2 - BAD USER -3 - BLOCKED USER
            printf("feedback = %d\n", feedback);
            if (feedback == 1) {
                int client_ipc_id = msgget(2000+login_rcvd.pid, 0666 | IPC_CREAT);
                user_ipcs[user_id] = client_ipc_id;
                user_pids[user_id] = login_rcvd.pid;
                login_to_send.ipcID = client_ipc_id;
            
            }
            msgsnd(server_id, &login_to_send, LCMSG_SIZE, IPC_NOWAIT);
        }

        for (int iUsers = 0; iUsers < users_loaded; iUsers++) {
            long_msg = (MBUF) {0};
            short_msg = (SMBUF) {0};
            if (user_pids[iUsers] == 0) continue;
            printf("now serving %d\n", user_pids[iUsers]);
            // serving all user IPC FIFOs
            int ibSMSG = msgrcv(user_ipcs[iUsers], &short_msg, SMSG_SIZE, -11, IPC_NOWAIT);
            // -11 is 1 to 11
            if (ibSMSG > 0) options_switch(user_ipcs[iUsers], iUsers);

            // code 12 (send msg to user/group) gets its own options bc of its size
            int ibMSG = msgrcv(user_ipcs[iUsers], &long_msg, MSG_SIZE, 12, IPC_NOWAIT);
            if (ibMSG > 0) long_msg_sender(user_ipcs[iUsers], iUsers);
        }
    }   
        // read msgs from main ipc fifo - log users in
        // write to main
        // check all the users ipc fifos (act on all requests)

    return 0;
}
