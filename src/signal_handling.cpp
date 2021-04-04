#include <leximaxIST_Encoder.h>
#include <leximaxIST_error.h>
#include <vector>
#include <utility>
#include <signal.h>
#include <chrono>
#include <thread>
#include <string.h>
#include <errno.h>
#include <algorithm> // for std::sort()
#include <sys/wait.h> // for waitpid()


namespace leximaxIST {

    bool descending_order (int i, int j) { return i > j; }

    // send signal signum to external solver if it is running, and get best solution
    int Encoder::terminate()
    { // TODO: in the end also do clean up: free dynamically allocated memory
        // TODO MAYBE CHANGE THIS: set m_child_pid = 0 in the beggining of solving and in constructor
        // each time fork() is run you update m_child_pid. What can occur: m_child_pid may be from previous iteration
        // in this case waitpid will fail and say: no child pid. If it fails just return
        // Or: m_child_pid = 0. In this case I know I have not called fork() -> just return
        // Otherwise: m_child_pid has not been waited for -> call waitpid. If it does not fail, the external solver is running.
        // In that case, send the signal SIGTERM
        // In https://www.gnu.org/software/libc/manual/html_node/Signals-in-Handler.html it is explained that:
        // I can send signals to child process because handlers block signals. Hence,
        // Send SIGTERM to child process and wait until timeout.
        // WARNING: if timeout is reached then parent does not wait for child process and it becomes a zombie
        // In this case, return the m_child_pid so that the caller can clean up the zombie process in the future by calling waitpid
        if (m_child_pid == 0) { // external solver is not running
            // check if there is output file to be read 
            // (this happens if signal is caught after external solver has finished
            //      and before the solution has been read)
            /*if (m_solver_output) {
                std::vector<int> model;
                if (read_solver_output(model) != 0) {
                    if (!m_leave_tmp_files)
                        remove_tmp_files();
                    return -1;
                }
                m_solution = std::move(model);
                //m_sat = !(m_solution.empty());
            }*/
            if (!m_leave_tmp_files)
                remove_tmp_files();
            return 0;
        }
        else {
            // if external solver is running then:
            // send signal to external solver with kill function
            // I think it might be unlikely, but it could happen: child has finished and I do not know it
            // in that case, kill() will fail because child does not exist any more
            if (kill(m_child_pid, SIGTERM) != 0) {
                std::string errno_str (strerror(errno));
                std::string errmsg ("In Encoder::terminate: when calling");
                errmsg += " kill() to send a signal to the external solver (pid ";
                errmsg += m_child_pid + "): '" + errno_str + "'";
                print_error_msg(errmsg);
                if (!m_leave_tmp_files)
                    remove_tmp_files(); 
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
                    if (!m_leave_tmp_files)
                        remove_tmp_files();
                    return -1;
                }
                else if (retv == m_child_pid) {
                    // external solver has finished
                    // but solution may be worse than previous iteration - compare
                    std::vector<int> new_sol;
                    if (/*read_solver_output(new_sol) != 0*/false) {
                        if (!m_leave_tmp_files)
                            remove_tmp_files();
                        return -1;
                    }
                    //m_sat = !(new_sol.empty());
                    if (m_status == 's') { // otherwise unsat, nothing to do
                        if (m_solution.empty()) // first iteration
                            m_solution = std::move(new_sol);
                        else {
                            // check which solution is leximax-better
                            std::vector<int> new_obj_vec (get_objective_vector(new_sol));
                            std::vector<int> old_obj_vec (get_objective_vector(m_solution));
                            std::sort(new_obj_vec.begin(), new_obj_vec.end(), descending_order);
                            std::sort(old_obj_vec.begin(), old_obj_vec.end(), descending_order);
                            for (size_t j (0); j < new_obj_vec.size(); ++j) {
                                if (new_obj_vec[j] < old_obj_vec[j]) {
                                    m_solution = std::move(new_sol);
                                    break;
                                }
                                else if (new_obj_vec[j] > old_obj_vec[j])
                                    break; // m_solution is better
                            }
                        }
                    }
                    if (!m_leave_tmp_files)
                        remove_tmp_files(); 
                    return 0;
                }
            }
            // timeout has been reached, clean up and return
            if (!m_leave_tmp_files)
                remove_tmp_files(); 
        }
        return 0;
    }
}/* namespace leximaxIST */
