#include <stdio.h>
#include <stdlib.h>

//Orden alfabetico


static int fisop_access (const char * path_archivo, int numero){
    printf("[debug] fisop_access\n");
    return 0;
}

static int fisop_chmod (const char * path_archivo, mode_t modo, struct fuse_file_info *fi){
    printf("[debug] fisop_chmod\n");
    return 0;
}

static int fisop_chown (const char * path_archivo, uid_t uid, gid_t gid, struct fuse_file_info *fi){
    printf("[debug] fisop_chown\n");
    return 0;
}

static int fisop_link(const char * path_archivo, const char * link){
    printf("[debug] fisop_link\n");
    return 0;
}

static int fisop_lock (const char *path_archivo, struct fuse_file_info *fi, int cmd, struct flock *flock){
    printf("[debug] fisop_lock\n");
    return 0;
}

static int fisop_mknod(const char * path, mode_t mode , dev_t dev){
    printf("[debug] fisop_mknod\n");
    return 0;
}

static int fisop_open (const char * path_archivo, struct fuse_file_info * fi){
    printf("[debug] fisop_open\n");
    return 0;
}

static int fisop_opendir (const char *path, struct fuse_file_info *fi){
    printf("[debug] fisop_opendir path: %s\n",path);
    return 0;
}

static int fisop_readlink(const char * link, char * buffer, size_t size){
    printf("[debug] fisop_readlink\n");
    return 0;
}

static int fisop_releasedir (const char *path, struct fuse_file_info *fi){
    printf("[debug] fisop_releasedir\n");
    return 0;
}

static int fisop_rename(const char *path_archivo, const char *nuevo_nombre, unsigned int flags){
    printf("[debug] fisop_rename\n");
    return 0;
}

static int fisop_symlink(const char * nombre, const char * nosequees){
    printf("[debug] fisop_symlink\n");
    return 0;
}

static int
fisop_stats(const char *path, struct statvfs *stvfs)
{
    printf("[debug] fisop_stats(%s)\n", path);
    return 0;
}

//static int fisop_utimens (const char *path_archivo, const struct timespec tv[2], struct fuse_file_info *fi){
    //printf("[debug] fisop_utimens\n");
//    return 0;
//}

//------------------------------------------------



static int fisop_release (const char *path, struct fuse_file_info *fi){
    printf("[debug] fisop_release\n===============\n\n");
    return 0;
}

static int fisop_fsync (const char *path , int numero, struct fuse_file_info *fi){
    printf("[debug] fisop_fsync\n");
    return 0;
}

static int fisop_setxattr (const char *nombre, const char *otronombre, const char *anothername, size_t size, int numero){
    printf("[debug] fisop_setxattr\n");
    return 0;
}

static int fisop_getxattr (const char *nombre, const char *otronombre, char *anothername, size_t size){
    printf("[debug] fisop_getxattr\n");
    return 0;
}

static int fisop_listxattr (const char *nombre, char *otronombre, size_t size){
    printf("[debug] fisop_listxattr\n");
    return 0;
}

static int fisop_removexattr (const char *nombre, const char *otronombre){
    printf("[debug] fisop_removexattr\n");
    return 0;
}

static int fisop_fsyncdir (const char *path, int numero, struct fuse_file_info *fi){
    printf("[debug] fisop_fsync\n");
    return 0;
}

static int fisop_bmap (const char *nombre, size_t blocksize, uint64_t *idx){
    printf("[debug] fisop_bmap\n");
    return 0;
}

static int fisop_ioctl (const char *nombre, unsigned int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags, void *data){
    printf("[debug] fisop_ioctl\n");
    return 0;
}

static int fisop_poll (const char *nombre, struct fuse_file_info *fi, struct fuse_pollhandle *ph, unsigned *reventsp){
    printf("[debug] fisop_poll\n");
    return 0;
}

static int fisop_write_buf (const char *path_archivo, struct fuse_bufvec *buf, off_t off, struct fuse_file_info *fi){
    printf("[debug] fisop_write_buf\n");
    return 0;
}

static int fisop_read_buf (const char *path_archivo, struct fuse_bufvec **bufp, size_t size, off_t off, struct fuse_file_info *fi){
    printf("[debug] fisop_read_buf\n");
    return 0;
}

static int fisop_flock (const char *path_archivo, struct fuse_file_info *fi, int op){
    printf("[debug] fisop_flock\n");
    return 0;
}

static int fisop_fallocate (const char *path_archivo, int numero, off_t offset1, off_t offset2, struct fuse_file_info *fi){
    printf("[debug] fisop_fallocate\n");
    return 0;
}
/*
static ssize_t fisop_copy_file_range (const char *path_in, struct fuse_file_info *fi_in, off_t offset_in, const char *path_out, struct fuse_file_info *fi_out, off_t offset_out, size_t size, int flags){
    printf("[debug] fisop_copy_file_range\n");
    return 0;
}

static off_t fisop_lseek (const char *nombre, off_t off, int whence, struct fuse_file_info *fi_out){
    printf("[debug] fisop_lseek\n");
    return 0;
}*/