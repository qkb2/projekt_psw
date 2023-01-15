#include "inf151825_151823.h"

int main(int argc, char const *argv[])
{
    printf("client active...\n");
    int my_pid = getpid();

    // read server id from shared.txt
    int file_id;
    while (1) {
        file_id = open("shared.txt", O_RDONLY);
        if (file_id > -1) break;
    }
    int server_id;
    read(file_id, &server_id, sizeof(server_id));
    // printf("server id: %d\n", server_id);
    close(file_id);
    // MSGBUF server_msg;

    // login loop
    while (1) {
        LBUF login_msg;
        login_msg.mtype = 1;
        login_msg.pid = getpid();
        
        printf("Login: ");
        char my_login[SHORT_MSG_LEN];
        scanf("%16s", my_login);
        strcpy(login_msg.nick, my_login);

        printf("Password: ");
        char my_password[LONG_MSG_LEN];
        scanf("%16s", my_password);
        strcpy(login_msg.pswd, my_password);

        msgsnd(server_id, &login_msg, LMSG_SIZE, 0);
        printf("sent\n");

        msgrcv(server_id, &login_msg, LMSG_SIZE, my_pid, 0);
        
        // try to connect to server
        // send login msg to server w/ pswd
        // if credentials are correct, exit loop w/ correct ipc adr
    }
    
    // comms loop
    while (1) {
        // get up to #MAX_READ_MSGS of msgs read
        // press enter to skip writing phase, enter the correct phrase to send msgs
        // write msg to server
    }

    return 0;
}
