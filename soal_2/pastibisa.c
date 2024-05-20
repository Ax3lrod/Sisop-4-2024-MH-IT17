#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>

#define LOG_FILE "/home/ax3lrod/sisop/modul4/praktikum/soal_2/logs-fuse.log"
#define PASSWORD "177013"
#define MAX_PASSWORD_LENGTH 256

static const char *root_path = NULL;
static int secret_authenticated = 0;
static int secret_accessed = 0;

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


static int gacor_getattr(const char *path, struct stat *stbuf) {
    int res;
    char fpath[PATH_MAX];
    snprintf(fpath, sizeof(fpath), "%s%s", root_path, path);

    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;
    
    printf("Fuse: File attributes read\n");
    log_event("SUCCESS", "fileAttrRead", path);

    return 0;
}

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

static struct fuse_operations gacor_oper = {
    .getattr    = gacor_getattr,
    .readdir    = gacor_readdir,
    .open       = gacor_open,
    .read       = gacor_read,
    .access     = gacor_access,
};

int main(int argc, char *argv[]) {

    root_path = realpath(argv[3], NULL);
    if (!root_path) {
        perror("Invalid root path");
        return 1;
    }

    ensure_log_file_exists();
    mkdir("/home/ax3lrod/sisop/modul4/praktikum/soal_2/sensitiffuse", 0755);

    // Modify arguments for FUSE
    int fuse_argc = argc - 1;
    char *fuse_argv[fuse_argc];
    fuse_argv[0] = argv[0];
    fuse_argv[1] = argv[1];
    fuse_argv[2] = argv[2];
    fuse_argv[3] = argv[3];

    return fuse_main(fuse_argc, fuse_argv, &gacor_oper, NULL);
}
