//
// Created by isara on 16-Sep-19.
//
#ifndef DBPRAC_MISCFUNCTIONS_H
#define DBPRAC_MISCFUNCTIONS_H

#include "globals.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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

    int result = 0;
    struct UserIndex userIndex;
    for (int i = 0; i < usersCount; i++) {
        fread(&userIndex, sizeof(struct UserIndex), 1, UsersIND);
        if (userIndex.exists)
            result++;
    }

    return result;
}

int ticketCount() {
    fseek(TicketsFL, 0, SEEK_END);
    long ticketCount = ftell(TicketsFL) / sizeof(struct Ticket);
    rewind(TicketsFL);

    int result = 0;
    struct Ticket ticket;
    for (int i = 0; i < ticketCount; i++) {
        fread(&ticket, sizeof(struct Ticket), 1, TicketsFL);
        if (ticket.exists)
            result++;
    }

    return result;
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
        if (ticket.ticketID == ticketID && ticket.exists)
            return true;
    }
    return false;
}

bool userExists(char email[]) {
    long userIndPos = get_userIndexPosition(email);
    if (userIndPos == -1) {
        return false;
    }
    return true;
}

#endif //DBPRAC_MISCFUNCTIONS_H
