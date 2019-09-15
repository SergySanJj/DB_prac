#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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


int userCount() {
    fseek(UsersIND, 0, SEEK_END);
    long usersCount = ftell(UsersIND) / sizeof(struct UserIndex);
    rewind(UsersIND);
    return usersCount;
}

int ticketCount() {
    fseek(TicketsFL, 0, SEEK_END);
    long ticketCount = ftell(TicketsFL) / sizeof(struct Ticket);
    rewind(TicketsFL);
    return ticketCount;
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

void get_s(struct Ticket *ticket, char email[], __int32 ticketID) {
    struct User user;
    get_m(&user, email);
    if (strcmp(user.email, email) != 0) {
        printf("User with %s doesn't exist\n", email);
        return;
    }
    if (!ticketExists(ticketID)) {
        printf("Ticket with %i doesn't exist\n", ticketID);
        return;
    }

    ticket->ticketID = -1;
    fseek(TicketsFL, 0, SEEK_SET);
    while (ticket->ticketID != user.firstOwnedTicketID) {
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }

    while (ticket->ticketID != ticketID || ticket->nextTicketPosition != -1) {
        fseek(TicketsFL, ticket->nextTicketPosition, SEEK_SET);
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }
    if (ticket->ticketID == ticketID) {
        return;
    } else {
        printf("No such ticket %i found for %s", ticketID, email);
        setTicket(ticket, -1, -1, -1);
    }
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
    struct UserIndex userIndex;
    fseek(UsersIND, userIndPos, SEEK_SET);
    fread(&userIndex, sizeof(struct UserIndex), 1, UsersIND);
    struct User user;
    fseek(UsersFL, userIndex.index, SEEK_SET);
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


int workCycle() {
    char command[10] = "";

    char email[256];
    char phoneNumber[18];
    char registrationDate[11];

    __int32 ticketID;
    __int32 price;
    __int32 sitNumber;

    char buff[8][64];


    printf("To end program use [end] command\n");

    while (true) {
        printf("Enter command "
               "[get-m, get-s, del-m, del-s, update-m, update-s, insert-m, insert-s, count, show-all]\n");
        scanf("%s", command);
        if (strcmp(command, "end") == 0) {
            printf("Ending work with DB..\n");
            finishDB();
            printf("Finished!");
            return 0;
        }
        printf("Enter argument for [%s]\n", command);
        if (strcmp(command, "get-m") == 0) {
            printf("email\n");
            scanf("%s", email);
            struct User user;
            get_m(&user, email);

            printf("%s %s %s\n", user.email, user.phoneNumber, user.registrationDate);
            if (user.firstOwnedTicketID != -1)
                show_sublist(user.firstOwnedTicketID);
        }

        if (strcmp(command, "get-s") == 0) {
            printf("email ticketID\n");
            scanf("%s %s", email, buff[0]);
            ticketID = strtol(buff[0], NULL, 10);

            struct Ticket ticket;
            get_s(&ticket, email, ticketID);
            if (ticket.ticketID != -1) {
                printf("%i %i %i\n", ticket.ticketID, ticket.price, ticket.sitNumber);
            }
        }

        if (strcmp(command, "update-m") == 0) {
            printf("email new: phoneNumber registrationDate\n");
            scanf("%s %s %s", email, phoneNumber, registrationDate);
            struct User user;
            setUser(&user, email, phoneNumber, registrationDate);

            update_m(&user);
        }

        if (strcmp(command, "update-s") == 0) {
            printf("email ticketID new: price sitNumber\n");
            scanf("%s %s %s %s", email, buff[0], buff[1], buff[2]);
            struct Ticket ticket;
            ticketID = strtol(buff[0], NULL, 10);
            price = strtol(buff[1], NULL, 10);
            sitNumber = strtol(buff[2], NULL, 10);
            setTicket(&ticket, ticketID, price, sitNumber);

            update_s(email, &ticket);
        }

        if (strcmp(command, "insert-m") == 0) {
            printf("email phoneNumber registrationDate\n");
            scanf("%s %s %s", email, phoneNumber, registrationDate);
            struct User user;
            setUser(&user, email, phoneNumber, registrationDate);

            insert_m(&user);
        }

        if (strcmp(command, "insert-s") == 0) {
            printf("email ticketID price sitNumber\n");
            scanf("%s %s %s %s", email, buff[0], buff[1], buff[2]);
            struct Ticket ticket;
            ticketID = strtol(buff[0], NULL, 10);
            price = strtol(buff[1], NULL, 10);
            sitNumber = strtol(buff[2], NULL, 10);
            setTicket(&ticket, ticketID, price, sitNumber);

            insert_s(email, &ticket);
        }

        if (strcmp(command, "count") == 0) {
            printf("User count: %i\nTickets count %i\n", userCount(), ticketCount());
        }

        if (strcmp(command, "show-all") == 0) {
            show_all();
        }
    }
}

int main() {
    startDB();
    workCycle();
}