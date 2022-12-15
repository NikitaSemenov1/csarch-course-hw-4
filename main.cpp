#include <string>
#include <iostream>
#include <filesystem>

[[noreturn]] void start(int, int, int, int);


int main(int argc, char* argv[]) {

    srand(time(nullptr));

    if (argc < 2) {
        std::cerr << "Input/output parameter is not provided" << std::endl;
        return 1;
    }

    int hotel_size, threads_number, wait_range, duration_range;

    if (argv[1] == std::string("-f")) {
        // input parameters from file
        // log to file

        if (argc < 4) {
            std::cerr << "Input/output files are not provided" << std::endl;
        }

        if (not freopen(argv[2], "r", stdin)) {
            std::cerr << "Can't open input file" << std::endl;
            return 1;
        }

        if (not freopen(argv[3], "w", stdout)) {
            std::cerr << "Can't open output file" << std::endl;
            return 1;
        }

        std::cin >> hotel_size >> threads_number >> wait_range >> duration_range;

    } else if (argv[1] == std::string("-i")) {
        // input parameters from console
        // log to console

        std::cout << """Enter four numbers:\n"
                     "1. The size of the hotel\n"
                     "2. The number of threads\n"
                     "3. Randomization range of the waiting time between new guests for each thread\n"
                     "4. Randomization range of the duration time""" << std::endl;

        std::cin >> hotel_size >> threads_number >> wait_range >> duration_range;

    } else if (argv[1] == std::string("-r")) {
        // generate parameters
        // log to console

        hotel_size = rand() % 100 + 1;
        threads_number = rand() % 7 + 2;
        wait_range = rand() % 5 + 1;
        duration_range = rand() % 20 + 1;
        std::cout << "Generated:"
                     "\nhotel_size: " << hotel_size <<
                     "\nthreads_number: " << threads_number <<
                     "\nwait_range: " << wait_range <<
                     "\nduration_range: " << duration_range << std::endl;
    } else {
        std::cerr << "Invalid argument" << std::endl;
        return 1;
    }

    start(hotel_size, threads_number, wait_range, duration_range);
}
