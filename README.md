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

## NOMOR 3
