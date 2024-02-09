#ifndef DOCTEST_EXTENSION_HPP_
#define DOCTEST_EXTENSION_HPP_

#include "doctest_base.h"

/* this is :very: simple implementation of death tests in doctest for Unix systems */

#define DOCTEST_CREATE_AND_REGISTER_FUNCTION_TEMPLATE(f, decorators, PARAM) \
    template<bool PARAM = true>                                             \
    static void f();                                                        \
    DOCTEST_REGISTER_FUNCTION(DOCTEST_EMPTY, f, decorators)                 \
    template<bool PARAM>                                                    \
    static void f()

#ifdef DEATH_TESTS_ENABLED
#define DOCTEST_DEATH_TEST(decorators) \
    DOCTEST_CREATE_AND_REGISTER_FUNCTION_TEMPLATE(DOCTEST_ANONYMOUS(DOCTEST_ANON_FUNC_), decorators, CHECK_DEATH_ENABLED)
#else
#define DOCTEST_DEATH_TEST(decorators) \
    DOCTEST_CREATE_AND_REGISTER_FUNCTION_TEMPLATE(DOCTEST_ANONYMOUS(DOCTEST_ANON_FUNC_), decorators * doctest::skip() * doctest::should_fail(), CHECK_DEATH_ENABLED)
#endif

#ifndef DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DEATH_TEST(name) DOCTEST_DEATH_TEST(name)
#endif /* DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES */

#ifdef DEATH_TESTS_ENABLED

#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace doctest {

class SignalManager {
    struct sigaction old_action_usr1;
    struct sigaction old_action_chld;
    sigset_t old_set;
    inline static void handler(int sig_nr) {
        if(sig_nr == SIGUSR1) { // test failed
            return;
        }
        // test passed (sig_nr == SIGCHLD)
        waitpid(-1, &status, WNOHANG); // handle zombie process
    }

public:
    // has to be inline because of ODR (this is header file)
    static inline int status = 0;
    SignalManager() {
        sigset_t sg;
        sigemptyset(&sg);
        sigaddset(&sg, SIGUSR1);
        sigaddset(&sg, SIGCHLD);
        sigprocmask(SIG_BLOCK, &sg, &old_set);
    }
    void setHandlers() {
        status = 0;
        struct sigaction ac;
        ac.sa_flags = 0;
        sigemptyset(&ac.sa_mask);
        ac.sa_handler = handler;
        sigaction(SIGUSR1, &ac, &old_action_usr1);
        sigaction(SIGCHLD, &ac, &old_action_chld);
    }
    void resetSigmask() {
        sigprocmask(SIG_SETMASK, &old_set, NULL);
    }
    void resetHandlers() {
        sigaction(SIGUSR1, &old_action_usr1, NULL);
        sigaction(SIGCHLD, &old_action_chld, NULL);
    }
    void waitForSignal() {
        sigset_t sg;
        sigemptyset(&sg);
        sigsuspend(&sg);
    }
};

template<typename Func>
void check_death(Func && fun) {
    SignalManager context{}; // block SIGUSR1 and SIGCHLD
    pid_t pid = fork();
    if(pid == -1) {
        std::cerr << "Fork failed!\n";
        exit(EXIT_FAILURE);
    }
    if(pid == 0) {              // child
        context.resetSigmask(); // change signal mask child to default
        // silence doctest output
        // TODO(FilipKon13) do something better? send output to parent?
        std::cout.flush();
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        fun();                    // fun should exit
        kill(getppid(), SIGUSR1); // inform parent about failure
        pause();                  // wait for termination
        exit(0);                  // unreachable
    }
    // parent
    context.setHandlers();
    context.waitForSignal();     // SIGUSR1 or SIGCHLD
    if(!SignalManager::status) { // test failed
        kill(pid, SIGKILL);      // kill child
        context.waitForSignal(); // wait for SIGCHLD
        FAIL("Death test failed");
    }
    // reset to normal state
    context.resetSigmask();
    context.resetHandlers();
}

}; /* namespace doctest */

// can only be used if CHECK_DEATH_ENABLED template parameter is defined, i.e. inside DEATH_TEST
#define CHECK_DEATH(expr)                                                               \
    do {                                                                                \
        static_assert(CHECK_DEATH_ENABLED && "Use CHECK_DEATH only inside DEATH_TEST"); \
        doctest::check_death([&] { expr; });                                            \
    } while(0)

#else /* DEATH_TESTS_ENABLED */

// this is reached only with --no-skip option
#define CHECK_DEATH(expr)                     \
    do {                                      \
        [[maybe_unused]] auto const _ = [&] { \
            expr;                             \
        };                                    \
        FAIL("Could not run death test");     \
    } while(0)

#endif /* DEATH_TESTS_ENABLED */

#endif /* DOCTEST_EXTENSION_HPP_ */