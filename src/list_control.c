//
// Created by leok on 2022/1/20.
//

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

#include "list_control.h"


#define TERMINAL_LIST_DB_FILE "./terminal_list.db"




void dumpData_leok(const unsigned char *buf, size_t length) {
    printf("leok----dump data: ");
    int n = 0;
    char dat[17] = {0};
    for (size_t i = 0; i < length; ++i) {
        dat[n] = buf[i];
        printf("%02x ", buf[i]);
        if (++n >= 16 || i == length - 1) {
            if (i == length - 1) {
                if (length % 16 != 0) {
                    for (size_t blk = 0; blk < 16 - length % 16; ++blk) {
                        printf("   ");
                    }
                }
            }

            printf("\t");
            for (int k = 0; k < n; k++) {
                if (dat[k] >= 32 && dat[k] <= 126) {
                    printf("%c", dat[k]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
            n = 0;
        }
    }
    printf("\n");
}



int init_table_list()
{
    sqlite3 *db;
    sqlite3_stmt *pStmt;
    char *err_msg = 0;

    printf("init_table_list start---------------------\n");

    if (sqlite3_open(TERMINAL_LIST_DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

//    const char *sql = "CREATE TABLE IF NOT EXISTS TerminalInfo( \
//        terminal_label CHAR(16) PRIMARY KEY,  \
//        terminal_id CHAR(8),  \
//        terminal_pid CHAR(4),  \
//        terminal_vid CHAR(4),  \
//        terminal_mac CHAR(16));";

    const char *sql = "CREATE TABLE IF NOT EXISTS TerminalInfo( \
        terminal_label VARCHAR(16) PRIMARY KEY,  \
        terminal_id VARCHAR(8),  \
        terminal_pid VARCHAR(4),  \
        terminal_vid VARCHAR(4),  \
        terminal_mac VARCHAR(16));";
    if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "%s, SQL error: %s\n", __func__ , err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
#if 0
    if (sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &pStmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    if (sqlite3_step(pStmt) == SQLITE_ROW)
        printf("%s\n", sqlite3_column_text(pStmt, 0));

    sqlite3_finalize(pStmt);
#endif
    sqlite3_close(db);

    printf("init_table_list end---------------------\n");

    return 0;
}


int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        dumpData_leok(argv[i], sizeof(argv[i]));
    }

    printf("\n");

    return 0;
}

int dump_list()
{
    sqlite3 *db;
    char *err_msg = 0;

    printf("dump_list start---------------------\n");
    if (sqlite3_open(TERMINAL_LIST_DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, "%s Cannot open database: %s\n", __func__ , sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    char *sql = "SELECT * FROM TerminalInfo;";
    if (sqlite3_exec(db, sql, callback, 0, &err_msg) != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db);
    printf("dump_list end---------------------\n");
    return 0;

}



int insert_item_to_list(void *data)
{
    sqlite3 *db;
//    sqlite3_stmt *pStmt;
    char *err_msg = 0;

    printf("insert_item_to_list start---------------------\n");

    if (init_table_list() != 0) {
        printf("%s init_table_list failed\n", __func__);
        return 1;
    }

    if (sqlite3_open(TERMINAL_LIST_DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, "%s Cannot open database: %s\n", __func__, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    TerminalInfo *tm_info = (TerminalInfo *)data;
    uint8_t terminal_label[17] = {0};
    snprintf(terminal_label, sizeof(terminal_label), "%02x%02x%02x%02x%02x%02x%02x%02x",
             tm_info->id[0],
             tm_info->id[1],
             tm_info->id[2],
             tm_info->id[3],
             tm_info->pid[0],
             tm_info->pid[1],
             tm_info->vid[0],
             tm_info->vid[1]);
    printf("terminal_label:\n");
    dumpData_leok(terminal_label, 17);

    printf("id:\n");
    uint8_t id[9] = {0};
    snprintf(id, sizeof(id), "%02x%02x%02x%02x",
             tm_info->id[0],
             tm_info->id[1],
             tm_info->id[2],
             tm_info->id[3]);
    dumpData_leok(id, 9);

    printf("pid:\n");
    uint8_t pid[5] = {0};
    snprintf(pid, sizeof(pid), "%02x%02x",
             tm_info->pid[0],
             tm_info->pid[1]);
    dumpData_leok(pid, 5);

    printf("vid:\n");
    uint8_t vid[5] = {0};
    snprintf(vid, sizeof(vid), "%02x%02x", tm_info->vid[0],
             tm_info->vid[1]);
    dumpData_leok(vid, 5);

    printf("mac:\n");
    uint8_t mac[17] = {0};
    snprintf(mac, sizeof(mac), "%02x%02x%02x%02x%02x%02x%02x%02x",
             tm_info->mac[0],
             tm_info->mac[1],
             tm_info->mac[2],
             tm_info->mac[3],
             tm_info->mac[4],
             tm_info->mac[5],
             tm_info->mac[6],
             tm_info->mac[7]);
    dumpData_leok(mac, 17);

    char sql[512] = {0};
    snprintf(sql, sizeof(sql), "INSERT INTO TerminalInfo VALUES('%s', '%s', '%s', '%s', '%s');",
             terminal_label,
             id,
             pid,
             vid,
             mac);
    dumpData_leok(sql, sizeof(sql));

    if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "%s, SQL error: %s\n", __func__ , err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db);

    dump_list();

    printf("insert_item_to_list end---------------------\n");
    return 0;
    
}

uint8_t *get_mac_from_list(void *data)
{

    printf("get_mac_from_list start---------------------\n");

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *pStmt;

    if (sqlite3_open(TERMINAL_LIST_DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, "%s Cannot open database: %s\n", __func__, sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    char *sql = "SELECT Id, Name FROM Cars WHERE terminal_label = @id";
    if (sqlite3_prepare_v2(db, sql, -1, &pStmt, 0) == SQLITE_OK){
        fprintf(stderr, "%s Failed to execute statement: %s\n", __func__ , sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    int idx = sqlite3_bind_parameter_index(pStmt, "@id");
//    uint8_t *terminal_label = (uint8_t *)data;
    uint8_t terminal_label[16] = {0};
//    snprintf(terminal_label, sizeof(terminal_label), "%s", (uint8_t *)data);
    memcpy(terminal_label, (uint8_t *)data, sizeof(terminal_label));
    printf("get mac terminal_label idx : %d\n", idx);
    dumpData_leok(terminal_label, sizeof(terminal_label));
    sqlite3_bind_text(pStmt, idx, terminal_label, sizeof(terminal_label), NULL);

    if (sqlite3_step(pStmt) == SQLITE_ROW) {
        printf("terminal_label: %s ", sqlite3_column_text(pStmt, 0));
        printf("terminal_id: %s ", sqlite3_column_text(pStmt, 1));
        printf("terminal_pid: %s ", sqlite3_column_text(pStmt, 2));
        printf("terminal_vid: %s ", sqlite3_column_text(pStmt, 3));
        printf("terminal_mac: %s\n", sqlite3_column_text(pStmt, 4));
        uint8_t *mac = malloc(sizeof(uint8_t)*8);
        memcpy(mac, sqlite3_column_text(pStmt, 4), 8);
        printf("%s mac: %s\n", __func__, mac);
        sqlite3_finalize(pStmt);
        sqlite3_close(db);
        printf("get_mac_from_list found---------------------\n");
        return mac;
    }

    sqlite3_finalize(pStmt);
    sqlite3_close(db);
    
    printf("get_mac_from_list end---------------------\n");
    return NULL;

}

