#include <leximaxIST_Encoder.h>
#include <leximaxIST_printing.h>
#include <leximaxIST_types.h>
#include <utility>
#include <vector>

namespace leximaxIST {

    /* a sorting network is a vector with size equal to the number of wires.
    * Entry i contains the last comparator (in the construction of the sorting network) that connects to wire i.
    * The comparator is represented as a pair. The first component is the other wire j that the comparator connects to.
    * The second component is the variable representing
    * the smallest of the outputs, if i < j
    * the greatest of the outputs, if i > j.*/

    void Encoder::encode_max(int var_out_max, int var_in1, int var_in2)
    {
        // encode var_out_max is equivalent to var_in1 OR var_in2
        add_clause(-var_out_max, var_in1, var_in2);
        add_clause(var_out_max, -var_in1);
        add_clause(var_out_max, -var_in2);
    }

    void Encoder::encode_min(int var_out_min, int var_in1, int var_in2)
    {
        // encode var_out_min is equivalent to var_in1 AND var_in2
        add_clause(-var_out_min, var_in1);
        add_clause(-var_out_min, var_in2);
        add_clause(var_out_min, -var_in1, -var_in2);
    }

    void Encoder::insert_comparator(int el1, int el2, const std::vector<int> *objective, SNET &sorting_network)
    {
        if (m_verbosity == 2)
            std::cout << "c Inserting comparator between wires " << el1 << " and " << el2 << '\n';
        // if the entry is empty, then it is the first comparator for that wire
        int var_in1 = (sorting_network[el1].first == -1) ? objective->at(el1) : sorting_network[el1].second;
        int var_in2 = (sorting_network[el2].first == -1) ? objective->at(el2) : sorting_network[el2].second;
        int var_out_min = fresh();
        int var_out_max = fresh();
        // encode outputs, if el1 > el2 then el1 is the largest, that is, the or. Otherwise, el1 is the smallest, i.e. the and.
        encode_max(var_out_max, var_in1, var_in2);
        encode_min(var_out_min, var_in1, var_in2);
        std::pair<int,int> comp1 (el2, 0);
        std::pair<int,int> comp2 (el1, 0);
        if(el1 > el2)
            comp1.second = var_out_max;
        else
            comp1.second = var_out_min;
        sorting_network[el1] = comp1;
        if(el2 > el1)
            comp2.second = var_out_max;
        else
            comp2.second = var_out_min;
        sorting_network[el2] = comp2;
    }

    int ceiling_of_half(int number)
    {
        // floor
        int result = number/2;
        // if number is odd then add 1
        if(number % 2 != 0)
            result++;
        return result;
    }

    // returns the number of comparators added to the sorting network by this function
    int Encoder::odd_even_merge(std::pair<std::pair<int,int>,int> seq1, std::pair<std::pair<int,int>,int> seq2, const std::vector<int> *objective, SNET &sorting_network)
    {
        int nb_comparators (0);
        int el1;
        int el2;
        int size1 = seq1.first.second;
        int size2 = seq2.first.second;
        if(size1 == 0 || size2 == 0){
            // nothing to merge
        }
        else if(size1 == 1 && size2 == 1){
            // merge two elements with a single comparator.
            el1 = seq1.first.first;
            el2 = seq2.first.first;
            insert_comparator(el1, el2, objective, sorting_network);
            ++nb_comparators;
        }
        else {
            // merge odd subsequences
            // size of odd subsequence is the ceiling of half of the size of the original sequence
            int offset1 = seq1.second;
            int offset2 = seq2.second;
            int first1 = seq1.first.first;
            int first2 = seq2.first.first;
            std::pair<int,int> p1(first1, ceiling_of_half(size1));
            std::pair<std::pair<int,int>,int> odd1(p1, 2*offset1);
            std::pair<int,int> p2(first2, ceiling_of_half(size2));
            std::pair<std::pair<int,int>,int> odd2(p2, 2*offset2);
            nb_comparators += odd_even_merge(odd1, odd2, objective, sorting_network);
            // merge even subsequences
            // size of even subsequence is the floor of half of the size of the original sequence
            p1 = std::make_pair(first1 + offset1, size1/2);
            std::pair<std::pair<int,int>,int> even1(p1, 2*offset1);
            p2 = std::make_pair(first2 + offset2, size2/2);
            std::pair<std::pair<int,int>,int> even2(p2, 2*offset2);
            nb_comparators += odd_even_merge(even1, even2, objective, sorting_network);
            // comparison-interchange
            for(int i{2}; i <= size1; i = i + 2){
                if(i == size1){
                    // connect last of seq1 to first element of seq2
                    el1 = first1 + offset1*(size1-1);
                    el2 = first2;
                    insert_comparator(el1, el2, objective, sorting_network);
                    ++nb_comparators;
                }
                else{
                    // connect i-th of seq1 with i+1-th of seq1
                    el1 = first1 + offset1*(i-1);
                    el2 = el1 + offset1;
                    insert_comparator(el1, el2, objective, sorting_network);
                    ++nb_comparators;
                }
            }
            int init = (size1 % 2 == 0) ? 2 : 1 ;
            for(int i{init}; i < size2; i = i + 2){
                    // connect i-th of seq2 with i+1-th of seq2
                    el1 = first2 + offset2*(i-1);
                    el2 = el1 + offset2;
                    insert_comparator(el1, el2, objective, sorting_network); 
                    ++nb_comparators;
            }
        }
        return nb_comparators;
    }

    // returns the number of comparators of the sorting network
    int Encoder::encode_network(const std::pair<int,int> elems_to_sort, const std::vector<int> *objective, SNET &sorting_network)
    {
        /* in the case the sorting network only has one element (this is important for the core-guided algorithm)
        * we must set the pair to (0, objective->at(0))
        * the first element is not used and it needs to be different from -1
        * the second element is the output variable of that wire
        */
        if (sorting_network.size() == 1) {
            sorting_network.at(0).first = 0;
            sorting_network.at(0).second = objective->at(0);
            return 0;
        }
        int nb_comparators (0);
        int size = elems_to_sort.second;
        int first_elem = elems_to_sort.first;
        if(size == 1){
            // do nothing - a single element is already sorted.
        }
        if(size > 1){
            int m = size/2;
            int n = size - m;
            std::pair<int,int> split1(first_elem, m);
            std::pair<int,int> split2(first_elem + m, n);
            // recursively sort the first m elements and the last n elements
            nb_comparators += encode_network(split1, objective, sorting_network);
            nb_comparators += encode_network(split2, objective, sorting_network);
            // merge the sorted m elements and the sorted n elements
            std::pair<std::pair<int,int>,int> seq1(split1,1);
            std::pair<std::pair<int,int>,int> seq2(split2,1);
            nb_comparators += odd_even_merge(seq1, seq2, objective, sorting_network);
        }
        return nb_comparators;
    }
    
    /* Create a sorting network to sort the obj_vars
     * Merge this sorting network with the old sorting network - the outputs are initially in sorted_vec
     * sorted_vec is changed and set to the outputs of the new (larger) sorting network.
     */
    void Encoder::merge_core_guided(const std::vector<std::vector<int>> &inputs_to_sort, const std::vector<std::vector<int>> &unit_core_vars)
    {
        for (size_t obj_index (0); obj_index < inputs_to_sort.size(); ++obj_index) {
            const std::vector<int> &obj_vars (inputs_to_sort.at(obj_index));
            if (obj_vars.size() > 0) {
                if (m_verbosity >= 1)
                    std::cout << "c Increasing the " << ordinal(obj_index + 1) << " sorting network...\n";
                // Create a sorting network to sort the obj_vars
                SNET sort_new_vars(obj_vars.size(), {-1,-1});
                const std::pair<int,int> elems_to_sort(0, obj_vars.size());
                if (m_verbosity == 2)
                    std::cout << "c Sorting the new variables\n";
                // update sorting network info - nb wires and comparators
                m_snet_info.at(obj_index).second += encode_network(elems_to_sort, &obj_vars, sort_new_vars);
                m_snet_info.at(obj_index).first += obj_vars.size();
                // Merge
                // the first old_size entries are equal to the old sorting network
                std::vector<int> &sorted_vec = m_sorted_vecs.at(obj_index);
                // remove unit_core_vars from the end of sorted_vec
                for (size_t k (0); k < unit_core_vars.at(obj_index).size(); ++k)
                    sorted_vec.pop_back();
                const size_t old_size (sorted_vec.size());
                SNET new_sort_net(old_size + obj_vars.size(), {-1,-1});
                for (size_t i (0); i < old_size; ++i) {
                    /* the first of the pair is the wire that is connected to i, through the last comparator
                    * in this case we don't know, so we put 0.
                    * we can put any value as long as it is different from -1.
                    * -1 is used for the case there is no comparator connecting wire i.
                    */
                    new_sort_net.at(i).first = 0;
                    // the second of the pair is the variable associated with the output of the last comparator of wire i.
                    new_sort_net.at(i).second = sorted_vec.at(i);
                }
                // the last obj_vars.size() entries are equal to sort_new_vars
                for (size_t i (old_size); i < old_size + obj_vars.size(); ++i) {
                    new_sort_net.at(i) = sort_new_vars.at(i - old_size);
                }
                const std::pair<int, int> p1 (0, old_size);
                const std::pair<int, int> p2 (old_size, obj_vars.size());
                const std::pair<std::pair<int, int>, int> seq1 (p1, 1); // ((first wire, number of elements), offset)
                const std::pair<std::pair<int, int>, int> seq2 (p2, 1);
                if (m_verbosity == 2)
                    std::cout << "c Merging the sorting networks\n";
                // Since the sorting network has comparators connecting all wires, hence the nullptr. It is not used.
                m_snet_info.at(obj_index).second += odd_even_merge(seq1, seq2, nullptr, new_sort_net); // also, update m_snet_info
                // set sorted_vec to the outputs of new_sort_net
                sorted_vec.resize(old_size + obj_vars.size());
                for (size_t i (0); i < old_size + obj_vars.size(); ++i)
                    sorted_vec.at(i) = new_sort_net.at(i).second;
            }
        }
        if (m_verbosity >= 1) {
            for (int j (0); j < m_num_objectives; ++j)
                print_snet_info(j);
        }
        add_unit_core_vars(unit_core_vars);
    }
}/* namespace leximaxIST */
