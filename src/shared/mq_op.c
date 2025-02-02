/* Copyright (C) 2015-2020, Wazuh Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#include "shared.h"
#include "config/config.h"
#include "os_net/os_net.h"

static log_builder_t * mq_log_builder;
int sock_fail_time;

#ifndef WIN32

/* Start the Message Queue. type: WRITE||READ */
int StartMQ(const char *path, short int type, short int n_attempts)
{
    if (type == READ) {
        return (OS_BindUnixDomain(path, SOCK_DGRAM, OS_MAXSTR + 512));
    }

    /* We give up to 21 seconds for the other end to start */
    else {
        int rc = 0, sleep_time = 5;
        short int attempt = 0;

        // If n_attempts is 0, trying to reconnect infinitely
        while ((rc = OS_ConnectUnixDomain(path, SOCK_DGRAM, OS_MAXSTR + 256)), rc < 0){
            attempt++;
            mdebug1("Can't connect to '%s': %s (%d). Attempt: %d", path, strerror(errno), errno, attempt);
            if (n_attempts != INFINITE_OPENQ_ATTEMPTS && attempt == n_attempts) {
                break;
            }
            sleep(sleep_time += 5);
        }

        if (rc < 0) {
            return OS_INVALID;
        }

        mdebug1("Connected succesfully to '%s' after %d attempts", path, attempt);
        mdebug1(MSG_SOCKET_SIZE, OS_getsocketsize(rc));
        return (rc);
    }
}

/* Send a message to the queue */
int SendMSG(int queue, const char *message, const char *locmsg, char loc)
{
    int __mq_rcode;
    char tmpstr[OS_MAXSTR + 1];
    static int reported = 0;

    tmpstr[OS_MAXSTR] = '\0';

    /* Check for global locks */
    os_wait();

    if (loc == SECURE_MQ) {
        loc = message[0];
        message++;

        if (message[0] != ':') {
            merror(FORMAT_ERROR);
            return (0);
        }
        message++; /* Pointing now to the location */

        if (strncmp(message, "keepalive", 9) == 0) {
            return (0);
        }

        snprintf(tmpstr, OS_MAXSTR, "%c:%s->%s", loc, locmsg, message);
    } else {
        snprintf(tmpstr, OS_MAXSTR, "%c:%s:%s", loc, locmsg, message);
    }

    /* Queue not available */
    if (queue < 0) {
        return (-1);
    }

    if ((__mq_rcode = OS_SendUnix(queue, tmpstr, 0)) < 0) {
        /* Error on the socket */
        if (__mq_rcode == OS_SOCKTERR) {
            merror("socketerr (not available).");
            close(queue);
            return (-1);
        }

        /* Unable to send. Socket busy */
        mdebug2("Socket busy, discarding message.");

        if (!reported) {
            reported = 1;
            mwarn("Socket busy, discarding message.");
        }
    }

    return (0);
}

/* Send a message to socket */
int SendMSGtoSCK(int queue, const char *message, const char *locmsg, __attribute__((unused)) char loc, logtarget * target)
{
    int __mq_rcode;
    char tmpstr[OS_MAXSTR + 1];
    time_t mtime;
    char * _message = NULL;
    int retval = 0;

    _message = log_builder_build(mq_log_builder, target->format, message, locmsg);

    tmpstr[OS_MAXSTR] = '\0';

    if (strcmp(target->log_socket->name, "agent") == 0) {
        if(SendMSG(queue, _message, locmsg, loc) != 0) {
            free(_message);
            return -1;
        }
    }else{
        int sock_type;
        const char * strmode;

        switch (target->log_socket->mode) {
        case IPPROTO_UDP:
            sock_type = SOCK_DGRAM;
            strmode = "udp";
            break;
        case IPPROTO_TCP:
            sock_type = SOCK_STREAM;
            strmode = "tcp";
            break;
        default:
            merror("At %s(): undefined protocol. This shouldn't happen.", __FUNCTION__);
            free(_message);
            return -1;
        }

        // create message and add prefix
        if (target->log_socket->prefix && *target->log_socket->prefix) {
            snprintf(tmpstr, OS_MAXSTR, "%s%s", target->log_socket->prefix, _message);
        } else {
            snprintf(tmpstr, OS_MAXSTR, "%s", _message);
        }

        // Connect to socket if disconnected
        if (target->log_socket->socket < 0) {
            if (mtime = time(NULL), mtime > target->log_socket->last_attempt + sock_fail_time) {
                if (target->log_socket->socket = OS_ConnectUnixDomain(target->log_socket->location, sock_type, OS_MAXSTR + 256), target->log_socket->socket < 0) {
                    target->log_socket->last_attempt = mtime;
                    merror("Unable to connect to socket '%s': %s (%s)", target->log_socket->name, target->log_socket->location, strmode);
                    free(_message);
                    return -1;
                }

                mdebug1("Connected to socket '%s' (%s)", target->log_socket->name, target->log_socket->location);
            } else {
                mdebug2("Discarding event from '%s' due to connection issue with '%s'", locmsg, target->log_socket->name);
                free(_message);
                return 1;
            }
        }

        // Send msg to socket
        if (__mq_rcode = OS_SendUnix(target->log_socket->socket, tmpstr, strlen(tmpstr)), __mq_rcode < 0) {
            if (__mq_rcode == OS_SOCKTERR) {
                if (mtime = time(NULL), mtime > target->log_socket->last_attempt + sock_fail_time) {
                    close(target->log_socket->socket);

                    if (target->log_socket->socket = OS_ConnectUnixDomain(target->log_socket->location, sock_type, OS_MAXSTR + 256), target->log_socket->socket < 0) {
                        merror("Unable to connect to socket '%s': %s (%s)", target->log_socket->name, target->log_socket->location, strmode);
                        target->log_socket->last_attempt = mtime;
                    } else {
                        mdebug1("Connected to socket '%s' (%s)", target->log_socket->name, target->log_socket->location);

                        if (OS_SendUnix(target->log_socket->socket, tmpstr, strlen(tmpstr)), __mq_rcode < 0) {
                            merror("Cannot send message to socket '%s'. (Retry)", target->log_socket->name);
                            SendMSG(queue, "Cannot send message to socket.", "logcollector", LOCALFILE_MQ);
                            target->log_socket->last_attempt = mtime;
                        }
                    }
                } else {
                    mdebug2("Discarding event from '%s' due to connection issue with '%s'", locmsg, target->log_socket->name);
                }
            } else {
                merror("Cannot send message to socket '%s'. (Retry)", target->log_socket->name);
                SendMSG(queue, "Cannot send message to socket.", "logcollector", LOCALFILE_MQ);
            }
            retval = 1;
        }
    }
    free(_message);
    return (retval);
}

#else

int SendMSGtoSCK(int queue, const char *message, const char *locmsg, char loc, logtarget * targets) {
    char * _message;
    int retval;

    if (!targets[0].log_socket) {
        merror("No targets defined for a localfile.");
        return -1;
    }

    _message = log_builder_build(mq_log_builder, targets[0].format, message, locmsg);
    retval = SendMSG(queue, _message, locmsg, loc);
    free(_message);
    return retval;
}

#endif /* !WIN32 */

void mq_log_builder_init() {
    assert(mq_log_builder == NULL);
    mq_log_builder = log_builder_init(true);
}

int mq_log_builder_update() {
    assert(mq_log_builder != NULL);
    return log_builder_update(mq_log_builder);
}
