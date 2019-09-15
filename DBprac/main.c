#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

FILE *TicketsFL;
FILE *UsersFL;
FILE *UsersIND;

struct Ticket {
    __int32 ticketID; //*
    __int32 price;
    __int32 sitNumber;
    __int32 nextTicketPosition;
    bool exists;
};

struct User {
    char email[256]; //*
    char phoneNumber[18];
    char registrationDate[11];
    __int64 firstOwnedTicketID;

};

struct UserIndex {
    char email[256];
    size_t index;
    bool exists;
};

void openFiles(char *mode) {
    TicketsFL = fopen("tickets.fl", mode);
    UsersFL = fopen("users.fl", mode);
    UsersIND = fopen("users.ind", mode);
}

void closeFiles() {
    fclose(TicketsFL);
    fclose(UsersFL);
    fclose(UsersIND);
}

int startDB() {
    openFiles("r+b");
    if (TicketsFL == NULL ||
        UsersFL == NULL ||
        UsersIND == NULL) {
        closeFiles();
        openFiles("w+b");
    }
    return 0;
}

void rewriteExistingUsers() {

}

int finishDB() {
    closeFiles();
    return 0;
}

void setUser(struct User *user, char email[], char number[], char date[]) {
    strcpy(user->email, email);
    strcpy(user->phoneNumber, number);
    strcpy(user->registrationDate, date);
    user->firstOwnedTicketID = -1;
}

void setTicket(struct Ticket *ticket, __int32 ticketID, __int32 price, __int32 sitNumber) {
    ticket->ticketID = ticketID;
    ticket->price = price;
    ticket->sitNumber = sitNumber;
    ticket->exists = true;
    ticket->nextTicketPosition = -1;
}


long get_userIndexPosition(char *email) {
    struct UserIndex userIndex;
    fseek(UsersIND, 0, SEEK_END);
    long userCount = ftell(UsersIND) / sizeof(struct UserIndex);
    rewind(UsersIND);

    fseek(UsersIND, 0, SEEK_SET);
    for (int i = 0; i < userCount; i++) {
        fread(&userIndex, 1, sizeof(struct UserIndex), UsersIND);
        if (strcmp(userIndex.email, email) == 0 && userIndex.exists) {
            return (ftell(UsersIND) - sizeof(struct UserIndex));
        }
    }
    return -1;
}

bool ticketExists(__int32 ticketID) {
    fseek(TicketsFL, 0, SEEK_END);
    long ticketsCount = ftell(TicketsFL) / sizeof(struct Ticket);
    rewind(TicketsFL);

    struct Ticket ticket;

    for (int i = 0; i < ticketsCount; i++) {
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
        if (ticket.ticketID == ticketID)
            return true;
    }
    return false;
}

bool userExists(char email[]) {
    fseek(UsersIND, 0, SEEK_END);
    long usersCount = ftell(UsersIND) / sizeof(struct UserIndex);
    rewind(UsersIND);

    struct UserIndex userIndex;

    for (int i = 0; i < usersCount; i++) {
        fread(&userIndex, sizeof(struct Ticket), 1, TicketsFL);
        if (strcmp(userIndex.email, email) == 0)
            return true;
    }
    return false;
}

void get_m(struct User *user, char email[]) {
    long userIndPos = get_userIndexPosition(email);
    if (userIndPos == -1) {
        setUser(user, "", "", "");
        return;
    }

    fseek(UsersIND, userIndPos, SEEK_SET);

    struct UserIndex userIndex;
    fread(&userIndex, sizeof(struct UserIndex), 1, UsersIND);
    fseek(UsersFL, userIndex.index, SEEK_SET);
    fread(user, 1, sizeof(struct User), UsersFL);
}

void insert_m(struct User *user) {
    struct User userChecker;
    get_m(&userChecker, user->email);
    if (strcmp(userChecker.email, user->email) == 0) {
        printf("User with %s already exists\n", user->email);

        return;
    }

    fseek(UsersFL, 0, SEEK_END);
    fwrite(user, sizeof(struct User), 1, UsersFL);

    struct UserIndex userIndex;
    strcpy(userIndex.email, user->email);
    fseek(UsersFL, -sizeof(struct User), SEEK_CUR);
    userIndex.index = ftell(UsersFL);
    userIndex.exists = true;

    fseek(UsersIND, 0, SEEK_END);
    fwrite(&userIndex, sizeof(struct UserIndex), 1, UsersIND);
}

void insert_s(char email[], struct Ticket *ticket) {
    if (ticketExists(ticket->ticketID)) {
        printf("Ticket with TicketID %i already exists\n", ticket->ticketID);
        return;
    }

    struct User user;
    get_m(&user, email);
    if (strcmp(user.email, email) != 0) {
        printf("User with email %s not found\n", email);
        return;
    }

    fseek(TicketsFL, 0, SEEK_END);
    fwrite(ticket, sizeof(struct Ticket), 1, TicketsFL);
    fseek(TicketsFL, -sizeof(struct Ticket), SEEK_CUR);
    long newTicketPos = ftell(TicketsFL);

    if (user.firstOwnedTicketID == -1) {
        user.firstOwnedTicketID = ticket->ticketID;

        long userInd = get_userIndexPosition(email);
        fseek(UsersIND, userInd, SEEK_SET);
        struct UserIndex userIndex;
        fread(&userIndex, sizeof(struct UserIndex), 1, UsersIND);

        fseek(UsersFL, userIndex.index, SEEK_SET);
        fwrite(&user, sizeof(struct User), 1, UsersFL);
    } else {
        fseek(TicketsFL, 0, SEEK_SET);
        struct Ticket tmpTicket;
        tmpTicket.ticketID = -1;
        while (tmpTicket.ticketID != user.firstOwnedTicketID) {
            fread(&tmpTicket, sizeof(struct Ticket), 1, TicketsFL);
        }

        while (tmpTicket.nextTicketPosition != -1) {
            fseek(TicketsFL, tmpTicket.nextTicketPosition, SEEK_SET);
            fread(&tmpTicket, sizeof(struct Ticket), 1, TicketsFL);
        }

        tmpTicket.nextTicketPosition = newTicketPos;
        fseek(TicketsFL, -sizeof(struct Ticket), SEEK_CUR);
        fwrite(&tmpTicket, sizeof(struct Ticket), 1, TicketsFL);
    }
}

void update_m(struct User *updatedUser) {
    long userIndPos = get_userIndexPosition(updatedUser->email);
    if (userIndPos == -1) {
        printf("User with email %s not found\n", updatedUser->email);
        return;
    }

    struct User user;
    fseek(UsersFL, userIndPos, SEEK_SET);
    fread(&user, sizeof(struct User), 1, UsersFL);
    updatedUser->firstOwnedTicketID = user.firstOwnedTicketID;
    fseek(UsersFL, userIndPos, SEEK_SET);
    fwrite(updatedUser, sizeof(struct User), 1, UsersFL);
}

void update_s(char email[], struct Ticket *updatedTicket) {
    long userIndPos = get_userIndexPosition(email);
    if (userIndPos == -1) {
        printf("User with email %s not found\n", email);
        return;
    }
    if (!ticketExists(updatedTicket->ticketID)) {
        printf("Ticket with id %i not found\n", updatedTicket->ticketID);
        return;
    }
    struct User user;
    fseek(UsersFL, userIndPos, SEEK_SET);
    fread(&user, sizeof(struct User), 1, UsersFL);

    struct Ticket ticket;
    ticket.ticketID = -1;
    fseek(TicketsFL, 0, SEEK_SET);
    while (ticket.ticketID != user.firstOwnedTicketID) {
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }

    while (ticket.ticketID != updatedTicket->ticketID || ticket.nextTicketPosition != -1) {
        fseek(TicketsFL, ticket.nextTicketPosition, SEEK_SET);
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }
    if (ticket.ticketID == updatedTicket->ticketID) {
        fseek(TicketsFL, -sizeof(struct Ticket), SEEK_CUR);
        updatedTicket->nextTicketPosition = ticket.nextTicketPosition;
        fwrite(updatedTicket, sizeof(struct Ticket), 1, TicketsFL);
    } else {
        printf("No such ticket %i found for %s", updatedTicket->ticketID, email);
    }
}

void show_sublist(__int32 firstOwnedTicketID) {
    if (firstOwnedTicketID < 0)
        return;
    struct Ticket ticket;
    ticket.ticketID = -1;
    fseek(TicketsFL, 0, SEEK_SET);
    while (ticket.ticketID != firstOwnedTicketID) {
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }
    if (ticket.exists)
        printf("   %i %i %i\n", ticket.ticketID, ticket.price, ticket.sitNumber);
    while (ticket.nextTicketPosition != -1) {
        fseek(TicketsFL, ticket.nextTicketPosition, SEEK_SET);
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
        if (ticket.exists)
            printf("   %i %i %i\n", ticket.ticketID, ticket.price, ticket.sitNumber);
    }

}

void show_all() {
    struct UserIndex userIndex;
    fseek(UsersIND, 0, SEEK_END);
    long userCount = ftell(UsersIND) / sizeof(struct UserIndex);
    rewind(UsersIND);

    struct User user;

    for (int i = 0; i < userCount; i++) {
        fread(&userIndex, 1, sizeof(struct UserIndex), UsersIND);
        if (userIndex.exists) {
            fseek(UsersFL, userIndex.index, SEEK_SET);
            fread(&user, 1, sizeof(struct User), UsersFL);

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
        if (strcmp(command, "get-m") == 0) {
            
        }
        scanf("%s", arg);
        callCommand(command, arg);
    }
}

int main() {
    startDB();
    struct User user;
    struct Ticket ticket;
    setUser(&user, "test@gmail.com", "+3803928322", "12/12/2019");
    setTicket(&ticket, 1113, 100, 1);
    insert_m(&user);
    insert_s("test@gmail.com", &ticket);
    setTicket(&ticket, 1113, 200, 1);
    update_s("test@gmail.com", &ticket);
    strcpy(user.phoneNumber, "+38011223344");
    update_m(&user);

    show_all();
    //workCycle();
}