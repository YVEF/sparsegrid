#include <cstring>
#include <iostream>
#include "runner.h"



static void movegen_job(unsigned level) {
    for(unsigned i=1; i<=level; i++) {
        perftGen(i);
        std::cout.flush();
    }
}


int main(int argc, char** argv) {
    unsigned level = 0, iterations = 0;
    bool is_movegen = false, is_eval = false;
    for(int i=1; i<argc; i++) {
        if(std::strcmp("--help", argv[i]) == 0) {
            // show help
            std::cout 
                    << "Help:\n"
                    << "level           Recursion level (movegen only)\n"
                    << "job             Type of job: movegen, eval\n"
                    << "iterations      Amount of iterations"
                    << std::endl;

            return 0;
        }

        if(std::strcmp("--level", argv[i]) == 0)
            level = std::strtol(argv[++i], nullptr, 10);
        else if(std::strcmp("--iterations", argv[i]) == 0)
            iterations = std::strtol(argv[++i], nullptr, 10);
        else if(std::strcmp("--job", argv[i]) == 0) {
            if(std::strcmp("movegen", argv[++i]) == 0)
                is_movegen = true;
            else if(std::strcmp("eval", argv[i]) == 0)
                is_eval = true;
        }
        else {
            std::cout << "unknown args: " << argv[i] 
                << "\nuse --help"
                << std::endl;
            return 1;
        }
    }

    if(is_movegen)
        movegen_job(level);
    // else if(is_eval)
    //     eval_job(iterations);


    return 0;
}
