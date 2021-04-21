/*
 * Copyright (C) 2015-2021, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>

#include "shared.h"
#include "list_op.h"
#include "os_regex/os_regex.h"
#include "os_net/os_net.h"
#include "wazuh_modules/wmodules.h"
#include "../external/cJSON/cJSON.h"
#include "os_execd/execd.h"

#include "../wrappers/common.h"
#include "../wrappers/libc/stdio_wrappers.h"
#include "../wrappers/posix/select_wrappers.h"
#include "../wrappers/wazuh/os_net/os_net_wrappers.h"
#include "../wrappers/wazuh/shared/debug_op_wrappers.h"
#include "../wrappers/wazuh/shared/exec_op_wrappers.h"
#include "../wrappers/wazuh/os_execd/exec_wrappers.h"

extern int test_mode;

int __wrap_CreateThreadJoinable(pthread_t *lthread, void * (*function_pointer)(void *), void *data) {
    return mock_type(int);
}

/* Setup/Teardown */

static int group_setup(void ** state) {
    test_mode = 1;
    is_disabled = 0;
    return 0;
}

static int group_teardown(void ** state) {
    test_mode = 0;
    return 0;
}

static int test_setup_file(void **state) {
    wfd_t* wfd = NULL;
    os_calloc(1, sizeof(wfd_t), wfd);
    *state = wfd;
    return 0;
}

static int test_teardown_file(void **state) {
    wfd_t* wfd = *state;
    os_free(wfd);
    return 0;
}

/* Tests */

static void test_execd_start_ok(void **state) {
    wfd_t * wfd = *state;
    int queue = 1;
    int now = 123456789;
    char *message = "{"
                        "\"version\":\"1\","
                        "\"origin\":{"
                            "\"name\":\"node01\","
                            "\"module\":\"wazuh-analysisd\""
                        "},"
                        "\"command\":\"restart-wazuh0\","
                        "\"parameters\":{"
                            "\"extra_args\":[],"
                            "\"alert\":{"
                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                "\"rule\":{"
                                    "\"level\":5,"
                                    "\"description\":\"File added to the system.\","
                                    "\"id\":\"554\""
                                "},"
                                "\"id\":\"1609860180.513333\","
                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                "\"syscheck\":{"
                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                    "\"mode\":\"realtime\","
                                    "\"event\":\"added\""
                                "},"
                                "\"location\":\"syscheck\""
                            "}"
                        "}"
                    "}";

    timeout_list = OSList_Create();

    will_return(__wrap_CreateThreadJoinable, 0);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    expect_string(__wrap__mtdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    expect_string(__wrap_fopen, path, DEFAULTAR);
    expect_string(__wrap_fopen, mode, "r");
    will_return(__wrap_fopen, (FILE *) 100);

    expect_string(__wrap__mtdebug1, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug1, formatted_msg, "Executing command 'active-response/bin/restart-wazuh.sh {"
                                                                                        "\"version\":\"1\","
                                                                                        "\"origin\":{"
                                                                                            "\"name\":\"node01\","
                                                                                            "\"module\":\"wazuh-modulesd\""
                                                                                        "},"
                                                                                        "\"command\":\"add\","
                                                                                        "\"parameters\":{"
                                                                                            "\"extra_args\":[],"
                                                                                            "\"alert\":{"
                                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                                "\"rule\":{"
                                                                                                    "\"level\":5,"
                                                                                                    "\"description\":\"File added to the system.\","
                                                                                                    "\"id\":\"554\""
                                                                                                "},"
                                                                                                "\"id\":\"1609860180.513333\","
                                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                                "\"syscheck\":{"
                                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                                    "\"mode\":\"realtime\","
                                                                                                    "\"event\":\"added\""
                                                                                                "},"
                                                                                                "\"location\":\"syscheck\""
                                                                                            "},"
                                                                                            "\"program\":\"active-response/bin/restart-wazuh.sh\""
                                                                                        "}"
                                                                                    "}'");

    expect_any(__wrap_fgets, __stream);
    will_return(__wrap_fgets, "restart-wazuh0 - restart-wazuh.sh - 0\n");

    expect_string(__wrap_fopen, path, "active-response/bin/restart-wazuh.sh");
    expect_string(__wrap_fopen, mode, "r");
    will_return(__wrap_fopen, (FILE *) 100);

    expect_value(__wrap_fclose, _File, (FILE *) 100);
    will_return(__wrap_fclose, 1);

    expect_any(__wrap_fgets, __stream);
    will_return(__wrap_fgets, NULL);

    expect_value(__wrap_fclose, _File, (FILE *) 100);
    will_return(__wrap_fclose, 1);

    will_return(__wrap_wpopenv, wfd);
    will_return(__wrap_fwrite, 0);
    will_return(__wrap_wpclose, 0);

    execd_start(queue);

    os_free(timeout_list);
}

static void test_execd_start_timeout(void **state) {
    wfd_t * wfd = *state;
    int queue = 1;
    int now = 123456789;
    char *message = "{"
                        "\"version\":\"1\","
                        "\"origin\":{"
                            "\"name\":\"node01\","
                            "\"module\":\"wazuh-analysisd\""
                        "},"
                        "\"command\":\"restart-wazuh0\","
                        "\"parameters\":{"
                            "\"extra_args\":[],"
                            "\"alert\":{"
                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                "\"rule\":{"
                                    "\"level\":5,"
                                    "\"description\":\"File added to the system.\","
                                    "\"id\":\"554\""
                                "},"
                                "\"id\":\"1609860180.513333\","
                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                "\"syscheck\":{"
                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                    "\"mode\":\"realtime\","
                                    "\"event\":\"added\""
                                "},"
                                "\"location\":\"syscheck\""
                            "}"
                        "}"
                    "}";

    timeout_list = OSList_Create();

    will_return(__wrap_CreateThreadJoinable, 0);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    expect_string(__wrap__mtdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    expect_string(__wrap__mtdebug1, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug1, formatted_msg, "Executing command 'active-response/bin/restart-wazuh.sh {"
                                                                                        "\"version\":\"1\","
                                                                                        "\"origin\":{"
                                                                                            "\"name\":\"node01\","
                                                                                            "\"module\":\"wazuh-modulesd\""
                                                                                        "},"
                                                                                        "\"command\":\"add\","
                                                                                        "\"parameters\":{"
                                                                                            "\"extra_args\":[],"
                                                                                            "\"alert\":{"
                                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                                "\"rule\":{"
                                                                                                    "\"level\":5,"
                                                                                                    "\"description\":\"File added to the system.\","
                                                                                                    "\"id\":\"554\""
                                                                                                "},"
                                                                                                "\"id\":\"1609860180.513333\","
                                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                                "\"syscheck\":{"
                                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                                    "\"mode\":\"realtime\","
                                                                                                    "\"event\":\"added\""
                                                                                                "},"
                                                                                                "\"location\":\"syscheck\""
                                                                                            "},"
                                                                                            "\"program\":\"active-response/bin/restart-wazuh.sh\""
                                                                                        "}"
                                                                                    "}'");

    will_return(__wrap_wpopenv, wfd);
    will_return(__wrap_fwrite, 0);
    will_return(__wrap_wpclose, 0);

    execd_start(queue);

    os_free(timeout_list);
}

static void test_execd_start_wpopenv_err(void **state) {
    wfd_t * wfd = *state;
    int queue = 1;
    int now = 123456789;
    char *message = "{"
                        "\"version\":\"1\","
                        "\"origin\":{"
                            "\"name\":\"node01\","
                            "\"module\":\"wazuh-analysisd\""
                        "},"
                        "\"command\":\"restart-wazuh0\","
                        "\"parameters\":{"
                            "\"extra_args\":[],"
                            "\"alert\":{"
                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                "\"rule\":{"
                                    "\"level\":5,"
                                    "\"description\":\"File added to the system.\","
                                    "\"id\":\"554\""
                                "},"
                                "\"id\":\"1609860180.513333\","
                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                "\"syscheck\":{"
                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                    "\"mode\":\"realtime\","
                                    "\"event\":\"added\""
                                "},"
                                "\"location\":\"syscheck\""
                            "}"
                        "}"
                    "}";
    int timeout = 0;

    timeout_list = OSList_Create();

    will_return(__wrap_CreateThreadJoinable, 0);

    will_return(__wrap_time, now);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    expect_string(__wrap__mtdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    expect_string(__wrap_fopen, path, DEFAULTAR);
    expect_string(__wrap_fopen, mode, "r");
    will_return(__wrap_fopen, (FILE *) 100);

    will_return(__wrap_time, now);

    expect_string(__wrap_get_command_by_name, name, "restart-wazuh0");
    will_return(__wrap_get_command_by_name, timeout);
    will_return(__wrap_get_command_by_name, "restart-wazuh");

    expect_string(__wrap__mtdebug1, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug1, formatted_msg, "Executing command 'active-response/bin/restart-wazuh.sh {"
                                                                                        "\"version\":\"1\","
                                                                                        "\"origin\":{"
                                                                                            "\"name\":\"node01\","
                                                                                            "\"module\":\"wazuh-modulesd\""
                                                                                        "},"
                                                                                        "\"command\":\"add\","
                                                                                        "\"parameters\":{"
                                                                                            "\"extra_args\":[],"
                                                                                            "\"alert\":{"
                                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                                "\"rule\":{"
                                                                                                    "\"level\":5,"
                                                                                                    "\"description\":\"File added to the system.\","
                                                                                                    "\"id\":\"554\""
                                                                                                "},"
                                                                                                "\"id\":\"1609860180.513333\","
                                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                                "\"syscheck\":{"
                                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                                    "\"mode\":\"realtime\","
                                                                                                    "\"event\":\"added\""
                                                                                                "},"
                                                                                                "\"location\":\"syscheck\""
                                                                                            "},"
                                                                                            "\"program\":\"active-response/bin/restart-wazuh.sh\""
                                                                                        "}"
                                                                                    "}'");

    expect_any(__wrap_fgets, __stream);
    will_return(__wrap_fgets, "restart-wazuh0 - restart-wazuh.sh - 0\n");

    expect_string(__wrap_fopen, path, "active-response/bin/restart-wazuh.sh");
    expect_string(__wrap_fopen, mode, "r");
    will_return(__wrap_fopen, (FILE *) 100);

    expect_value(__wrap_fclose, _File, (FILE *) 100);
    will_return(__wrap_fclose, 1);

    expect_any(__wrap_fgets, __stream);
    will_return(__wrap_fgets, NULL);

    expect_value(__wrap_fclose, _File, (FILE *) 100);
    will_return(__wrap_fclose, 1);

    will_return(__wrap_wpopenv, NULL);
    expect_string(__wrap__mterror, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mterror, formatted_msg, "(1317): Could not launch command Success (0)");

    will_return(__wrap_time, now);
    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    expect_string(__wrap__mtdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    will_return(__wrap_time, now);

    /*expect_string(__wrap_get_command_by_name, name, "restart-wazuh0");
    will_return(__wrap_get_command_by_name, timeout);
    will_return(__wrap_get_command_by_name, "restart-wazuh");*/

    expect_string(__wrap_fopen, path, "etc/ossec.conf");
    expect_string(__wrap_fopen, mode, "r");
    will_return(__wrap_fopen, (FILE *) 100);

    expect_value(__wrap_fclose, _File, (FILE *) 100);
    will_return(__wrap_fclose, 1);

    expect_string(__wrap__mtdebug1, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug1, formatted_msg, "Executing command 'restart-wazuh {"
                                                                                        "\"version\":\"1\","
                                                                                        "\"origin\":{"
                                                                                            "\"name\":\"node01\","
                                                                                            "\"module\":\"wazuh-execd\""
                                                                                        "},"
                                                                                        "\"command\":\"add\","
                                                                                        "\"parameters\":{"
                                                                                            "\"extra_args\":[],"
                                                                                            "\"alert\":{"
                                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                                "\"rule\":{"
                                                                                                    "\"level\":5,"
                                                                                                    "\"description\":\"File added to the system.\","
                                                                                                    "\"id\":\"554\""
                                                                                                "},"
                                                                                                "\"id\":\"1609860180.513333\","
                                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                                "\"syscheck\":{"
                                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                                    "\"mode\":\"realtime\","
                                                                                                    "\"event\":\"added\""
                                                                                                "},"
                                                                                                "\"location\":\"syscheck\""
                                                                                            "},"
                                                                                            "\"program\":\"restart-wazuh\""
                                                                                        "}"
                                                                                    "}'");

    will_return(__wrap_wpopenv, wfd);
    will_return(__wrap_fwrite, 0);
    will_return(__wrap_wpclose, 0);

    execd_start(queue);

    os_free(timeout_list);
}

static void test_execd_start_get_command_err(void **state) {
    wfd_t * wfd = *state;
    int queue = 1;
    int now = 123456789;
    char *message = "{"
                        "\"version\":\"1\","
                        "\"origin\":{"
                            "\"name\":\"node01\","
                            "\"module\":\"wazuh-analysisd\""
                        "},"
                        "\"command\":\"restart-wazuh0\","
                        "\"parameters\":{"
                            "\"extra_args\":[],"
                            "\"alert\":{"
                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                "\"rule\":{"
                                    "\"level\":5,"
                                    "\"description\":\"File added to the system.\","
                                    "\"id\":\"554\""
                                "},"
                                "\"id\":\"1609860180.513333\","
                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                "\"syscheck\":{"
                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                    "\"mode\":\"realtime\","
                                    "\"event\":\"added\""
                                "},"
                                "\"location\":\"syscheck\""
                            "}"
                        "}"
                    "}";
    int timeout = 0;

    will_return(__wrap_CreateThreadJoinable, 0);

    will_return(__wrap_time, now);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    will_return(__wrap_read_exec_config, 1);

    expect_string(__wrap_fopen, path, OSSECCONF);
    expect_string(__wrap_fopen, mode, "r");
    will_return(__wrap_fopen, (FILE *) 100);

    expect_value(__wrap_fclose, _File, (FILE *) 100);
    will_return(__wrap_fclose, 1);

    expect_string(__wrap__mtdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mtdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    will_return(__wrap_time, now);

    expect_string(__wrap_get_command_by_name, name, "restart-wazuh0");
    will_return(__wrap_get_command_by_name, timeout);
    will_return(__wrap_get_command_by_name, NULL);

    expect_string(__wrap_get_command_by_name, name, "restart-wazuh0");
    will_return(__wrap_get_command_by_name, timeout);
    will_return(__wrap_get_command_by_name, NULL);

    expect_string(__wrap__mterror, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mterror, formatted_msg, "(1311): Invalid command name 'restart-wazuh0' provided.");

    will_return(__wrap_time, now);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    expect_string(__wrap__mdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    will_return(__wrap_time, now);

    expect_string(__wrap_get_command_by_name, name, "restart-wazuh0");
    will_return(__wrap_get_command_by_name, timeout);
    will_return(__wrap_get_command_by_name, "restart-wazuh");

    expect_string(__wrap__mdebug1, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug1, formatted_msg, "Executing command 'restart-wazuh {"
                                                                                        "\"version\":\"1\","
                                                                                        "\"origin\":{"
                                                                                            "\"name\":\"node01\","
                                                                                            "\"module\":\"wazuh-execd\""
                                                                                        "},"
                                                                                        "\"command\":\"add\","
                                                                                        "\"parameters\":{"
                                                                                            "\"extra_args\":[],"
                                                                                            "\"alert\":{"
                                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                                "\"rule\":{"
                                                                                                    "\"level\":5,"
                                                                                                    "\"description\":\"File added to the system.\","
                                                                                                    "\"id\":\"554\""
                                                                                                "},"
                                                                                                "\"id\":\"1609860180.513333\","
                                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                                "\"syscheck\":{"
                                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                                    "\"mode\":\"realtime\","
                                                                                                    "\"event\":\"added\""
                                                                                                "},"
                                                                                                "\"location\":\"syscheck\""
                                                                                            "},"
                                                                                            "\"program\":\"restart-wazuh\""
                                                                                        "}"
                                                                                    "}'");

    will_return(__wrap_wpopenv, wfd);

    will_return(__wrap_fwrite, 0);

    will_return(__wrap_wpclose, 0);

    execd_start(queue);
}

static void test_execd_start_get_name_err(void **state) {
    wfd_t * wfd = *state;
    int queue = 1;
    int now = 123456789;
    char *message = "{}";
    char *message2 = "{"
                        "\"version\":\"1\","
                        "\"origin\":{"
                            "\"name\":\"node01\","
                            "\"module\":\"wazuh-analysisd\""
                        "},"
                        "\"command\":\"restart-wazuh0\","
                        "\"parameters\":{"
                            "\"extra_args\":[],"
                            "\"alert\":{"
                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                "\"rule\":{"
                                    "\"level\":5,"
                                    "\"description\":\"File added to the system.\","
                                    "\"id\":\"554\""
                                "},"
                                "\"id\":\"1609860180.513333\","
                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                "\"syscheck\":{"
                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                    "\"mode\":\"realtime\","
                                    "\"event\":\"added\""
                                "},"
                                "\"location\":\"syscheck\""
                            "}"
                        "}"
                    "}";
    int timeout = 0;

    will_return(__wrap_time, now);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    expect_string(__wrap__mdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug2, formatted_msg, "Received message: '{}'");

    will_return(__wrap_time, now);

    expect_string(__wrap__mterror, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mterror, formatted_msg, "(1316): Invalid AR command: '{}'");

    will_return(__wrap_time, now);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message2);
    will_return(__wrap_OS_RecvUnix, strlen(message2));

    expect_string(__wrap__mdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    will_return(__wrap_time, now);

    expect_string(__wrap_get_command_by_name, name, "restart-wazuh0");
    will_return(__wrap_get_command_by_name, timeout);
    will_return(__wrap_get_command_by_name, "restart-wazuh");

    expect_string(__wrap__mdebug1, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug1, formatted_msg, "Executing command 'restart-wazuh {"
                                                                                        "\"version\":\"1\","
                                                                                        "\"origin\":{"
                                                                                            "\"name\":\"node01\","
                                                                                            "\"module\":\"wazuh-execd\""
                                                                                        "},"
                                                                                        "\"command\":\"add\","
                                                                                        "\"parameters\":{"
                                                                                            "\"extra_args\":[],"
                                                                                            "\"alert\":{"
                                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                                "\"rule\":{"
                                                                                                    "\"level\":5,"
                                                                                                    "\"description\":\"File added to the system.\","
                                                                                                    "\"id\":\"554\""
                                                                                                "},"
                                                                                                "\"id\":\"1609860180.513333\","
                                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                                "\"syscheck\":{"
                                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                                    "\"mode\":\"realtime\","
                                                                                                    "\"event\":\"added\""
                                                                                                "},"
                                                                                                "\"location\":\"syscheck\""
                                                                                            "},"
                                                                                            "\"program\":\"restart-wazuh\""
                                                                                        "}"
                                                                                    "}'");

    will_return(__wrap_wpopenv, wfd);

    will_return(__wrap_fwrite, 0);

    will_return(__wrap_wpclose, 0);

    execd_start(queue);
}

static void test_execd_start_json_err(void **state) {
    wfd_t * wfd = *state;
    int queue = 1;
    int now = 123456789;
    char *message = "unknown";
    char *message2 = "{"
                        "\"version\":\"1\","
                        "\"origin\":{"
                            "\"name\":\"node01\","
                            "\"module\":\"wazuh-analysisd\""
                        "},"
                        "\"command\":\"restart-wazuh0\","
                        "\"parameters\":{"
                            "\"extra_args\":[],"
                            "\"alert\":{"
                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                "\"rule\":{"
                                    "\"level\":5,"
                                    "\"description\":\"File added to the system.\","
                                    "\"id\":\"554\""
                                "},"
                                "\"id\":\"1609860180.513333\","
                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                "\"syscheck\":{"
                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                    "\"mode\":\"realtime\","
                                    "\"event\":\"added\""
                                "},"
                                "\"location\":\"syscheck\""
                            "}"
                        "}"
                    "}";
    int timeout = 0;

    will_return(__wrap_time, now);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message);
    will_return(__wrap_OS_RecvUnix, strlen(message));

    expect_string(__wrap__mdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug2, formatted_msg, "Received message: 'unknown'");

    will_return(__wrap_time, now);

    expect_string(__wrap__mterror, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mterror, formatted_msg, "(1315): Invalid JSON message: 'unknown'");

    will_return(__wrap_time, now);

    will_return(__wrap_select, 1);

    expect_value(__wrap_OS_RecvUnix, socket, queue);
    expect_value(__wrap_OS_RecvUnix, sizet, OS_MAXSTR);
    will_return(__wrap_OS_RecvUnix, message2);
    will_return(__wrap_OS_RecvUnix, strlen(message2));

    expect_string(__wrap__mdebug2, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug2, formatted_msg, "Received message: '{"
                                                                        "\"version\":\"1\","
                                                                        "\"origin\":{"
                                                                            "\"name\":\"node01\","
                                                                            "\"module\":\"wazuh-analysisd\""
                                                                        "},"
                                                                        "\"command\":\"restart-wazuh0\","
                                                                        "\"parameters\":{"
                                                                            "\"extra_args\":[],"
                                                                            "\"alert\":{"
                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                "\"rule\":{"
                                                                                    "\"level\":5,"
                                                                                    "\"description\":\"File added to the system.\","
                                                                                    "\"id\":\"554\""
                                                                                "},"
                                                                                "\"id\":\"1609860180.513333\","
                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                "\"syscheck\":{"
                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                    "\"mode\":\"realtime\","
                                                                                    "\"event\":\"added\""
                                                                                "},"
                                                                                "\"location\":\"syscheck\""
                                                                            "}"
                                                                        "}"
                                                                    "}'");

    will_return(__wrap_time, now);

    expect_string(__wrap_get_command_by_name, name, "restart-wazuh0");
    will_return(__wrap_get_command_by_name, timeout);
    will_return(__wrap_get_command_by_name, "restart-wazuh");

    expect_string(__wrap__mdebug1, tag, WM_EXECD_LOGTAG);
    expect_string(__wrap__mdebug1, formatted_msg, "Executing command 'restart-wazuh {"
                                                                                        "\"version\":\"1\","
                                                                                        "\"origin\":{"
                                                                                            "\"name\":\"node01\","
                                                                                            "\"module\":\"wazuh-execd\""
                                                                                        "},"
                                                                                        "\"command\":\"add\","
                                                                                        "\"parameters\":{"
                                                                                            "\"extra_args\":[],"
                                                                                            "\"alert\":{"
                                                                                                "\"timestamp\":\"2021-01-05T15:23:00.547+0000\","
                                                                                                "\"rule\":{"
                                                                                                    "\"level\":5,"
                                                                                                    "\"description\":\"File added to the system.\","
                                                                                                    "\"id\":\"554\""
                                                                                                "},"
                                                                                                "\"id\":\"1609860180.513333\","
                                                                                                "\"full_log\":\"File '/home/vagrant/file/n41.txt' added\\nMode: realtime\\n\","
                                                                                                "\"syscheck\":{"
                                                                                                    "\"path\":\"/home/vagrant/file/n41.txt\","
                                                                                                    "\"mode\":\"realtime\","
                                                                                                    "\"event\":\"added\""
                                                                                                "},"
                                                                                                "\"location\":\"syscheck\""
                                                                                            "},"
                                                                                            "\"program\":\"restart-wazuh\""
                                                                                        "}"
                                                                                    "}'");

    will_return(__wrap_wpopenv, wfd);

    will_return(__wrap_fwrite, 0);

    will_return(__wrap_wpclose, 0);

    execd_start(queue);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_execd_start_ok, test_setup_file, test_teardown_file),
        cmocka_unit_test_setup_teardown(test_execd_start_timeout, test_setup_file, test_teardown_file),
        //cmocka_unit_test_setup_teardown(test_execd_start_wpopenv_err, test_setup_file, test_teardown_file),
        //cmocka_unit_test_setup_teardown(test_execd_start_get_command_err, test_setup_file, test_teardown_file),
        //cmocka_unit_test_setup_teardown(test_execd_start_get_name_err, test_setup_file, test_teardown_file),
        //cmocka_unit_test_setup_teardown(test_execd_start_json_err, test_setup_file, test_teardown_file),
    };

    return cmocka_run_group_tests(tests, group_setup, group_teardown);
}
