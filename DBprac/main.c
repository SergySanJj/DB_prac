#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

FILE *TicketsFL;
FILE *TicketsIND;
FILE *UsersFL;
FILE *UsersIND;

struct Ticket {
    __int32 ticketID;
    __int32 price;
    __int32 sitNumber;
    __int32 nextTicketPosition;
    bool exists;
};

struct User {
    char email[256];
    char phoneNumber[18];
    char registrationDate[11];
    __int32 firstOwnedTicketID;
    bool exists;
};

struct UserIndex {
    char email[256];
    size_t index;
};

void openFiles(char *mode) {
    TicketsFL = fopen("tickets.fl", mode);
    TicketsIND = fopen("tickets.ind", mode);
    UsersFL = fopen("users.fl", mode);
    UsersIND = fopen("users.ind", mode);
}

void closeFiles() {
    fclose(TicketsFL);
    fclose(TicketsIND);
    fclose(UsersFL);
    fclose(UsersIND);
}

int startDB() {
    openFiles("r+b");
    if (TicketsFL == NULL ||
        TicketsIND == NULL ||
        UsersFL == NULL ||
        UsersIND == NULL) {
        closeFiles();
        openFiles("w+b");
    }
    return 0;
}

int finishDB() {
    closeFiles();
    return 0;
}

void setUser(struct User *user, char email[], char number[], char date[]) {
    strcpy(user->email, email);
    strcpy(user->phoneNumber, number);
    strcpy(user->registrationDate, date);
    user->exists = true;
    user->firstOwnedTicketID = -1;
}

void insert_m(struct User *user) {
    fseek(UsersFL, 0, SEEK_END);
    fwrite(user, sizeof(struct User), 1, UsersFL);

    struct UserIndex userIndex;
    strcpy(userIndex.email, user->email);
    userIndex.index = ftell(UsersFL);

    fseek(UsersIND, 0, SEEK_END);
    fwrite(&userIndex, sizeof(struct UserIndex), 1, UsersIND);
}

void get_m(struct User *user, char email[]) {
    struct UserIndex userIndex;
    fseek(UsersIND, 0, SEEK_END);
    long userCount = ftell(UsersIND) / sizeof(struct UserIndex);
    rewind(UsersIND);

    fseek(UsersIND, 0, SEEK_SET);
    for (int i = 0; i < userCount; i++) {
        fread(&userIndex, 1, sizeof(struct UserIndex), UsersIND);
        if (strcmp(userIndex.email, email) == 0) {

            fseek(UsersFL, userIndex.index, SEEK_SET);
            fread(user, 1, sizeof(struct User), UsersFL);
            if (user->exists) {
                return;
            }

        }
    }

    setUser(user, "", "", "");
    user->exists=false;
}

void show_sublist(__int32 firstOwnedTicketID) {
    printf("   \n");
}

void show_all() {
    struct User user;
    fseek(UsersFL, 0, SEEK_END);
    long userCount = ftell(UsersFL) / sizeof(struct User);
    rewind(UsersFL);

    for (int i = 0; i < userCount; i++) {
        fread(&user, 1, sizeof(struct User), UsersFL);
        if (user.exists) {
            printf("%s %s %s\n", user.email, user.phoneNumber, user.registrationDate);
            if (user.firstOwnedTicketID != -1)
                show_sublist(user.firstOwnedTicketID);
        }
    }

}

void callCommand(char *command, char *arg) {
    printf("Calling %s with %s\n", command, arg);

}


int workCycle() {
    char command[10] = "";
    char arg[32];
    printf("To end program use [end] command\n");

    while (true) {
        printf("Enter command "
               "[get-m, get-s, del-m, del-s, update-m, update-s, insert-m, insert-s, count-m, count-s, show-all]\n");
        scanf("%s", command);
        if (strcmp(command, "end") == 0) {
            printf("Ending work with DB..\n");
            finishDB();
            printf("Finished!");
            return 0;
        }
        printf("Enter argument for [%s]\n", command);
        scanf("%s", arg);
        callCommand(command, arg);
    }
}


int main() {
    startDB();
    struct User user;
    setUser(&user, "test@gmail.com", "+3803928321", "12/12/2019");

    insert_m(&user);
    get_m(&user,"test@yaddex.com");
    printf("%i\n",user.exists);
    //show_all();
    //workCycle();
}