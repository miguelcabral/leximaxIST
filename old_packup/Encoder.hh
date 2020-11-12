/******************************************************************************\
 *    This file is part of packup.                                            *
 *                                                                            *
 *    packup is free software: you can redistribute it and/or modify          *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation, either version 3 of the License, or       *
 *    (at your option) any later version.                                     *
 *                                                                            *
 *    packup is distributed in the hope that it will be useful,               *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *    GNU General Public License for more details.                            *
 *                                                                            *
 *    You should have received a copy of the GNU General Public License       *
 *    along with packup.  If not, see <http://www.gnu.org/licenses/>.         *            
\******************************************************************************/           
/* 
 * File:   Encoder.hh
 * Author: mikolas
 *
 * Created on September 26, 2010, 2:01 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef ENCODER_HH
#define	ENCODER_HH
#include "common_types.hh"
#include "SolverWrapperTypes.hh"
#include "SolutionReader.hh"
#include "InstallableUnit.hh"
#include "EncoderTypes.hh"
#include "IntervalVariables.hh"
#include "PackageVersionVariables.hh"
#include "SolverWrapper.hh"
#include "Printer.hh"
#include "NotRemoved.hh"

using namespace version_operators;

class Encoder {
public:
    Encoder(SolverWrapper<ClausePointer>& solver,IDManager& id_manager);
    virtual ~Encoder();
    void add_unit (InstallableUnit* unit);
    void add_install (CONSTANT PackageVersionsList& list);
    void add_upgrade (CONSTANT PackageVersionsList& list);
    void add_remove (CONSTANT PackageVersionsList& list);

    void set_criterion(Criterion optimization_criterion);
    void set_lexicographic(CONSTANT vector<Objective>& lexicographic_ordering);
    void set_solution_file(ostream* file) {psolution_file = file;}
    inline CONSTANT vector<Objective>& get_lexicographic() const {return lexicographic;}
    inline void disable_solving () {should_solve=false;}
    inline void set_iv(UINT piv) {iv=piv;}
    inline void set_opt_edges(int v=1) {opt_edges=v;}
    inline void set_opt_not_removed(bool v=true) {opt_not_removed=v;}
    inline bool get_opt_edges() const {return opt_edges;}
    inline bool get_opt_not_removed() const {return opt_not_removed;}
    inline void enable_valuation() {valuation = true;}
    void encode();
    bool solution();
    void print_solution();
    void print_time();
    SolverWrapper<ClausePointer>& get_solver_wrapper() {return solver;}

    UINT unit_count;
    UINT cut_off;
#ifdef MAPPING
    inline unordered_map<Variable, string>& get_variable_names() {return variable_names;}
    inline ofstream& get_mapping() {return mapping;}
#endif
private:
    vector<Objective>             lexicographic;
    ostream*                      psolution_file;
    bool                          should_solve;
    SolverWrapper<ClausePointer>& solver;
    IDManager&                    id_manager;
#ifdef MAPPING
    VariableToName                variable_names;// Names of variables for debugging purposes.
    ofstream                      mapping; // file for recording mapping from variable numbers to their names
#endif
    PackageVersionVariables       variables;
    IntervalVariables             interval_vars;
#ifdef CONV_DBG
    Printer printer;
#endif

    bool valuation;
    int opt_edges; // add redundancy implications between interval variables
    bool process_recommends; // flag whether recommends should be taken into account (currently used only for the trendy criterion)
    UINT iv;
    bool printing_started_or_finished;
    vector<ClausePointer> recommends_clauses_pointers;

    PackageUnits read_units; // installable
    PackageUnits needed_units;//installable units after slicing

    FeatureToUnits feature_units; //units that provide a given feature
    FeatureToVersions feature_versions;//versions of a given feature
    PackageToVersions needed_feature_versions;//versions of a feature after slicing
    PackageVersionSet required_features;//not needed, replace with needed_feature_versions TODO!!

    PackageVersionsSet install; // entities that must be installed
    PackageVersionsList upgrade; // upgrade requirement
    PackageVersionsList remove; // remove requirement

    bool opt_not_removed; // flag whether to use optimization "not removed"
    NotRemoved not_removed; // a set of packages that will certainly be in the final solution, valid only if opt_not_removed is on

    bool is_any_installed_package(const UnitVector& unit_vector);
    bool is_installed_feature (CONSTANT PackageVersion& feature);
    bool find_latest_installed_version_in_original_problem (CONSTANT string &name,Version& version);
    bool latest_installed_feature_version (CONSTANT string &name,Version& version);
    bool latest_installed_package_version (CONSTANT string &name,Version& version);
    inline bool is_feature(const string &name) const {return feature_versions.find(name) != feature_versions.end();}

    void handle_keep (InstallableUnit* unit);
    void handle_provides (InstallableUnit* unit);

    void refine_upgrade_request ();

    void encode_lexicographic();
    void encode_new(XLINT& total_weight, bool maximize);
    void encode_unmet_recommends(XLINT& total_weight,bool maximize);
    void encode_not_up_to_date(XLINT& total_weight,bool maximize);
    void encode_removed(XLINT& total_weight,bool maximize);
    void encode_changed(XLINT& total_weight,bool maximize);
    XLINT get_top(CONSTANT Objective& objective) const;
    XLINT get_top(CONSTANT vector<Objective>& lexicographic) const;

    void encode_provides ();
    void encode_keep ();

    void encode_upgrade ();
    void atmost_1 (CONSTANT string& package_name, Version from);
    void atmost_1_feature (CONSTANT string& package_name, Version from);

    void encode_request_remove();
    void encode (CONSTANT InstallableUnit* unit);
    void encode_conflicts (CONSTANT InstallableUnit* unit);
    void encode_depends (CONSTANT InstallableUnit* unit);
    void encode_recommends (CONSTANT InstallableUnit* unit);
    void encode_package_self_conflict (CONSTANT InstallableUnit* unit,
                               CONSTANT PackageVersions& pvs);
    void encode_not_self_conflict (CONSTANT InstallableUnit* unit,
                                   CONSTANT PackageVersions& pvs);

    void slice ();
    void slice_changed ();
    
    void need (CONSTANT PackageVersions& versions);
    void need_packages(CONSTANT string  &package_name, Operator version_operator, Version operand);
    void need_features(CONSTANT string  &feature_name, Operator version_operator, Version operand);
    void need (InstallableUnit* unit);
    void need_latest (CONSTANT string& package_name);
    void need (PackageVersionsCNF cnf);
    void need_feature(CONSTANT PackageVersion &feature);

    void request_to_clause (CONSTANT PackageVersions& pvs,
                            LiteralVector& literals);
    void disable_to_term (CONSTANT PackageVersions& pvs,
                          LiteralVector& term);


    bool require_up_to_clause (CONSTANT string& name, Version from,
                            const UnitVector *unit_vector,
                            const VersionVector *version_vector,
                            LiteralVector& literals);
    bool require_down_to_clause (CONSTANT string& name, Version from,
                            const UnitVector *unit_vector,
                            const VersionVector *version_vector,
                            LiteralVector& literals);
    bool disable_up_to_term (CONSTANT string& name, Version from,
                            const UnitVector *unit_vector,
                            const VersionVector *version_vector,
                            LiteralVector& literals);
    bool disable_down_to_term (CONSTANT string& name, Version from,
                            const UnitVector *unit_vector,
                            const VersionVector *version_vector,
                            LiteralVector& literals);



    void requests_to_clause (CONSTANT VersionsList::const_iterator begin,
                             CONSTANT VersionsList::const_iterator end,
                             LiteralVector& literals);

    inline Variable new_variable() { return id_manager.new_id(); }
    void conflict_all (CONSTANT Variable& pv, CONSTANT VariableSet& set);
    void add_providers(CONSTANT PackageVersions&  features, VariableSet& providers);
    void add_providers(CONSTANT PackageVersion&  feature, VariableSet& providers);

    inline Variable get_package_version_variable(CONSTANT PackageVersion& pv) {
        Variable return_value=0;
        CONSTANT bool found = variables.find_variable(pv, return_value);
#ifdef CONV_DBG
        if (!found) { cerr << "no variable for: " << pv.to_string() << endl;}
#endif
        if (!found) assert(0);//getting rid of the compiler warning
        return return_value;
    }

    //global processing (for whole package repository)
    void process_interval_variables ();
    void process_require_up(const string& package,
                            const VersionVector& intervals,
                            const VersionVector& package_versions,
                            bool is_feature=false);

    void process_require_down(const string& package,
                            const VersionVector& intervals,
                            const VersionVector& package_versions,
                            bool is_feature=false);

    void process_disable_up(const string& package,
                            const VersionVector& intervals,
                            const VersionVector& package_versions,
                            bool is_feature=false);

    void process_disable_down(const string& package,
                            const VersionVector& intervals,
                            const VersionVector& package_versions,
                            bool is_feature=false);

    void encode_edges();
    void assert_not_removed();

    void paranoid_score (IntVector& model);
    void trendy_score(IntVector& model);
    int removed_score(IntVector& model);
    int new_score(IntVector& model);
    int changed_score(IntVector& model);
    int notuptodate_score(IntVector& model);
    int recommends_score (IntVector& model);
    bool is_installed (const InstallableUnit* unit,IntVector& model);
    bool is_any_installed (PackageVersions& package,IntVector& model);
    bool is_any_installed_feature(CONSTANT string &feature_name, 
                                       Operator version_operator, 
                                       Version operand,
                                       IntVector& model);
    bool is_installed_feature(const PackageVersion& feature, IntVector& model);

    bool _process_not_uptodate;
    inline bool process_not_uptodate() {return _process_not_uptodate;}

    enum CT { CT_MIN, CT_MAX, CT_NO };
    CT gt(OBJECTIVE_FUNCTION o,vector<Objective>& lex) {
        FOR_EACH(vector<Objective>::const_iterator,oi,lex) {
            if (oi->first==o) return oi->second ? CT_MAX : CT_MIN;
        }
        return CT_NO;
    }
    void print_valuation(ostream& output,IntVector& model);

public:
    bool cs;
    PackageVersionSet* stc;
    void check_solution();
};

#endif	/* ENCODER_HH */
