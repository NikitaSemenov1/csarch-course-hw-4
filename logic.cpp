#include <iostream>
#include <random>
#include <unordered_map>

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// parameters
namespace {
    int wait_range;
    int duration_range;
    int threads_number;
    int hotel_size;
}

// parallel primitives
namespace {
    sem_t empty;

    pthread_mutex_t print_mutex, guest_number_mutex, suites_mutex;
}

// tools
namespace {
    inline void wait() {
        sleep(std::rand() % wait_range + 1);
    }

    template<typename ...ArgsT>
    void log(int thread_number, const char *fmt, ArgsT ...args) {
        pthread_mutex_lock(&print_mutex);
        printf("[Thread %d]  ", thread_number);
        printf(fmt, args...);
        pthread_mutex_unlock(&print_mutex);
    }
}

// guest logic
namespace {
    struct Guest {
        unsigned int duration;
        unsigned int number;
    };

    unsigned int guest_number;  // to set a number to new guest

    // generates guest and allocate it in the heap
    Guest *generate_guest() {
        auto *guest = new Guest;
        guest->duration = rand() % duration_range + 1;  // Amount living time in seconds

        pthread_mutex_lock(&guest_number_mutex);
        guest->number = ++guest_number;  // the number of the guest
        pthread_mutex_unlock(&guest_number_mutex);

        return guest;
    }
}

// suites logic
namespace {
    std::unordered_map<Guest *, time_t> suites;  // implements set of suites taken by guests. Contains pointer to guest structure and time of living start

    void add_guest(Guest *guest) {
        suites[guest] = time(nullptr);
    }

    void remove_guest(Guest *guest) {
        suites.erase(guest);
    }
}

namespace {

    void checkin(Guest *guest, int thread_number) {

        // addressing to mutual suites,
        pthread_mutex_lock(&suites_mutex);
        log(thread_number,
            "Guest %d has come to the hotel to live for %d seconds. There are %d guests in the hotel\n",
            guest->number, guest->duration, suites.size());
        pthread_mutex_unlock(&suites_mutex);

        sem_wait(&empty);  // "decrease semaphore value" or wait for a sem_post

        // addressing to mutual suites in add_guest function
        pthread_mutex_lock(&suites_mutex);

        add_guest(guest);
        log(thread_number,"Guest %d has checked into a room\n",  guest->number);

        pthread_mutex_unlock(&suites_mutex);
    }

    void checkout(Guest *guest, int thread_number) {

        // addressing to mutual suites in remove_guest function
        pthread_mutex_lock(&suites_mutex);

        remove_guest(guest);

        log(thread_number,"Guest %d has left the hotel\n", guest->number);
        sem_post(&empty);

        pthread_mutex_unlock(&suites_mutex);
    }

    [[noreturn]] void *checkiner(void *arg) {
        int thread_number = *(int *) arg;

        while (true) {
            Guest *guest = generate_guest();
            checkin(guest, thread_number);
            wait();
        }
    }

    // finds guests whose living time has expired and remove them from their suites.
    void check_duration(int thread_number) {
        for (auto it = suites.begin(); it != suites.end();) {
            auto prev = it;
            ++it;
            if (prev->second + prev->first->duration <= time(nullptr)) {
                Guest *guest = prev->first;
                checkout(guest, thread_number);
                delete guest;
            }
        }
    }

    [[noreturn]] void *checkouter(void *arg) {
        int thread_number = *(int *) arg;

        while (true) {
            check_duration(thread_number);
        }
    }
}

[[noreturn]] void start(int hotel_size_, int threads_number_, int wait_range_, int duration_range_) {

    // parameters
    threads_number = threads_number_;  // the number of threads to be created
    hotel_size = hotel_size_;  // the size of the hotel (maximal amount of guests that can be in the hotel at the same time)
    wait_range = wait_range_;  // Randomization range of the waiting time between new guests for each thread
    duration_range = duration_range_;  // Randomization range of the duration time

    pthread_t threads[threads_number];  // (VLA array)
    int thread_numbers[threads_number];  // (VLA array)

    // to pass number of a thread to a concurrent function using pthread_create
    for (int i = 0; i < threads_number; i++) {
        thread_numbers[i] = i;
    }

    sem_init(&empty, 0, hotel_size);  // intended to simulate occupancy of the hotel

    pthread_mutex_init(&print_mutex, nullptr);  // enforces limit on access to log
    pthread_mutex_init(&guest_number_mutex, nullptr);  // enforces limit on access to guest_number variable
    pthread_mutex_init(&suites_mutex, nullptr);  // enforces limit on access to suites

    // checkouter thread
    pthread_create(&threads[0], nullptr, checkouter, &thread_numbers[0]);

    // checkiner threads
    for (int i = 1; i < threads_number; i++) {
        pthread_create(&threads[i], nullptr, checkiner, &thread_numbers[i]);
    }

    // to avoid program finishing. It implies that threads run infinitely
    while(true);
}
