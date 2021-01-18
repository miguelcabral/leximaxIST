#include <Leximax_encoder.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <string.h>
#include <errno.h>
#include <algorithm> // for std::sort()

bool descending_order (LINT i, LINT j) { return (i>j); }

// send signal signum to external solver if it is running, and get best solution
int Leximax_encoder::terminate(int signum)
{
    // TODO
    if (m_child_pid == 0) { // external solver is not running
        // check if there is output file to be read 
        // (this happens if signal is caught after external solver has finished
        //      and before the solution has been read)
        if (m_solver_output) {
            read_solver_output(m_solution);
        }
        if (!m_leave_temporary_files)
            remove_tmp_files(); 
        return 0;
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
            if (!m_leave_temporary_files)
                remove_tmp_files(); 
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
                // but solution may be worse than previous iteration - compare
                std::vector<LINT> new_sol;
                if (read_solver_output(new_sol) != 0) {
                    if (!m_leave_temporary_files)
                        remove_tmp_files();
                    return -1;
                }
                if (!new_sol.empty()) { // otherwise unsat, nothing to do
                    if (m_solution.empty())
                         m_solution = new_sol;
                    else {
                        // check which solution is leximax-better
                        std::vector<LINT> new_obj_vec (get_objective_vector(new_sol));
                        std::vector<LINT> old_obj_vec (get_objective_vector(m_solution));
                        std::sort(new_obj_vec.begin(), new_obj_vec.end(), descending_order());
                        std::sort(old_obj_vec.begin(), old_obj_vec.end(), descending_order());
                        for (size_t j (0); j < new_obj_vec.size(); ++j) {
                            if (new_obj_vec[j] < old_obj_vec[j]) {
                                m_solution = new_sol;
                                break;
                            }
                            else if (new_obj_vec[j] > old_obj_vec[j])
                                break; // m_solution is better
                        }
                    }
                }
                if (!m_leave_temporary_files)
                    remove_tmp_files(); 
                return 0;
            }
        }
        // timeout has been reached, clean up and return
        if (!m_leave_temporary_files)
            remove_tmp_files(); 
    }
    return 0;
}
