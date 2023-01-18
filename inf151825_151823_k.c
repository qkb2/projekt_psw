#include "inf151825_151823.h"

int logout(int my_key) {
    SMBUF smsg;
    smsg.mtype = 11;
    msgsnd(my_key, &smsg, SMSG_SIZE, 0);
    return 0;
}


// Function after successful logging
int is_logged(int my_key) {   
    MBUF msg;
    SMBUF smsg;

    // comms loop
    // char delim[] = " ";
    while (1) {
        // receive msgs
        int iMsgs = 0;
        while (iMsgs < MAX_READ_MSGS) {
            int sm = msgrcv(my_key, &msg, MSG_SIZE, 99, IPC_NOWAIT);
            iMsgs++;
            if (sm < 0) continue;

            if (msg.code == 2) {
                printf("User %s wrote to you (pv): \n", msg.shortMsg);
            } else {
                printf("User %s wrote to group %s: \n", msg.shortMsg, msg.shortMsgB);
            }
            printf("%s\n", msg.longMsg);
        }


        printf(
            "Enter what you want to do? [list, join, leave, mute, send, logout]\n");
        char request[SHORT_MSG_LEN];
        fflush(stdin);


        scanf("%s", request);
        if (strcmp(request, "list") == 0) {
            printf("Do you want to list all [u], [g] or users by group [b]?\n");
            char opt = 0;
            scanf(" %c", &opt);
            if (opt == 'u') smsg.mtype = 2;
            else if (opt == 'g') smsg.mtype = 3;
            else if (opt == 'b') {
                printf("Which group: \n");
                scanf("%16s", smsg.name);
                smsg.mtype = 4;
            } else {
                printf("Command not found.\n");
                continue;
            }
            msgsnd(my_key, &smsg, SMSG_SIZE, 0);
            printf("waiting for msg...\n");
            msgrcv(my_key, &msg, MSG_SIZE, 100, 0);
            printf("msg received...\n");
            if (msg.code == -1) {
                printf("No such name exists.\n");
                continue;
            }
            if (msg.code == 1) {
                printf("List from the server: \n%s\n", msg.longMsg);
                continue;
            }
        }
        else if (strcmp(request, "join") == 0) {
            printf("Enter the name of the group you want to join: \n");
            scanf("%16s", smsg.name);
            smsg.mtype = 5;
            msgsnd(my_key, &smsg, SMSG_SIZE, 0);
            msgrcv(my_key, &smsg, SMSG_SIZE, 101, 0);
            if (smsg.code == -1) {
                printf("No such group exists.\n");
            }
            if (smsg.code == 1) {
                printf("Operation successful.\n");
            }
        }
        else if (strcmp(request, "leave") == 0) {
            printf("Enter the name of the group you want to leave: \n");
            scanf("%16s", smsg.name);
            smsg.mtype = 6;
            msgsnd(my_key, &smsg, SMSG_SIZE, 0);
            msgrcv(my_key, &smsg, SMSG_SIZE, 101, 0);
            if (smsg.code == -1) {
                printf("No such group exists.\n");
            }
            if (smsg.code == 1) {
                printf("Operation successful.\n");
            }
        }

        else if (strcmp(request, "mute") == 0) {
            printf(
                "Do you want to mute an user [u] or a group [g]? Example: [u username]\n");
            char opt = 0;
            scanf(" %c %s", &opt, smsg.name);
            if (opt == 'u') smsg.mtype = 7;
            else if (opt == 'g') smsg.mtype = 8;
            else {
                printf("Command not found\n");
                continue;
            }

            msgsnd(my_key, &smsg, SMSG_SIZE, 0);
            msgrcv(my_key, &smsg, MSG_SIZE, 101, 0);
            if (smsg.code == -1) {
                printf("No such name exists.\n");
                continue;
            }
            if (smsg.code == 1) {
                printf("Operation successful\n");
                continue;
            }
        }

        else if (strcmp(request, "unmute") == 0) {
            printf(
                "Do you want to unmute an user [u] or a group [g]? Example: [u username]\n");
            char opt = 0;
            scanf(" %c %s", &opt, smsg.name);
            if (opt == 'u') smsg.mtype = 9;
            else if (opt == 'g') smsg.mtype = 10;
            else {
                printf("Command not found\n");
                continue;
            }

            msgsnd(my_key, &smsg, SMSG_SIZE, 0);
            msgrcv(my_key, &smsg, MSG_SIZE, 101, 0);
            if (smsg.code == -1) {
                printf("No such name exists.\n");
                continue;
            }
            if (smsg.code == 1) {
                printf("Operation successful\n");
                continue;
            }
        }
        else if (strcmp(request, "send") == 0) {
            printf(
                "Do you want to send a msg to an user [u] or a group [g]? Example: [u username]\n");
            char opt = 0;
            msg.mtype = 12;
            scanf(" %c %s", &opt, msg.shortMsg);
            if (opt == 'u') msg.code = 2;
            else if (opt == 'g') msg.code = 3;
            else {
                printf("Command not found\n");
                continue;
            }

            msgsnd(my_key, &msg, MSG_SIZE, 0);
            msgrcv(my_key, &smsg, SMSG_SIZE, 101, 0);
            if (smsg.code == -1) {
                printf("No such name exists.\n");
                continue;
            }
            if (smsg.code == 1) {
                printf("Operation successful\n");
                continue;
            }
        }
        else if (strcmp(request, "logout") == 0) {
            printf("Logging out...\n");
            logout(my_key);
            return 0;
        }
        else if (strcmp(request, "") == 0) continue;
        else printf("Command not found");
    }
    return 0;
}


int main(int argc, char const *argv[])
{
    printf("client active...\n");
    int my_pid = getpid();

    // read server id from shared.txt
    // int file_id;
    // while (1) {
    //     file_id = open("shared.txt", O_RDONLY);
    //     if (file_id > -1) break;
    // }
    int server_id = msgget(PROGRAM_KEY, 0666 | IPC_CREAT);
    // read(file_id, &server_id, sizeof(server_id));
    printf("server id: %d\n", server_id);
    // close(file_id);
    // MSGBUF server_msg;

    // login loop
    LBUF login_msg;
    LCBUF login_rcvd;
    MBUF msg_to_server;
    MBUF msg_from_server;

    while (1) {
        login_msg.mtype = 1;
        login_msg.pid = getpid();
        
        printf("Login: ");
        char my_login[SHORT_MSG_LEN];
        scanf("%16s", my_login);
        strcpy(login_msg.nick, my_login);

        printf("Password: ");
        char my_password[SHORT_MSG_LEN];
        scanf("%16s", my_password);
        strcpy(login_msg.pswd, my_password);

        printf("sending\n");
        msgsnd(server_id, &login_msg, LMSG_SIZE, 0);
        printf("sent\n");

        printf("receiving\n");
        msgrcv(server_id, &login_rcvd, LCMSG_SIZE, my_pid, 0);
        printf("received\n");

        
        switch (login_rcvd.msgCode)
        {
        case 1:     // Logged in
            printf("You logged in correctly.\n");
            is_logged(login_rcvd.ipcID);
            break;
        case -1:    // Wrong pswd
            printf("Your password is wrong. Try again.\n");
            break;
        case -2:    // User not found
            printf("User not found.\n");
            break;
        case -3:    // Block pid
            printf("You've been blocked, sir. Stuck in a loop...\n");
            while(1);    
        default:    // idk
            break;
        }
        
        // try to connect to server
        // send login msg to server w/ pswd
        // if credentials are correct, exit loop w/ correct ipc adr
    }

    return 0;
}


        // char request[] = "list -u asf fas fa sf";
        
        // fgets(request1, 32, stdin);
        // int cmdn = fscanf(stdin, "%s", request1);
        // char *ptr = strtok(request1, delim);     // ptr is first word
        // if (ptr != NULL) {
        //     if (strcmp(ptr, "list") == 0) {   
        //         printf("jestem w list\n");
        //         ptr = strtok(NULL, delim);      // ptr is second word
        //         printf("Słowo 2: %s", ptr);
        //         if (ptr != NULL) {
        //             printf("jestem w drugim słowie nie NULL\n");
        //             if (strcmp(ptr, "-u\n") == 0) {
        //                 printf("cmd is list -u\n");
        //                 continue;
        //             }
        //             else if (strcmp(ptr, "-g\n") == 0) {
        //                 printf("cmd is list -g\n");
        //                 continue;
        //             }
        //             else if (strcmp(ptr, "-b") == 0) {
        //                 printf("cmd is list -b\n");
        //                 ptr = strtok(NULL, delim);
        //                 continue;
        //             }
        //             else {
        //                 printf("Command not found.\n");
        //             }
        //         }
        //         else {
        //             printf("Command not found.\n");
        //         }
        //     }         
        //     else if (strcmp(ptr, "join") == 0) {  
        //         printf("cmd is join");
        //         ptr = strtok(NULL, delim);  
        //         continue;   // Do join on ptr
        //     }        
        //     else if (strcmp(ptr, "leave") == 0) {  
        //         printf("cmd is leave");
        //         ptr = strtok(NULL, delim);  
        //         continue;   // Do join on ptr
        //     }
        //     else if (strcmp(ptr, "mute") == 0) {   
        //         ptr = strtok(NULL, delim);      // ptr is second word
        //         if (ptr != NULL) {
        //             if (strcmp(ptr, "-u") == 0) {
        //                 continue;
        //             }

        //             else if (strcmp(ptr, "-g") == 0) {
        //                 continue;
        //             }

        //             else {
        //                 printf("Command not found.\n");
        //             }
        //         }
        //         else {
        //             printf("Command not found.\n");
        //         }
        //     }
        //     else if (strcmp(ptr, "send") == 0) {   
        //         ptr = strtok(NULL, delim);      // ptr is second word
        //         if (ptr != NULL) {
        //             if (strcmp(ptr, "-u") == 0) {
        //                 continue;
        //             }
        //             if (strcmp(ptr, "-g") == 0) {
        //                 continue;
        //             }
        //             else {
        //                 printf("Command not found.\n");
        //             }
        //         }
        //         else {
        //             printf("Command not found.\n");
        //         }
        //     }
        //     else if (strcmp(ptr, "logout") == 0) {   
        //         // send info to server that im logging out
        //         return 0;
        //     }
        //     else printf("Command not found.\n");
        // }