#include <cerrno>
#include <cstdlib>
#include <exception>
#include <forward_list>
#include <string>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include "utils.hpp"

using path_list = std::forward_list<std::string>;

std::ostream&
operator<<(std::ostream& os, const path_list& list)
{
    if (list.empty()) {
        os << '/';
    } else {
       for (const auto& elem : list)
            os << '/' << elem;
    }
    return os;
}

path_list*
get_cwd()
{
    struct stat statbuf;
    const struct dirent* ent;
    path_list* list;
    DIR* dir;
    dev_t dev;
    ino_t ino;
    int fd, cfd;
    bool found_all;

    try {
        list = new path_list;
    } catch (const std::exception& ex) {
        exit_errx("new: ", ex.what());
    }
    cfd = AT_FDCWD;
    for (found_all = false; !found_all;) {
        fd = openat(cfd, "..", O_RDONLY);
        if (fd < 0)
            exit_err("openat()");
        dir = fdopendir(fd);
        if (dir == NULL)
            exit_err("fdopendir()");
        if (fstatat(cfd, ".", &statbuf, 0) < 0)
            exit_err("fstatat()");
        dev = statbuf.st_dev;
        ino = statbuf.st_ino;
        for (;;) {
            errno = 0;
            ent = readdir(dir);
            if (ent == NULL) {
                if (errno != 0)
                    exit_err("readdir()");
                break;
            }
            if (fstatat(fd, ent->d_name, &statbuf, AT_SYMLINK_NOFOLLOW) < 0) {
                if (errno == ENOENT)
                    continue;
                exit_err("fstatat()");
            }
            if (S_ISDIR(statbuf.st_mode) &&
                statbuf.st_dev == dev && statbuf.st_ino == ino
            ) {
                try {
                    list->push_front(ent->d_name);
                } catch (const std::exception& ex) {
                    exit_errx("std::forward_list::push_front(): ", ex.what());
                }
                break;
            }
        }
        if (closedir(dir) < 0)
            exit_err("closedir()");
        if (ent == NULL)
            exit_errx("some component in CWD was removed");
        fd = cfd;
        if (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' ||
            (ent->d_name[1] == '.' && ent->d_name[2] == '\0'))
        ) {
            found_all = true;
            list->pop_front();
        } else {
            cfd = openat(cfd, "..", O_RDONLY);
            if (cfd < 0)
                exit_err("openat()");
        }
        if (fd != AT_FDCWD && close(fd) < 0)
            exit_err("close()");
    }
    return list;
}

int
main(int argc, char *argv[])
{
    const char* root_dir = nullptr;
    const char* work_dir = nullptr;
    int opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, "r:w:")) != -1)
        switch (opt) {
        case 'r':
            root_dir = optarg;
            break;
        case 'w':
            work_dir = optarg;
            break;
        default:
            exit_errx("wrong option -", static_cast<char>(optopt));
        }
    if (work_dir != nullptr && chdir(work_dir) < 0)
        exit_err("chdir()");
    if (root_dir != nullptr && chroot(root_dir) < 0)
        exit_err("chroot()");
    auto list = get_cwd();
    std::cout << *list << '\n';
}






















