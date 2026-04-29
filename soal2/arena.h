#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define SHM_KEY 0x00001234
#define MSG_KEY 0x00005678
#define SEM_KEY 0x00009012
#define MAX_PLAYERS 100

// Struktur untuk Semaphore
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

typedef struct {
    char username[50];
    int gold;
    int lvl;
    int xp;
    int bonus_dmg;
    int hp;
    pid_t pid;
    bool is_logged_in;
    bool in_queue;
    int room_id; // -1 jika tidak di dalam battle
} Player;

typedef struct {
    int p1_idx;
    int p2_idx;
    int p1_hp;
    int p2_hp;
    int turn; // 1 untuk p1, 2 untuk p2
    bool active;
} BattleRoom;

typedef struct {
    Player players[MAX_PLAYERS];
    int player_count;
    BattleRoom rooms[MAX_PLAYERS/2];
} ArenaState;

typedef struct {
    long mtype;       // Tujuan pesan (1 untuk server, PID untuk client)
    pid_t pid;        // Pengirim
    int command;      /* 1:Reg, 2:Login, 3:Ping, 4:Logout, 5:Match, 
                         6:Attack, 7:Heal, 8:Flee, 9:BuyArmory, 10:History */
    char payload[256];
    int status;       // 1: Sukses, 0: Gagal, 2: Update Battle
} Message;

#endif
