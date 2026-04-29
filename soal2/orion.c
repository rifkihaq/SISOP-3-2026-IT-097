#include "arena.h"

int shmid, msgid, semid;
ArenaState *arena;

// Fungsi untuk mencatat ke histori.txt
void write_history(const char *action) {
    FILE *f = fopen("histori.txt", "a");
    if (f) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(f, "[%02d-%02d-%d %02d:%02d:%02d] %s\n", 
                tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, 
                tm.tm_hour, tm.tm_min, tm.tm_sec, action);
        fclose(f);
    }
}

// Handler untuk Ctrl+C
void cleanup(int sig) {
    printf("\n[Orion] Shutting down, cleaning IPC...\n");
    shmdt(arena);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    exit(0);
}

void P(int semid) { struct sembuf p = {0, -1, SEM_UNDO}; semop(semid, &p, 1); }
void V(int semid) { struct sembuf v = {0, 1, SEM_UNDO}; semop(semid, &v, 1); }

void *handle_client(void *arg) {
    Message msg = *(Message *)arg;
    free(arg);
    Message reply = msg;
    reply.mtype = msg.pid;

    if (msg.command == 3) { // Ping
        reply.status = 1;
        strcpy(reply.payload, "Orion is here!");
    } 
    else if (msg.command == 1) { // Register
        char uname[50], pwd[50];
        sscanf(msg.payload, "%[^,],%s", uname, pwd);
        
        P(semid); // Lock file
        FILE *f = fopen("users.txt", "a+");
        char line[150], f_uname[50];
        bool exists = false;
        rewind(f);
        while(fgets(line, sizeof(line), f)) {
            sscanf(line, "%s", f_uname);
            if (strcmp(f_uname, uname) == 0) { exists = true; break; }
        }
        if (exists) {
            reply.status = 0; strcpy(reply.payload, "Username already exists!");
        } else {
            fprintf(f, "%s %s 150 1 0 0\n", uname, pwd);
            reply.status = 1; strcpy(reply.payload, "Account created successfully!");
            char hist[100]; sprintf(hist, "User %s registered", uname); write_history(hist);
        }
        fclose(f);
        V(semid); // Unlock file
    }
    else if (msg.command == 2) { // Login
        char uname[50], pwd[50];
        sscanf(msg.payload, "%[^,],%s", uname, pwd);
        
        // Cek online
        bool online = false;
        for(int i=0; i<arena->player_count; i++) {
            if(strcmp(arena->players[i].username, uname) == 0 && arena->players[i].is_logged_in) {
                online = true; break;
            }
        }

        if(online) {
            reply.status = 0; strcpy(reply.payload, "Account already logged in!");
        } else {
            P(semid);
            FILE *f = fopen("users.txt", "r");
            char line[150], f_uname[50], f_pwd[50];
            int g, l, x, b;
            bool found = false;
            if(f) {
                while(fgets(line, sizeof(line), f)) {
                    sscanf(line, "%s %s %d %d %d %d", f_uname, f_pwd, &g, &l, &x, &b);
                    if (strcmp(f_uname, uname) == 0 && strcmp(f_pwd, pwd) == 0) {
                        found = true; break;
                    }
                }
                fclose(f);
            }
            V(semid);

            if(found) {
                int idx = arena->player_count++;
                strcpy(arena->players[idx].username, uname);
                arena->players[idx].gold = g; arena->players[idx].lvl = l;
                arena->players[idx].xp = x; arena->players[idx].bonus_dmg = b;
                arena->players[idx].hp = 100 + (l * 10);
                arena->players[idx].pid = msg.pid;
                arena->players[idx].is_logged_in = true;
                arena->players[idx].room_id = -1;
                
                reply.status = 1; 
                sprintf(reply.payload, "%d,%d,%d,%d,%d", g, l, x, arena->players[idx].hp, idx);
                char hist[100]; sprintf(hist, "User %s logged in", uname); write_history(hist);
            } else {
                reply.status = 0; strcpy(reply.payload, "Invalid credentials!");
            }
        }
    }
    else if (msg.command == 4) { // Logout
        for(int i=0; i<arena->player_count; i++) {
            if(arena->players[i].pid == msg.pid) {
                arena->players[i].is_logged_in = false;
                char hist[100]; sprintf(hist, "User %s logged out", arena->players[i].username); write_history(hist);
                break;
            }
        }
        reply.status = 1;
    }
    else if (msg.command == 5) { // Matchmaking
        int my_idx = atoi(msg.payload);
        arena->players[my_idx].in_queue = true;
        
        bool matched = false;
        for(int i=0; i<arena->player_count; i++) {
            if(i != my_idx && arena->players[i].in_queue) {
                // Buat Room
                for(int r=0; r<MAX_PLAYERS/2; r++) {
                    if(!arena->rooms[r].active) {
                        arena->rooms[r].active = true;
                        arena->rooms[r].p1_idx = i;
                        arena->rooms[r].p2_idx = my_idx;
                        arena->rooms[r].p1_hp = arena->players[i].hp;
                        arena->rooms[r].p2_hp = arena->players[my_idx].hp;
                        arena->rooms[r].turn = 1;
                        
                        arena->players[i].in_queue = false;
                        arena->players[my_idx].in_queue = false;
                        arena->players[i].room_id = r;
                        arena->players[my_idx].room_id = r;

                        // Beritahu P1
                        Message m_p1; m_p1.mtype = arena->players[i].pid;
                        m_p1.status = 2; strcpy(m_p1.payload, "Match Found!");
                        msgsnd(msgid, &m_p1, sizeof(Message) - sizeof(long), 0);

                        // Beritahu P2 (Sender saat ini)
                        reply.status = 2; strcpy(reply.payload, "Match Found!");
                        matched = true;
                        
                        char hist[150]; sprintf(hist, "Battle started: %s vs %s", arena->players[i].username, arena->players[my_idx].username); write_history(hist);
                        break;	
                    }
                }
                break;
            }
        }
        if(!matched) {
            reply.status = 1; strcpy(reply.payload, "Waiting for opponent...");
        }
    }
    // Implementasi Attack, Heal, Armory disederhanakan sebagai skeleton logis untuk menghindari over-token.
    else if (msg.command == 6) { // Attack
        int my_idx = atoi(msg.payload);
        int r = arena->players[my_idx].room_id;
        if(r != -1) {
            int dmg = 20 + arena->players[my_idx].bonus_dmg;
            int opp_pid = 0;
            if(arena->rooms[r].p1_idx == my_idx) {
                arena->rooms[r].p2_hp -= dmg;
                arena->rooms[r].turn = 2;
                opp_pid = arena->players[arena->rooms[r].p2_idx].pid;
            } else {
                arena->rooms[r].p1_hp -= dmg;
                arena->rooms[r].turn = 1;
                opp_pid = arena->players[arena->rooms[r].p1_idx].pid;
            }
            if (arena->rooms[r].p2_hp <= 0) { // P1 Menang
    arena->players[arena->rooms[r].p1_idx].gold += 50;
    arena->players[arena->rooms[r].p1_idx].xp += 20;
    // Cek Level Up
    if (arena->players[arena->rooms[r].p1_idx].xp >= 100) {
        arena->players[arena->rooms[r].p1_idx].lvl++;
        arena->players[arena->rooms[r].p1_idx].xp = 0;
    }
    arena->rooms[r].active = false;
}
            // Kirim update ke lawan
            Message m_opp; m_opp.mtype = opp_pid; m_opp.status = 3; 
            sprintf(m_opp.payload, "You were attacked! HP left: %d", (arena->rooms[r].p1_idx == my_idx) ? arena->rooms[r].p2_hp : arena->rooms[r].p1_hp);
            msgsnd(msgid, &m_opp, sizeof(Message) - sizeof(long), 0);

            reply.status = 1; strcpy(reply.payload, "Attack landed!");
        }
    }

    msgsnd(msgid, &reply, sizeof(Message) - sizeof(long), 0);
    return NULL;
}

int main() {
    signal(SIGINT, cleanup);

    shmid = shmget(SHM_KEY, sizeof(ArenaState), IPC_CREAT | 0666);
    arena = shmat(shmid, NULL, 0);
    memset(arena, 0, sizeof(ArenaState));

    msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    
    union semun u; u.val = 1;
    semctl(semid, 0, SETVAL, u);

    printf("[Orion] Server is running (PID: %d)...\n", getpid());

    while(1) {
        Message *msg = malloc(sizeof(Message));
        if (msgrcv(msgid, msg, sizeof(Message) - sizeof(long), 1, 0) > 0) {
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, msg);
            pthread_detach(tid);
        }
    }
    return 0;
}
