//
// Created by isara on 16-Sep-19.
//
#ifndef DBPRAC_GLOBALS_H
#define DBPRAC_GLOBALS_H

#include <stdio.h>
#include <stdbool.h>

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

#endif //DBPRAC_GLOBALS_H
