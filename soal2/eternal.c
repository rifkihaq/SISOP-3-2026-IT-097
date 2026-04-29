#include "arena.h"

int msgid;
int my_idx = -1;
bool in_battle = false;
char my_username[50];

// Thread untuk mendengarkan pesan asinkron dari server (seperti saat diserang lawan)
void *listen_server(void *arg) {
    Message msg;
    while(1) {
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), getpid(), 0) > 0) {
            if (msg.status == 2) {
                printf("\n[!] %s\n", msg.payload);
                in_battle = true;
            } else if (msg.status == 3) {
                printf("\n[BATTLE UPDATE] %s\n> ", msg.payload);
            } else if (msg.command != 3) { // Skip ping
                // Pesan sinkron biasa akan ditangkap oleh msgrcv di fungsi main/menu
                msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0); // Kembalikan ke queue agar dibaca fungsi pemanggil
            }
        }
    }
    return NULL;
}

void battle_mode() {
    Message m; m.mtype = 1; m.pid = getpid(); m.command = 5;
    sprintf(m.payload, "%d", my_idx);
    msgsnd(msgid, &m, sizeof(Message) - sizeof(long), 0);
    
    printf("Waiting for opponent...\n");
    while(!in_battle) { sleep(1); } // Tunggu sampai thread listener mengubah status

    int action;
    while(in_battle) {
        printf("\n=== BATTLE MENU ===\n");
        printf("1. Attack\n2. Heal\n3. Flee\n> ");
        scanf("%d", &action);
        
        m.mtype = 1; m.pid = getpid(); 
        if(action == 1) m.command = 6;
        else if (action == 2) m.command = 7;
        else if (action == 3) { m.command = 8; in_battle = false; break; }
        
        sprintf(m.payload, "%d", my_idx);
        msgsnd(msgid, &m, sizeof(Message) - sizeof(long), 0);
        sleep(1); // Jeda sejenak menunggu balasan update diolah thread
    }
}

void main_menu(int gold, int lvl, int xp, int hp) {
    int choice;
    while(1) {
        printf("\n=== PROFILE ===\n");
        printf("Name: %s | Lvl: %d | HP: %d\n", my_username, lvl, hp);
        printf("Gold: %d | XP: %d\n", gold, xp);
        printf("===============\n");
        printf("1. Battle\n2. Armory\n3. History\n4. Logout\n> ");
        scanf("%d", &choice);

        if (choice == 1) battle_mode();
        else if (choice == 2) printf("Armory feature selected...\n");
        else if (choice == 3) system("cat histori.txt 2>/dev/null || echo 'No history yet.'");
        else if (choice == 4) {
            Message m; m.mtype = 1; m.pid = getpid(); m.command = 4;
            msgsnd(msgid, &m, sizeof(Message) - sizeof(long), 0);
            printf("Logged out.\n");
            break;
        }
    }
}

void login_register(int type) {
    char uname[50], pwd[50];
    printf("Username: "); scanf("%s", uname);
    printf("Password: "); scanf("%s", pwd);

    Message m; m.mtype = 1; m.pid = getpid(); m.command = type;
    sprintf(m.payload, "%s,%s", uname, pwd);
    msgsnd(msgid, &m, sizeof(Message) - sizeof(long), 0);

    Message reply;
    msgrcv(msgid, &reply, sizeof(Message) - sizeof(long), getpid(), 0);
    
    printf("%s\n", reply.payload);
    if (type == 2 && reply.status == 1) {
        int g, l, x, h;
        sscanf(reply.payload, "%d,%d,%d,%d,%d", &g, &l, &x, &h, &my_idx);
        strcpy(my_username, uname);
        
        // Jalankan thread listener
        pthread_t tid;
        pthread_create(&tid, NULL, listen_server, NULL);
        
        main_menu(g, l, x, h);
    }
}

int main() {
    msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) { printf("Orion are you there?\n"); exit(1); }

    Message ping; ping.mtype = 1; ping.pid = getpid(); ping.command = 3;
    if (msgsnd(msgid, &ping, sizeof(Message) - sizeof(long), IPC_NOWAIT) == -1) {
        printf("Orion are you there?\n"); exit(1);
    }
    
    Message reply;
    if (msgrcv(msgid, &reply, sizeof(Message) - sizeof(long), getpid(), 0) < 0) {
        printf("Orion are you there?\n"); exit(1);
    }

    int choice;
    while(1) {
        printf("\n=== ETERION ===\n1. Register\n2. Login\n3. Exit\n> ");
        scanf("%d", &choice);
        if (choice == 1 || choice == 2) login_register(choice);
        else if (choice == 3) break;
    }
    return 0;
}
