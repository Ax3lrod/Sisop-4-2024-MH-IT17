# LAPRES SOAL SHIFT MODUL 4

## Anggota Kelompok

1. Aryasatya Alaauddin 5027231082 (Mengerjakan soal 2)
2. Diandra Naufal Abror 5027231004 (Mengerjakan soal 1)
3. Muhamad Rizq Taufan 5027231021 (Mengerjakan soal 3)

## NOMOR 1
`Diandra Naufal A`Adfi merupakan seorang CEO agency creative bernama Ini Karya Kita. Ia sedang melakukan inovasi pada manajemen project photography Ini Karya Kita. Salah satu ide yang dia kembangkan adalah tentang pengelolaan foto project dalam sistem arsip Ini Karya Kita. Dalam membangun sistem ini, Adfi tidak bisa melakukannya sendirian, dia perlu bantuan mahasiswa Departemen Teknologi Informasi angkatan 2023 untuk membahas konsep baru yang akan mengubah project fotografinya lebih menarik untuk dilihat. Adfi telah menyiapkan portofolio hasil project fotonya yang bisa didownload dan diakses di www.inikaryakita.id . Silahkan eksplorasi web Ini Karya Kita dan temukan halaman untuk bisa mendownload projectnya. Setelah kalian download terdapat folder gallery dan bahaya.
- Pada folder “gallery”:
  - Membuat folder dengan prefix "wm." Dalam folder ini, setiap gambar yang dipindahkan ke dalamnya akan diberikan watermark bertuliskan inikaryakita.id. 
			Ex: "mv ikk.jpeg wm-foto/" 
Output: 

Before: (tidak ada watermark bertuliskan inikaryakita.id)

After: (terdapat watermark tulisan inikaryakita.id)


- Pada folder "bahaya," terdapat file bernama "script.sh." Adfi menyadari pentingnya menjaga keamanan dan integritas data dalam folder ini. 
  - Mereka harus mengubah permission pada file "script.sh" agar bisa dijalankan, karena jika dijalankan maka dapat menghapus semua dan isi dari  "gallery"
  - Adfi dan timnya juga ingin menambahkan fitur baru dengan membuat file dengan prefix "test" yang ketika disimpan akan mengalami pembalikan (reverse) isi dari file tersebut.
### Solusi
#### 1. Deklarasi _Root Path_
```
static const char *root_path = "/home/ubuntu/portofolio";
```
Lokasi direktori untuk menyimpan _root path_ dari _file_ yang akan di-_mount_.
#### 2. Mendapatkan Atribut _File_
```
static int fs_getattr(const char *path, struct stat *stbuf) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = lstat(fullpath, stbuf);
    if (res == -1)
        return -errno;
    return 0;
}
```
Menetapkan _buffer_ `fullpath` untuk menyimpan _path_ dengan menggabungkan `root_path` dan `path`. Lalu mengeksekusi `lstat` untuk membaca atribut _file_, jika gagal maka menghasilkan kode _error_.
#### 3. Membaca Konten Direktori
```
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    DIR *dp = opendir(fullpath);
    if (dp == NULL)
        return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (filler(buf, de->d_name, NULL, 0))
            break;
    }

    closedir(dp);
    return 0;
}
```
Setelah menetapkan _buffer_, kode akan membuka direktori pada _path_ yang diberikan dan memberinya _pointer_ sebagai entri direktori. Setelah _buffer_ penuh dari membaca keseluruhan entri, maka akan menutup direktorinya. Jika gagal membuka _path_ maka akan muncul kode _error_.
#### 4. Membuat Direktori Baru
```
static int fs_mkdir(const char *path, mode_t mode) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = mkdir(fullpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}
```
Esensinya untuk membuat direktori baru sesuai dengan mode akses yang diberikan, jika operasi gagal maka akan memunculkan kode _error_.
#### 5. Mengganti Nama Direktori
```
static int fs_rename(const char *from, const char *to) {
    char from_path[1024], to_path[1024];
    snprintf(from_path, sizeof(from_path), "%s%s", root_path, from);
    snprintf(to_path, sizeof(to_path), "%s%s", root_path, to);

    int res = rename(from_path, to_path);
    if (res == -1)
        return -errno;

    if (strstr(to, "/gallery/") != NULL) {
        char watermark_cmd[4096];
        snprintf(watermark_cmd, sizeof(watermark_cmd), "convert '%s' -gravity south -geometry +0+20 -fill white -pointsize 36 -annotate +0+0 'inikaryakita.id' '%s'", to_path, to_path);
        system(watermark_cmd);
    }

    return 0;
}
```
`rename` akan mengganti nama direktori, lalu mengecek apakah _path_ tujuan benar akan menuju "/gallery". Yang kemudian perintah `system` akan dieksekusi untuk menambahkan _watermark_ dengan `convert`.
#### 6. Mengubah Mode _File_
```
static int fs_chmod(const char *path, mode_t mode) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = chmod(fullpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}
```
Mengeksekusi `chmod` untuk mengubah mode akses _file_.
#### 7. Membuat _File_ Baru
```
static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int fd = creat(fullpath, mode);
    if (fd == -1)
        return -errno;
    fi->fh = fd;
    return 0;
}
```
Operasi `creat` digunakan untuk membuat _file_ baru sesuai dengan mode akses yang diberikan lalu menyimpannya di `fi`.
#### 8. Menghapus _file_
```
static int fs_unlink(const char *path) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = unlink(fullpath);
    if (res == -1)
        return -errno;
    return 0;
}
```
Mengeksekusi `unlink` untuk menghapus _file_ pada _path_ yang diberikan.
#### 9. Membalikkan Isi _Buffer_
```
static void reverse_content(char *buffer, size_t size) {
    for (size_t i = 0, j = size - 1; i < j; i++, j--) {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}

static int contains_substring(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}
```
Menjalankan _loop_ untuk membalikkan isi _buffer_ dan menukar posisi `i` dan `j`. Lalu `strstr` digunakan untuk mengecek keberadaan _substring_.
#### 10. Membaca Isi _File_
```
static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int fd = open(fullpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    ssize_t res = pread(fd, buf, size, offset);
    if (res == -1) {
        close(fd);
        return -errno;
    }

    if (contains_substring(path, "ver") || contains_substring(path, "test")) {
        reverse_content(buf, res);
    }

    close(fd);
    return res;
}
```
`open` akan membuka _file_ untuk dibaca yang kemudian diisikan ke _buffer dari posisi `offset` dengan operasi `pread`. Lalu kode akan mengecek apakah _path_ terdapat kata kunci "ver" atau "test", lalu akan membalikkannya jika ada. Terakhir _file_ akan ditutup.
#### 11. Menulis Data ke _File_
```
static int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int fd = open(fullpath, O_WRONLY);
    if (fd == -1)
        return -errno;

    ssize_t res = pwrite(fd, buf, size, offset);
    if (res == -1) {
        close(fd);
        return -errno;
    }

    close(fd);
    return res;
}
```
Membuka _file_ untuk `pwrite`, menuliskan _buffer_ dari posisi `offset` dan muncul kode _error_ apabila operasi `pwrite` gagal. Terakhir adalah menutup _file_.
#### 12. Operasi FUSE
```
static struct fuse_operations fs_operations = {
    .getattr    = fs_getattr,
    .readdir    = fs_readdir,
    .mkdir      = fs_mkdir,
    .rename     = fs_rename,
    .chmod      = fs_chmod,
    .create     = fs_create,
    .unlink     = fs_unlink,
    .read       = fs_read,
    .write      = fs_write,
};
```
Menjalankan argumen yang mengacu pada kumpulan penjelasan di atas.
#### 13. Fungsi `main`
```
int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &fs_operations, NULL);
}
```
Memanggil `fuse_main` dengan argumen _input_ yang diberikan _user_ sesuai dengan operasi `fs_operations` yang telah didefinisikan.
## NOMOR 2
Masih dengan Ini Karya Kita, sang CEO ingin melakukan tes keamanan pada folder sensitif Ini Karya Kita. Karena Teknologi Informasi merupakan departemen dengan salah satu fokus di Cyber Security, maka dia kembali meminta bantuan mahasiswa Teknologi Informasi angkatan 2023 untuk menguji dan mengatur keamanan pada folder sensitif tersebut. Untuk mendapatkan folder sensitif itu, mahasiswa IT 23 harus kembali mengunjungi website Ini Karya Kita pada www.inikaryakita.id/schedule . Silahkan isi semua formnya, tapi pada form subject isi dengan nama kelompok_SISOP24 , ex: IT01_SISOP24 . Lalu untuk form Masukkan Pesanmu, ketik “Mau Foldernya” . Tunggu hingga 1x24 jam, maka folder sensitif tersebut akan dikirimkan melalui email kalian. Apabila folder tidak dikirimkan ke email kalian, maka hubungi sang CEO untuk meminta bantuan.   
Pada folder "pesan" Adfi ingin meningkatkan kemampuan sistemnya dalam mengelola berkas-berkas teks dengan menggunakan fuse.
Jika sebuah file memiliki prefix "base64," maka sistem akan langsung mendekode isi file tersebut dengan algoritma Base64.
Jika sebuah file memiliki prefix "rot13," maka isi file tersebut akan langsung di-decode dengan algoritma ROT13.
Jika sebuah file memiliki prefix "hex," maka isi file tersebut akan langsung di-decode dari representasi heksadesimalnya.
Jika sebuah file memiliki prefix "rev," maka isi file tersebut akan langsung di-decode dengan cara membalikkan teksnya.
Contoh:
File yang belum didecode/ dienkripsi rot_13
![image](https://github.com/Ax3lrod/Sisop-4-2024-MH-IT17/assets/150204139/377f3ce0-b794-4153-a4dc-62794868202e)
File yang sudah didecode/ dienkripsi rot_13
![image](https://github.com/Ax3lrod/Sisop-4-2024-MH-IT17/assets/150204139/2fd59fef-2af8-40d9-960a-bfb1a9289169)


Pada folder “rahasia-berkas”, Adfi dan timnya memutuskan untuk menerapkan kebijakan khusus. Mereka ingin memastikan bahwa folder dengan prefix "rahasia" tidak dapat diakses tanpa izin khusus. 
Jika seseorang ingin mengakses folder dan file pada “rahasia”, mereka harus memasukkan sebuah password terlebih dahulu (password bebas). 
Setiap proses yang dilakukan akan tercatat pada logs-fuse.log dengan format :
[SUCCESS/FAILED]::dd/mm/yyyy-hh:mm:ss::[tag]::[information]
Ex:
[SUCCESS]::01/11/2023-10:43:43::[moveFile]::[File moved successfully]

**Solusi**
**Oleh Aryasatya Alaauddin (5027231082)**
 
Untuk mengerjakan soal ini pertama saya perlu mengisi formulir yang ada di website www.inikaryakita.id/schedule. Setelah mendapatkan email dari CEO, saya mengunduh file sensitif.zip dari link drive yang diberikan. Setelah itu sensitif.zip saya unzip dan meletakkan isinya di folder pengerjaan soal ini. Kemudian saya akan membuat program FUSE yang menggunakan direktori folder sensitif sebagai folder root dan program akan menciptakan folder baru yang akan digunakan sebagai mount. Cara kerja program fuse yang saya buat adalah:
1. User melakukan kompilasi dan run program di terminal 1.
2. Program akan membuat FUSE di path mount yang sudah ditetapkan.
3. Terminal 1 akan digunakan untuk mengeluarkan informasi mengenai operasi apa saja yang telah dijalankan.
4. Terminal 2 digunakan untuk menerima input operasi pada folder mount (cd, ls, cat).
5. Terminal 1 akan menanyakan password jika user ingin melakukan operasi pada file atau folder dengan prefix "rahasia"

### Fungsi Main
```
int main(int argc, char *argv[]) {

    root_path = realpath(argv[3], NULL);
    if (!root_path) {
        perror("Invalid root path");
        return 1;
    }

    ensure_log_file_exists();
    mkdir("/home/ax3lrod/sisop/modul4/praktikum/soal_2/sensitiffuse", 0755);

    int fuse_argc = argc - 1;
    char *fuse_argv[fuse_argc];
    fuse_argv[0] = argv[0];
    fuse_argv[1] = argv[1];
    fuse_argv[2] = argv[2];
    fuse_argv[3] = argv[3];

    return fuse_main(fuse_argc, fuse_argv, &gacor_oper, NULL);
}
```

DI fungsi main pertama program akan menerima command run dengan argumen sebanyak 4 (./pastibisa -f /home/ax3lrod/sisop/modul4/praktikum/soal_2/sensitiffuse /home/ax3lrod/sisop/modul4/praktikum/soal_2/sensitif). Command ini berisi argumen ./pastibisa untuk menjalankan program, -f untuk menjalankan program di foreground, dan dilanjutkan dengan path ke direktori folde mount dan path ke direktori root. Fungsi main juga akan memeriksa apakah argumen ke 4 merupakan path ke direktori root yang valid, memanggil fungsi untuk membuat file log, dan membuat direktori mount.

```
static struct fuse_operations gacor_oper = {
    .getattr    = gacor_getattr,
    .readdir    = gacor_readdir,
    .open       = gacor_open,
    .read       = gacor_read,
    .access     = gacor_access,
};
```

Setelah code dicompile dan dijalankan, program fuse akan menggunakan fungsi-fungsi seperti gacor_getattr dan gacor_readdir untuk melakukan operasi pada FUSE sesuai command yang diberikan oleh user pada folder mount.

### Decode File

Setelah program dijalankan, program akan melakukan decode ke semua file yang memiliki string base64, rot13, hex, atau rev di nama atau fullpathnya menggunakan algoritmanya masing-masing.

```
char *decode_base64(const char *input) {
    size_t length = strlen(input);
    char *decoded = malloc(length + 1);
    if (!decoded) return NULL;

    int decode_length = 0;
    for (size_t i = 0; i < length; i += 4) {
        unsigned int buf = 0;
        for (int j = 0; j < 4; j++) {
            buf <<= 6;
            if (input[i + j] >= 'A' && input[i + j] <= 'Z')
                buf |= input[i + j] - 'A';
            else if (input[i + j] >= 'a' && input[i + j] <= 'z')
                buf |= input[i + j] - 'a' + 26;
            else if (input[i + j] >= '0' && input[i + j] <= '9')
                buf |= input[i + j] - '0' + 52;
            else if (input[i + j] == '+')
                buf |= 62;
            else if (input[i + j] == '/')
                buf |= 63;
        }
        decoded[decode_length++] = (buf >> 16) & 0xFF;
        decoded[decode_length++] = (buf >> 8) & 0xFF;
        decoded[decode_length++] = buf & 0xFF;
    }
    decoded[decode_length] = '\0';
    return decoded;
}

char *decode_rot13(const char *input) {
    char *decoded = strdup(input);
    if (!decoded) return NULL;

    for (char *p = decoded; *p; p++) {
        if ((*p >= 'A' && *p <= 'M') || (*p >= 'a' && *p <= 'm')) {
            *p += 13;
        } else if ((*p >= 'N' && *p <= 'Z') || (*p >= 'n' && *p <= 'z')) {
            *p -= 13;
        }
    }
    return decoded;
}

char *decode_hex(const char *input) {
    size_t length = strlen(input);
    char *decoded = malloc(length / 2 + 1);
    if (!decoded) return NULL;

    for (size_t i = 0; i < length; i += 2) {
        sscanf(&input[i], "%2hhx", &decoded[i / 2]);
    }
    decoded[length / 2] = '\0';
    return decoded;
}

char *decode_rev(const char *input) {
    size_t length = strlen(input);
    char *decoded = malloc(length + 1);
    if (!decoded) return NULL;

    for (size_t i = 0; i < length; i++) {
        decoded[i] = input[length - i - 1];
    }
    decoded[length] = '\0';
    return decoded;
}

char *decode_content(const char *path, const char *content) {
    if (strstr(path, "base64")) {
        return decode_base64(content);
    } else if (strstr(path, "rot13")) {
        return decode_rot13(content);
    } else if (strstr(path, "hex")) {
        return decode_hex(content);
    } else if (strstr(path, "rev")) {
        return decode_rev(content);
    } else {
        return strdup(content);
    }
}
```
### Akses ke folder dengan prefix rahasia

Program juga perlu menanyakan password setiap kali user ingin melakukan operasi pada file atau folder dengan prefix "rahasia" pada nama atau fullpathnya. Untuk mengimplementasikannya saya akan menggunakan fungsi prompt_for_password untuk menanyakan input password dan fungsi authenticate untuk memeriksa validitas dari password yang sudah di inputkan.

```
void authenticate(const char *password) {
    if (strcmp(password, PASSWORD) == 0) {
        secret_authenticated = 1;
        log_event("SUCCESS", "authenticate", "User authenticated successfully");
    } else {
        log_event("FAILED", "authenticate", "User authentication failed");
    }
}

void prompt_for_password() {
    char password[MAX_PASSWORD_LENGTH];
    printf("Enter password to access secret files: ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        perror("Error reading password");
        return;
    }
    password[strcspn(password, "\n")] = '\0'; // Remove trailing newline character
    authenticate(password);
}
```
Kemudian fungsi prompt_for_password akan dipanggil pada fungsi-fungsi operasi FUSE seperti berikut:

```
static int gacor_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    DIR *dp;
    struct dirent *de;
    char fpath[PATH_MAX];
    snprintf(fpath, sizeof(fpath), "%s%s", root_path, path);

    if (strstr(path, "rahasia-berkas") != NULL){
        prompt_for_password();
        if (!secret_authenticated) {
            log_event("FAILED", "accessDenied", path);
            return -EACCES;
        }else{
            printf("Fuse: Authenticated\n");
            chmod(fpath, 0755);
        }
    }

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    printf("Fuse: Directory read\n");
    log_event("SUCCESS", "dirRead", path);

    closedir(dp);
    return 0;
}

static int gacor_open(const char *path, struct fuse_file_info *fi) {
    int res;
    char fpath[PATH_MAX];
    snprintf(fpath, sizeof(fpath), "%s%s", root_path, path);

    if (strstr(path, "rahasia-berkas") != NULL){
        prompt_for_password();
        if (!secret_authenticated) {
            log_event("FAILED", "accessDenied", path);
            return -EACCES;
        }else{
            printf("Fuse: Authenticated\n");
            chmod(fpath, 0755);
        }
    }

    res = open(fpath, fi->flags);
    if (res == -1)
        return -errno;

    close(res);

    printf("Fuse: File opened\n");
    log_event("SUCCESS", "fileOpen", path);
    return 0;
}

static int gacor_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    int fd;
    int res;
    char fpath[PATH_MAX];
    snprintf(fpath, sizeof(fpath), "%s%s", root_path, path);

    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);

    if (strncmp(path, "/rahasia-berkas", 15) == 0 && !secret_authenticated) {
        prompt_for_password();
        if (!secret_authenticated) {
            log_event("FAILED", "accessDenied", path);
            return -EACCES;
        }else{
            chmod(fpath, 0755);
        }
    }

    char *decoded_content = decode_content(path, buf);
    if (!decoded_content) {
        return -ENOMEM;
    }

    strncpy(buf, decoded_content, size);
    free(decoded_content);

    printf("Fuse: File read\n");
    log_event("SUCCESS", "fileRead", path);
    return res;
}
static int gacor_access(const char *path, int mask) {
    char fpath[PATH_MAX];
    snprintf(fpath, sizeof(fpath), "%s%s", root_path, path);
    secret_authenticated = 0;

    if (strncmp(path, "/rahasia-berkas", 15) == 0 && !secret_authenticated) {
        prompt_for_password();
        if (!secret_authenticated) {
            log_event("FAILED", "accessDenied", path);
            return -EACCES;
        }else{
            printf("Fuse: Authenticated\n");
            secret_accessed = 1;
            chmod(fpath, 0755);
        }
    }

    int res = access(fpath, mask);
    if (res == -1)
        return -errno;
    
    printf("Fuse: Access granted\n");
    log_event("SUCCESS", "accessGranted", path);

    return 0;
}

```
Program akan memeriksa apakah filepath ada di folder rahasia-berkas dan apakah flag secret_authenticated bernilai 0. Jika iya, program akan memanggil fungsi prompt_for_password. Jika password benar maka nilai flag secret_authenticated akan berubah menjadi 1 (true) dan akan merubah mode permission dari file atau folder menjadi 755 (read/write/execute). Jika password salah maka program akan mengembalikan output "access denied" dan permission tidak akan berubah.

### Logging Proses

Setiap operasi yang terjadi di FUSE akan dicatat di file logs-fuse.log menggunakan 2 fungsi berikut.

```
void ensure_log_file_exists() {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Unable to create log file");
        exit(EXIT_FAILURE);
    }
    fclose(log_file);
}

void log_event(const char *status, const char *tag, const char *information) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Unable to open log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf) - 1, "%d/%m/%Y-%H:%M:%S", t);

    fprintf(log_file, "[%s]::%s::[%s]::[%s]\n", status, time_buf, tag, information);
    fclose(log_file);
}
```
Fungsi ensure_log_file_exists berfungsi untuk membuat file log jika belum ada. Fungsi ini dipanggil di fungsi main.
Fungsi log_event adalah fungsi yang menulis log operasi ke dalam file log yang telah dibuat. Fungsi ini dipanggil di semua fungsi operasi FUSE.

### Sample Output

Untuk memnjalankan program perlu dilakukan compile di terminal 1 dengan command berikut:
gcc -o pastibisa pastibisa.c `pkg-config fuse --cflags --libs` -D_FILE_OFFSET_BITS=64
Kemudian untuk menjalankan code, gunakan command berikut:
./pastibisa -f [full path ke folder mount] [full path ke folder root]
Untuk melakukan tes saya menggunakan command ./pastibisa -f /home/ax3lrod/sisop/modul4/praktikum/soal_2/sensitiffuse /home/ax3lrod/sisop/modul4/praktikum/soal_2/sensitif

![image](https://github.com/Ax3lrod/Sisop-4-2024-MH-IT17/assets/150204139/490c4bac-902b-4477-a87e-6f9ebf0a655b)

Setelah ini, user dapat menggunakan terminal 2 untuk pindah ke folder mount (dalam kasus saya folder mount bernama "sensitiffuse"). Setelah masuk ke folder mount, user dapat menjalankan command "cat pesan/*" untuk menampilkan isi folder pesan dengan file yang sudah didecode.

![image](https://github.com/Ax3lrod/Sisop-4-2024-MH-IT17/assets/150204139/f147431a-8024-497d-af6d-de925bf755d4)




## NOMOR 3
