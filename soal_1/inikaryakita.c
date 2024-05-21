#define FUSE_USE_VERSION 29
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

static const char *root_path = "/home/ubuntu/portofolio";

static int fs_getattr(const char *path, struct stat *stbuf) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = lstat(fullpath, stbuf);
    if (res == -1)
        return -errno;
    return 0;
}

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

static int fs_mkdir(const char *path, mode_t mode) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = mkdir(fullpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}

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

static int fs_chmod(const char *path, mode_t mode) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = chmod(fullpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}

static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int fd = creat(fullpath, mode);
    if (fd == -1)
        return -errno;
    fi->fh = fd;
    return 0;
}

static int fs_unlink(const char *path) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s%s", root_path, path);

    int res = unlink(fullpath);
    if (res == -1)
        return -errno;
    return 0;
}

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

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &fs_operations, NULL);
}
