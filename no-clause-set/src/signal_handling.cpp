#include <Leximax_encoder.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <string.h>
#include <errno.h>

void Leximax_encoder::terminate(int signum)
{
    // TODO
    if (m_child_pid == 0) { // external solver is not running
        // check if there is output file to be read
        if (m_solver_output) {
            // if there is: read solver output
            read_solver_output();
        }
        return;
    }
    else {
        // if external solver is running then:
        // send signal to external solver with kill function
        if (kill(m_child_pid, signum) != 0) {
            std::string errno_str (strerror(errno));
            std::string errmsg ("In Leximax_encoder::terminate: when calling";
            errmsg += " kill() to send a signal to the external solver (pid ";
            errmsg += m_child_pid + "): '" + err_str + "'";
            print_error_msg(errmsg);
            return -1;
        }
        double accumulated_time (0.0);
        // every half second check if external solver has finished until:
        // 1) timeout has been reached or 2) solver finishes
        while (accumulated_time < m_timeout) {
            accumulated_time += 500.0; // 500 milliseconds = 0.5 seconds
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            retv = waitpid(m_child_pid, &pid_status, WNOHANG);
            if (retv == -1) {
                print_waitpid_error(strerror(errno));
                return -1;
            }
            else if (retv == m_child_pid) {
                // external solver has finished
                read_solver_output();
                if (!m_leave_temporary_files)
                    remove_tmp_files(); 
                return 0;
            }
        }

        // at the end check if there is output to read
        read_solver_output();
        if (!m_leave_temporary_files)
            remove_tmp_files(); 
        // if not return, otherwise:
        // read solution file and store this solution
        // if unsatisfiable return
        // otherwise:
        // if I have a solution from previous iteration then compare with new solution
        // choose best solution and return

        // wait a few seconds for the external solver to return the best solution so far
        // read solution
        // if satisfiable set m_sat to true, else m_sat to false and m_solution to empty
        // store solution in m_solution, if one exists
    }

}
