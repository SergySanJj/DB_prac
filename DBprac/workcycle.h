//
// Created by isara on 16-Sep-19.
//

#ifndef DBPRAC_WORKCYCLE_H
#define DBPRAC_WORKCYCLE_H

#include "globals.h"
#include "miscfunctions.h"
#include "dbcommands.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

        if (strcmp(command, "del-m") == 0) {
            printf("Email\n");
            scanf("%s", email);
            del_m(email);
        }

        if (strcmp(command, "del-s") == 0) {
            printf("Email ticketID\n");
            scanf("%s %s", email, buff[0]);
            ticketID = strtol(buff[0], NULL, 10);
            del_s(email, ticketID);
        }

        if (strcmp(command, "count") == 0) {
            printf("User count: %i\nTickets count %i\n", userCount(), ticketCount());
        }

        if (strcmp(command, "show-all") == 0) {
            show_all();
        }
    }
}

#endif //DBPRAC_WORKCYCLE_H
