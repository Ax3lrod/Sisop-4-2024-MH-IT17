#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>  

static const char *base_path = "relics";

static int archeology_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    char fpath[1000];
    sprintf(fpath, "%s%s", base_path, path);

    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int archeology_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    dp = opendir(base_path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_DIR) continue;
        if (strstr(de->d_name, ".000") == NULL) continue;

        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        filler(buf, de->d_name, &st, 0);
    }

    closedir(dp);
    return 0;
}

static int archeology_open(const char *path, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", base_path, path);
    int res = open(fpath, fi->flags);
    if (res == -1)
        return -errno;

    close(res);
    return 0;
}

static int archeology_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    char fpath[1000];

    sprintf(fpath, "%s%s", base_path, path);
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static struct fuse_operations archeology_oper = {
    .getattr = archeology_getattr,
    .readdir = archeology_readdir,
    .open = archeology_open,
    .read = archeology_read,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &archeology_oper, NULL);
}
