// Code written by Daniel Ehrenberg, released into the public domain

#include <fcntl.h>
#include <cassert>
#include <iostream>
#include <string>
#include <libaio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <thread>
#include <chrono>         // std::chrono::seconds


class Package {
public:
    Package() {
        num = -1;
    }

    Package(int i) {
        num = i;
    }

    void complete() {
        std::cout << "package processed. num: " << num << std::endl;
    }

    int num;
};

int do_reap(io_context_t ctx_id, long max_events) {
    struct io_event* events = new io_event[max_events];
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 100000000;
    int num_events = io_getevents(ctx_id, 1, max_events, events, &timeout);
    std::cout << "io_getevents got " << num_events << " events" << std::endl;
    for (int i = 0; i < num_events; i++) {
        struct io_event event = events[i];
        Package* pkg = static_cast<Package*>(event.data);
        pkg->complete();
        delete pkg;
    }
    delete events;
    return num_events;
}

void fifo_write(int msg_cnt) {
    std::string fifo_file = "/tmp/myfile";
    int fd = open(fifo_file.c_str(), O_WRONLY | O_CREAT, 0644);
    perror("open file for writing");
    assert(fd >= 0);
    fallocate(fd, 0, 0, msg_cnt * sizeof(int));

    // async io
    io_context_t ioctx(0);
    int setup_res = io_setup(100, &ioctx);
    std::cout << "io_setup returns " << setup_res << std::endl;
    assert(setup_res >= 0);

    for(int i = 0; i < msg_cnt; i++) {
        std::cout << "submit a write" << std::endl;
        Package *pkg = new Package(i);
        struct iocb iocb;
        io_prep_pwrite(&iocb, fd, &(pkg->num), sizeof(pkg->num), i * 4);
        iocb.data = pkg;
        struct iocb *iocb_ptrs[1] = { &iocb };
        int res = io_submit(ioctx, 1, iocb_ptrs);
        assert(res == 1);
    }
    do_reap(ioctx, 1000);

    close(fd);
}

void fifo_read(int msg_cnt) {
    std::string fifo_file = "/tmp/myfile";
    int fd = open(fifo_file.c_str(), O_RDONLY, 0644);
    perror("open file for reading");
    assert(fd >= 0);

    // async io
    io_context_t ioctx(0);
    int setup_res = io_setup(100, &ioctx);
    std::cout << "io_setup returns " << setup_res << std::endl;
    assert(setup_res >= 0);

    for(int i = 0; i < msg_cnt; i++) {
        std::cout << "submit a read" << std::endl;
        Package *pkg = new Package();
        struct iocb iocb;
        io_prep_pread(&iocb, fd, &(pkg->num), sizeof(pkg->num), i * 4);
        iocb.data = pkg;
        struct iocb *iocb_ptrs[1] = { &iocb };
        int res = io_submit(ioctx, 1, iocb_ptrs);
        assert(res == 1);
    }
    do_reap(ioctx, 1000);

    close(fd);
}


int main(int argc, char *argv[]) {
    fifo_write(10);
    fifo_read(10);

    return 0;
}