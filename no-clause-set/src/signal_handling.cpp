#include <leximaxIST_Encoder.h>
#include <vector>
#include <signal.h>
#include <chrono>
#include <thread>
#include <string.h>
#include <errno.h>
#include <algorithm> // for std::sort()
#include <sys/wait.h> // for waitpid()


namespace leximaxIST {

    bool descending_order (long long i, long long j) { return i > j; }

    // send signal signum to external solver if it is running, and get best solution
    int Encoder::terminate(int signum)
    {
        // TODO
        if (m_child_pid == 0) { // external solver is not running
            // check if there is output file to be read 
            // (this happens if signal is caught after external solver has finished
            //      and before the solution has been read)
            if (m_solver_output) {
                std::vector<long long> model;
                if (read_solver_output(model) != 0) {
                    if (!m_leave_temporary_files)
                        remove_all_tmp_files();
                    return -1;
                }
                m_solution = model;
                m_sat = !(m_solution.empty());
            }
            if (!m_leave_temporary_files)
                remove_all_tmp_files();
            return 0;
        }
        else {
            // if external solver is running then:
            // send signal to external solver with kill function
            if (kill(m_child_pid, signum) != 0) {
                std::string errno_str (strerror(errno));
                std::string errmsg ("In Encoder::terminate: when calling");
                errmsg += " kill() to send a signal to the external solver (pid ";
                errmsg += m_child_pid + "): '" + errno_str + "'";
                print_error_msg(errmsg);
                if (!m_leave_temporary_files)
                    remove_all_tmp_files(); 
                return -1;
            }
            double accumulated_time (0.0);
            // every half second check if external solver has finished until:
            // 1) timeout has been reached or 2) solver finishes
            while (accumulated_time < m_timeout) {
                accumulated_time += 500.0; // 500 milliseconds = 0.5 seconds
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                int pid_status;
                int retv = waitpid(m_child_pid, &pid_status, WNOHANG); // check if child has finished
                if (retv == -1) {
                    std::string errmsg (strerror(errno));
                    print_error_msg("Error waiting for child process: " + errmsg);
                    if (!m_leave_temporary_files)
                        remove_all_tmp_files();
                    return -1;
                }
                else if (retv == m_child_pid) {
                    // external solver has finished
                    // but solution may be worse than previous iteration - compare
                    std::vector<long long> new_sol;
                    if (read_solver_output(new_sol) != 0) {
                        if (!m_leave_temporary_files)
                            remove_all_tmp_files();
                        return -1;
                    }
                    m_sat = !(new_sol.empty());
                    if (m_sat) { // otherwise unsat, nothing to do
                        if (m_solution.empty()) // first iteration
                            m_solution = new_sol;
                        else {
                            // check which solution is leximax-better
                            std::vector<long long> new_obj_vec (get_objective_vector(new_sol));
                            std::vector<long long> old_obj_vec (get_objective_vector(m_solution));
                            std::sort(new_obj_vec.begin(), new_obj_vec.end(), descending_order);
                            std::sort(old_obj_vec.begin(), old_obj_vec.end(), descending_order);
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
                        remove_all_tmp_files(); 
                    return 0;
                }
            }
            // timeout has been reached, clean up and return
            if (!m_leave_temporary_files)
                remove_all_tmp_files(); 
        }
        return 0;
    }
}/* namespace leximaxIST */
