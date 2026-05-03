# SISOP-3-2026-IT-097
MUHAMMAD SALMAN RIFKI HAQ

5027251097

## Soal 1
Pada soal 1, dibuat sebuah sistem komunikasi bernama The Wired.
Sistem ini terdiri dari:

wired.c sebagai server

navi.c sebagai client

protocol.h sebagai file header bersama

protocol.c sebagai file pendukung

Makefile untuk proses kompilasi

Tujuan program ini adalah membuat komunikasi antar client melalui server dengan fitur identitas unik, broadcast pesan, dan logging aktivitas.

### # protocol.h

File ini berisi definisi yang dipakai bersama oleh server dan client, seperti:

port server
ukuran buffer
struktur data client

Fungsi utamanya adalah agar konstanta dan struktur yang sama bisa dipakai oleh beberapa file tanpa menulis ulang.
```
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char name[50];
} Client;

#endif
```

### # protocol.c

File ini merupakan file pendukung untuk modul protocol.
Kalau di implementasi kamu isinya masih sederhana, file ini tetap dipakai agar struktur program lebih terpisah dan mudah dikembangkan.
```
#include "protocol.h"
```
### # wired.c

File ini adalah inti dari server.
Fungsi yang dilakukan server:

membuat socket server
bind ke alamat dan port
menunggu koneksi client
menerima nama user
memastikan nama user tidak sama dengan client lain
menyimpan daftar client yang aktif
mengirim pesan ke client lain dengan mekanisme broadcast
mencatat aktivitas ke file history.log

Selain itu, server juga menangani perintah keluar seperti /exit, sehingga koneksi client dapat ditutup dengan rapi.

[Lihat kode wired.c](./soal1/wired.c)

### # navi.c

File ini adalah program client.
Fungsi yang dilakukan client:

membuat koneksi ke server
mengirim nama user
mengirim pesan ke server
menerima pesan dari server
menampilkan pesan dari client lain

Client inilah yang dipakai pengguna untuk bergabung ke The Wired.

[Lihat kode navi.c](./soal1/navi.c)

### # Makefile

File ini digunakan untuk mempermudah kompilasi program.

Dengan make, program server dan client dapat dibangun tanpa harus mengetik command gcc satu per satu.
```
all:
	gcc wired.c protocol.c -o wired
	gcc navi.c protocol.c -o navi
```
### OUTPUT
<img width="952" height="81" alt="Screenshot 2026-05-03 184024" src="https://github.com/user-attachments/assets/e988c866-24a2-4453-8c4e-0d010b1d98b3" />
<img width="1476" height="127" alt="Screenshot 2026-05-03 184447" src="https://github.com/user-attachments/assets/8602566a-2c69-49c7-8d3d-417656207d5d" />
<img width="1474" height="511" alt="Screenshot 2026-05-03 184423" src="https://github.com/user-attachments/assets/ea3da3e5-1888-4aea-9976-bdf0aa3c2724" />

## Soal 2
Pada soal 2, dibuat sistem komunikasi antar proses dengan konsep IPC (Inter Process Communication).
Program ini menggunakan dua proses utama:

orion.c sebagai server

eternal.c sebagai client

File pendukungnya adalah:

arena.h untuk definisi struktur dan konfigurasi
Makefile untuk kompilasi dan pembersihan IPC

Komunikasi pada soal ini tidak menggunakan RPC, tetapi berjalan secara lokal melalui IPC seperti Message Queue dan Shared Memory.

### # arena.h

File ini berisi bagian penting yang dipakai bersama oleh orion.c dan eternal.c, seperti:

definisi struct

konfigurasi program

key IPC

kebutuhan data lain yang dipakai saat pertarungan

Fungsinya adalah sebagai pusat pengaturan sistem Battle Eterion.

### # orion.c

File ini berperan sebagai server.

Tugas utama orion adalah:

menyiapkan layanan yang akan dipakai client

menerima koneksi/permintaan dari eternal

menjaga komunikasi internal antar proses

menjadi pusat pengolahan data pertarungan

Dari sisi konsep, orion adalah pihak yang selalu siap menerima dan memproses informasi.

### # eternal.c

File ini berperan sebagai client.

Tugas utama eternal adalah:

menampilkan menu utama

melakukan register

melakukan login

mengirim data ke server

menerima respons dari server

Jadi, eternal adalah antarmuka yang dipakai pengguna untuk berinteraksi dengan sistem Battle Eterion.

### # Makefile

File ini mempermudah proses kompilasi dan pembersihan IPC.

Biasanya Makefile digunakan untuk:

compile orion dan eternal

membersihkan file hasil kompilasi

menghapus shared memory dan message queue yang masih tersisa

Perintah tambahan seperti make clear_ipc sangat membantu saat program sudah pernah dijalankan dan masih menyisakan IPC di sistem.
