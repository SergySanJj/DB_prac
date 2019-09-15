//
// Created by isara on 16-Sep-19.
//

#ifndef DBPRAC_DBCOMMANDS_H
#define DBPRAC_DBCOMMANDS_H

#include "globals.h"
#include "miscfunctions.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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

void del_m(char email[]) {
    if (!userExists(email)) {
        printf("User with email %s not found\n", email);
        return;
    }
    struct UserIndex userIndex;
    struct User user;
    long userIndPos = get_userIndexPosition(email);
    fseek(UsersIND, userIndPos, SEEK_SET);
    fread(&userIndex, sizeof(struct UserIndex), 1, UsersIND);
    fseek(UsersIND, -sizeof(struct UserIndex), SEEK_CUR);
    userIndex.exists = false;
    fwrite(&userIndex, sizeof(struct UserIndex), 1, UsersIND);

    fseek(UsersFL, userIndex.index, SEEK_SET);
    fread(&user, sizeof(struct User), 1, UsersFL);

    if (user.firstOwnedTicketID == -1)
        return;

    struct Ticket ticket;
    ticket.ticketID = -1;
    fseek(TicketsFL, 0, SEEK_SET);
    while (ticket.ticketID != user.firstOwnedTicketID) {
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }

    ticket.exists = false;
    fseek(TicketsFL, -sizeof(struct Ticket), SEEK_CUR);
    fwrite(&ticket, sizeof(struct Ticket), 1, TicketsFL);

    while (ticket.nextTicketPosition != -1) {
        fseek(TicketsFL, ticket.nextTicketPosition, SEEK_SET);
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
        ticket.exists = false;
        fseek(TicketsFL, -sizeof(struct Ticket), SEEK_CUR);
        fwrite(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }
}

void del_s(char email[], __int32 ticketID) {
    if (!userExists(email)) {
        printf("User with email %s not found\n", email);
        return;
    }
    struct UserIndex userIndex;
    struct User user;
    long userIndPos = get_userIndexPosition(email);
    fseek(UsersIND, userIndPos, SEEK_SET);
    fread(&userIndex, sizeof(struct UserIndex), 1, UsersIND);

    fseek(UsersFL, userIndex.index, SEEK_SET);
    fread(&user, sizeof(struct User), 1, UsersFL);

    if (user.firstOwnedTicketID == -1)
        return;

    struct Ticket ticket;
    ticket.ticketID = -1;
    fseek(TicketsFL, 0, SEEK_SET);
    while (ticket.ticketID != user.firstOwnedTicketID) {
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }


    while (ticket.ticketID != ticketID && ticket.nextTicketPosition != -1) {
        fseek(TicketsFL, ticket.nextTicketPosition, SEEK_SET);
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
    }
    if (ticket.ticketID == ticketID) {
        ticket.exists = false;
        fseek(TicketsFL, -sizeof(struct Ticket), SEEK_CUR);
        fwrite(&ticket, sizeof(struct Ticket), 1, TicketsFL);
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

#endif //DBPRAC_DBCOMMANDS_H
