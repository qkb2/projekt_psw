#include "inf151825_151823.h"

int main(int argc, char const *argv[])
{
    printf("client active...\n");

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
        MSGBUF loginMsg;
        loginMsg.mtype = 1;
        
        printf("Login: ");
        char myLogin[SHORT_MSG_LEN];
        scanf("%16s", myLogin);
        strcpy(loginMsg.shortMsg, myLogin);

        printf("Password: ");
        char myPassword[LONG_MSG_LEN];
        scanf("%16s", myPassword);
        strcpy(loginMsg.longMsg, myPassword);

        msgsnd(server_id, &loginMsg, MSG_SIZE, 0);
        printf("sent\n");

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
