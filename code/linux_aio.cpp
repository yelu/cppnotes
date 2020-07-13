// Code written by Daniel Ehrenberg, released into the public domain

#include <fcntl.h>
#include <cassert>
#include <iostream>
#include <string>
#include <libaio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

// Path to the file to manipulate
std::string FILE_PATH = "/tmp/testfile";
// Length of file in 4k blocks
const int FILE_SIZE = 1000;
// Number of concurrent requests
const int CONCURRENT_REQUESTS = 100;
// min_nr
const int MIN_NR = 1;
// max_nr
const int MAX_NR = 1;
// The size of operation that will occur on the device
static const int kPageSize = 4096;

class AIORequest {
public:
    int *buffer_;

    virtual void Complete(int res) = 0;

    AIORequest() {
        int ret = posix_memalign(reinterpret_cast<void **>(&buffer_),
                                 kPageSize, kPageSize);
        assert(ret == 0);
    }

    virtual ~AIORequest() {
        free(buffer_);
    }
};

class Adder {
public:
    virtual void Add(int amount) = 0;

    virtual ~Adder() {};
};

class AIOReadRequest : public AIORequest {
private:
    Adder *adder_;

public:
    AIOReadRequest(Adder *adder) : AIORequest(), adder_(adder) {}

    virtual void Complete(int res) {
        assert(res == kPageSize && "Read incomplete or error");
        int value = buffer_[0];
        printf("Read of %d completed", value);
        adder_->Add(value);
    }
};

class AIOWriteRequest : public AIORequest {
private:
    int value_;

public:
    AIOWriteRequest(int value) : AIORequest(), value_(value) {
        buffer_[0] = value;
    }

    virtual void Complete(int res) {
        assert(res == kPageSize && "Write incomplete or error");
        printf("Write of %d completed", value_);
    }
};

class AIOAdder : public Adder {
public:
    int fd_;
    io_context_t ioctx_;
    int counter_;
    int reap_counter_;
    int sum_;
    int length_;

    AIOAdder(int length)
            : ioctx_(0), counter_(0), reap_counter_(0), sum_(0), length_(length) {}

    void Init() {
        std::cout << "Opening file" << std::endl;
        fd_ = open(FILE_PATH.c_str(), O_RDWR | O_DIRECT | O_CREAT, 0644);
        assert(fd_ >= 0 && "Error opening file");
        std::cout << "Allocating enough space for the sum" << std::endl;
        assert(fallocate(fd_, 0, 0, kPageSize * length_) >= 0 && "Error in fallocate");
        printf("Setting up the io context");
        assert(io_setup(100, &ioctx_) >= 0 && "Error in io_setup");
    }

    virtual void Add(int amount) {
        sum_ += amount;
        std::cout << "Adding " << amount << " for a total of " << sum_ << std::endl;
    }

    void SubmitWrite() {
        std::cout << "Submitting a write to " << counter_ << std::endl;
        struct iocb iocb;
        struct iocb *iocbs = &iocb;
        AIORequest *req = new AIOWriteRequest(counter_);
        io_prep_pwrite(&iocb, fd_, req->buffer_, kPageSize, counter_ * kPageSize);
        iocb.data = req;
        int res = io_submit(ioctx_, 1, &iocbs);
        assert(res == 1);
    }

    void WriteFile() {
        reap_counter_ = 0;
        for (counter_ = 0; counter_ < length_; counter_++) {
            SubmitWrite();
            Reap();
        }
        ReapRemaining();
    }

    void SubmitRead() {
        std::cout << "Submitting a read from " << counter_ << std::endl;
        struct iocb iocb;
        struct iocb *iocbs = &iocb;
        AIORequest *req = new AIOReadRequest(this);
        io_prep_pread(&iocb, fd_, req->buffer_, kPageSize, counter_ * kPageSize);
        iocb.data = req;
        int res = io_submit(ioctx_, 1, &iocbs);
        assert(res == 1);
    }

    void ReadFile() {
        reap_counter_ = 0;
        for (counter_ = 0; counter_ < length_; counter_++) {
            SubmitRead();
            Reap();
        }
        ReapRemaining();
    }

    int DoReap(int min_nr) {
        std::cout << "Reaping between " << min_nr << " and "
                  << MAX_NR << " io_events" << std::endl;
        struct io_event *events = new io_event[MAX_NR];
        struct timespec timeout;
        timeout.tv_sec = 0;
        timeout.tv_nsec = 100000000;
        int num_events;
        std::cout << "Calling io_getevents" << std::endl;
        num_events = io_getevents(ioctx_, min_nr, MAX_NR, events,
                                  &timeout);
        std::cout << "Calling completion function on results" << std::endl;
        for (int i = 0; i < num_events; i++) {
            struct io_event event = events[i];
            AIORequest *req = static_cast<AIORequest *>(event.data);
            req->Complete(event.res);
            delete req;
        }
        delete events;

        std::cout << "Reaped " << num_events << " io_events" << std::endl;
        reap_counter_ += num_events;
        return num_events;
    }

    void Reap() {
        if (counter_ >= MIN_NR) {
            DoReap(MIN_NR);
        }
    }

    void ReapRemaining() {
        while (reap_counter_ < length_) {
            DoReap(1);
        }
    }

    ~AIOAdder() {
        std::cout << "Closing AIO context and file" << std::endl;
        io_destroy(ioctx_);
        close(fd_);
    }

    int Sum() {
        std::cout << "Writing consecutive integers to file" << std::endl;
        WriteFile();
        std::cout << "Reading consecutive integers from file" << std::endl;
        ReadFile();
        return sum_;
    }
};

#include <vector>

class WriteRequest {

public:
    WriteRequest(const char* buf, int size) : buffer(buf, buf + size)
    {}

    void Complete()
    {
        std::cout << "write complete." << std::endl;
    }

    std::vector<char> buffer;
};



int main(int argc, char *argv[]) {
    int file_size = 1000;
    AIOAdder adder(file_size);
    adder.Init();
    int sum = adder.Sum();
    int expected = (file_size * (file_size - 1)) / 2;
    std::cout << "AIO is complete" << std::endl;
    assert(sum == expected);
    std::cout << "Expected " << expected << " Got " << sum << std::endl;
    printf("Successfully calculated that the sum of integers from 0"
           " to %d is %d\n", file_size - 1, sum);



    int fd = open(FILE_PATH.c_str(), O_RDWR | O_DIRECT | O_CREAT, 0644);
    assert(fd >= 0 && "error opening file");
    io_context_t ioctx;
    int setup_res = io_setup(100, &ioctx);
    assert(setup_res >= 0 && "error in io_setup");

    std::cout << "submit a write" << std::endl;
    struct iocb iocb;
    struct iocb *iocbs = &iocb;
    char* buffer = "hello";
    WriteRequest* req = new WriteRequest(buffer, strlen(buffer));
    io_prep_pwrite(&iocb, fd, req->buffer.data(), req->buffer.size(), 0);
    iocb.data = req;
    int res = io_submit(ioctx, 1, &iocb);
    assert(res == 1);






    return 0;
}