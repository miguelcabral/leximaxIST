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
 * File:   Encoder.cc
 * Author: mikolas
 * 
 * Created on September 26, 2010, 2:01 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include "Encoder.hh"
#include "PackageVersionVariables.hh"
#include "Encoder.hh"
#include "SolverWrapper.hh"
#include "NotRemoved.hh"

Encoder::Encoder(SolverWrapper<ClausePointer>& solver,IDManager& id_manager)
:unit_count(0)
,cut_off(INT_MAX)
,psolution_file(NULL)
//---- scalar values initialization
,should_solve(true)
,solver(solver)
,id_manager(id_manager)
//----- variables initialization
,        variables(id_manager
#ifdef MAPPING 
                 , variable_names,mapping
#endif
           )
// ---- interval_vars initialization
,interval_vars(id_manager
#ifdef  MAPPING
                 , variable_names,mapping
#endif
           )
#ifdef CONV_DBG
// ---- printer initialization
       , printer (variable_names)
#endif
, valuation(false)
, opt_edges(0)
, iv(3)
, printing_started_or_finished(false)
, opt_not_removed(false)
, not_removed(needed_units)
, cs(false)
//===== end of variable initialization
{
#ifdef CONV_DBG
    solver.set_printer(&printer);
#endif
    solver.init();    
}

Encoder::~Encoder() {}

void Encoder::add_unit(InstallableUnit* unit) {
    unit->needed = false;
    // Insert the unit into a collection of units
    record_unit(read_units, unit);

    // Record information about the provides field
    handle_provides(unit);

    // Record information about the keep field
    handle_keep(unit);
}

void Encoder::add_install(CONSTANT PackageVersionsList& list) {
    FOR_EACH(PackageVersionsList::const_iterator, i, list)
       install.insert(*i);
}

void Encoder::add_upgrade (CONSTANT PackageVersionsList& list)
{upgrade.insert (upgrade.end(), list.begin(), list.end ());}
void Encoder::add_remove (CONSTANT PackageVersionsList& list)
{remove.insert (remove.end(), list.begin(), list.end ());}

void Encoder::set_lexicographic(const vector<Objective>& lexicographic_ordering) {
    OUT ( cerr <<  "lex: "; print(lexicographic_ordering,cerr);  cerr << endl; )
    // store the ordering
    lexicographic.insert(lexicographic.end(), lexicographic_ordering.begin(),lexicographic_ordering.end());
    OUT ( cerr <<  "lex: "; print(lexicographic,cerr);  cerr << endl; )
    // determine which information needs to be stored to be able to address the required functions
    process_recommends=false;
    _process_not_uptodate=false;

    FOR_EACH (vector<Objective>::const_iterator, objective_index, lexicographic) {
        CONSTANT Objective objective = *objective_index;
        switch (objective.first) {
            case COUNT_UNMET_RECOMMENDS:
                if (!objective.second) process_recommends=true;
                break;
            case COUNT_NOT_UP_TO_DATE:
                if (!objective.second) _process_not_uptodate=true;
                break;
            case COUNT_NEW:
            case COUNT_REMOVED:
            case COUNT_CHANGED:
                break;
        }
    }    
}

void Encoder::set_criterion(Criterion optimization_criterion) {
    vector<Objective> lexicographic_ordering;
    switch (optimization_criterion)
    {
        case PARANOID:
            lexicographic_ordering.push_back(Objective(COUNT_REMOVED, false));
            lexicographic_ordering.push_back(Objective(COUNT_CHANGED, false));
            break;
        case TRENDY:
            lexicographic_ordering.push_back(Objective(COUNT_REMOVED, false));
            lexicographic_ordering.push_back(Objective(COUNT_NOT_UP_TO_DATE, false));
            lexicographic_ordering.push_back(Objective(COUNT_UNMET_RECOMMENDS, false));
            lexicographic_ordering.push_back(Objective(COUNT_NEW, false));
            break;
        case NO_OPTIMIZATION:
            break;
    }
    set_lexicographic(lexicographic_ordering);
}

void Encoder::handle_keep(InstallableUnit* unit) {
    if (!unit->installed) return;
    switch (unit->keep) {
        case KEEP_VERSION:
            install.insert(PackageVersions(unit->name,VERSIONS_EQUALS,unit->version));
            break;
        case KEEP_PACKAGE:
            install.insert(PackageVersions(unit->name,VERSIONS_NONE,0));
            break;
        case KEEP_FEATURE:
            FOR_EACH (PackageVersionList::const_iterator,i,unit->provides) {
                PackageVersion f=*i;
                if (f.version()==0)
                    install.insert(PackageVersions(f.name(),VERSIONS_NONE,0));
                else
                    install.insert(PackageVersions(f.name(),VERSIONS_EQUALS,f.version()));
            }
            break;
        case KEEP_NONE:
            break;
    }
}

void Encoder::handle_provides(InstallableUnit* unit) {
    CONSTANT PackageVersionList &provides = unit->provides;
    FOR_EACH(PackageVersionList::const_iterator, i, provides) {
        CONSTANT PackageVersion& feature = *i;
        record_unit(feature_units, feature, unit);
        record_version(feature_versions, feature.name(), feature.version());
    }
}

void Encoder::slice () {
    //process upgrade
    FOR_EACH(PackageVersionsList::const_iterator,i,upgrade) need (*i);
    //process install    
    FOR_EACH(PackageVersionsSet::const_iterator,i,install) need (*i);

    //process preference
    if (gt(COUNT_REMOVED, lexicographic)==CT_MIN) {
            FOR_EACH(PackageUnits::const_iterator,index,read_units)
            {
                UnitVector &unit_vector=*(index->second);
                CONSTANT bool any_installed = is_any_installed_package(unit_vector);
                //if any package is installed, then all of its versions are needed
                if (any_installed) need_packages(index->first, VERSIONS_NONE, 0);
            }
    }
    
    if (gt(COUNT_NEW, lexicographic)==CT_MAX) {
            FOR_EACH(PackageUnits::const_iterator,index,read_units)
            {
                UnitVector &unit_vector=*(index->second);
                CONSTANT bool any_installed = is_any_installed_package(unit_vector);
                //if no versions are installed, then all of its versions are needed
                if (!any_installed) need_packages(index->first, VERSIONS_NONE, 0);
            }
    }

    if (gt(COUNT_UNMET_RECOMMENDS, lexicographic) == CT_MAX) {
       FOR_EACH(PackageUnits::const_iterator,index,read_units)
       {
           UnitVector &unit_vector=*(index->second);
           FOR_EACH (UnitVector::iterator, unit_index, unit_vector)
           {
               InstallableUnit* unit=*unit_index;
               if (!unit->recommends_cnf.empty()) need(unit);
           }
       }
    }

    if (gt(COUNT_NOT_UP_TO_DATE, lexicographic)==CT_MAX) {
            FOR_EACH(PackageUnits::const_iterator,index,read_units)
            {
                UnitVector &unit_vector=*(index->second);
                if (unit_vector.size()<=1) continue;// if a package is only one version it cannot be not up to date
                need_packages(index->first, VERSIONS_NONE, 0);
            }
    }


    slice_changed();

    // make sure that variables are introduced for all features provided by  needed packages
   FOR_EACH(PackageUnits::const_iterator,index,needed_units)
   {
        UnitVector &unit_vector = *(index->second);
        FOR_EACH(UnitVector::iterator, unit_index, unit_vector) {
            CONSTANT InstallableUnit& unit = **unit_index;
            FOR_EACH(PackageVersionList::const_iterator, feature_index, (unit.provides)) {
                CONSTANT PackageVersion& feature = *feature_index;
                if (CONTAINS(required_features, feature))
                    continue; //feature already required, therefore all of its providers have been recorded
                if (!variables.has_variable(feature)) {
                    variables.add_variable_feature(feature);
                    record_version(needed_feature_versions, feature.name(), feature.version());
                }
            }
        }
   }

  OUT( cerr << "number of packages: " << unit_count << endl; )
}

void Encoder::slice_changed() {
   if (gt(COUNT_CHANGED, lexicographic)==CT_NO)  return;
   CONSTANT bool maximize = (gt(COUNT_CHANGED, lexicographic)==CT_MAX);
   FOR_EACH(PackageUnits::const_iterator,index,read_units)
   {
       UnitVector &unit_vector=*(index->second);
       FOR_EACH (UnitVector::iterator, unit_index, unit_vector)
       {
           InstallableUnit* unit=*unit_index;
           if (maximize && !unit->installed) need(unit);
           if (!maximize && unit->installed) need(unit);
       }
   }
}


void Encoder::need_packages(CONSTANT string &package_name, Operator version_operator, Version operand)
{
    CONSTANT PackageUnits::const_iterator i = read_units.find(package_name);
    if (i==read_units.end()) return;
    CONSTANT UnitVector &vector=*(i->second);
    CONSTANT UINT size = vector.size();
    for (UINT index = 0; index < size; ++index)
    {
        InstallableUnit *unit = vector[index];
        if (evaluate(unit->version, version_operator, operand))
            need (unit);
    }
}

void Encoder::need_feature(CONSTANT PackageVersion &feature)
{
    if (CONTAINS(required_features, feature)) return; // already traversed
    FeatureToUnits::const_iterator i0=feature_units.find(feature);
    if (i0!=feature_units.end())
    {

        OUT ( cerr <<  "need: " << (feature.name()) << ":" << (feature.version()) << endl; )
        // Add the information that the feature is needed
        required_features.insert (feature);
        variables.add_variable_feature (feature);
        record_version (needed_feature_versions,feature.name(),feature.version ());

        // All of the providers of the feature are needed
        CONSTANT UnitVector &providers_vector=*(i0->second);
        FOR_EACH (UnitVector::const_iterator,vi,providers_vector)
          need(*vi);
    }
}

void Encoder::need_features(CONSTANT string  &feature_name, Operator version_operator, Version operand)
{
    // Process unversioned providers
    need_feature(PackageVersion (feature_name,0));

    CONSTANT FeatureToVersions::const_iterator i = feature_versions.find(feature_name);
    if (i==feature_versions.end()) return;
    CONSTANT VersionSet &vs=*(i->second);
    FOR_EACH (VersionSet::const_iterator,vi,vs)
    {
        CONSTANT Version version=*vi;
        if (evaluate(version, version_operator, operand))
            need_feature(PackageVersion(feature_name,version));
    }
}


void Encoder::need (InstallableUnit* unit)
{
    if (unit->needed) return;
    if (unit_count > cut_off) return;
    ++unit_count;
    OUT ( cerr <<  "need: " << (unit->name) << ":" << (unit->version) << endl; )
    unit->needed=true;
    unit->variable=variables.add_variable_package(PackageVersion (unit->name,unit->version));
    OUT( cerr<< "var: " << unit->variable << endl; )
    record_unit(needed_units,unit);
    need (unit->depends_cnf);
    if (process_recommends) need(unit->recommends_cnf);
    if (process_not_uptodate()) need_latest(unit->name);
}

void Encoder::need_latest(const string& package_name)
{
    PackageUnits::const_iterator index = read_units.find(package_name);
    if (index == read_units.end ()) return;        
    UnitVector &unit_vector=*(index->second);
    InstallableUnit* l=NULL;
    Version lv=0;
    FOR_EACH(UnitVector::const_iterator,ui,unit_vector)
    {
        InstallableUnit* u=*ui;
        if (lv<u->version) {
          lv=u->version;
          l=u;
        }
    }
    if (l!=NULL) need(l);
}

void Encoder::need (PackageVersionsCNF cnf) {
    FOR_EACH (PackageVersionsCNF::const_iterator,i,cnf)
      FOR_EACH (PackageVersionsList::const_iterator,vi, (**i)) need(*vi);
 }

void Encoder::need (CONSTANT PackageVersions& versions) {
    if (is_feature (versions.name()))
        need_features(versions.name(),versions.op(),versions.version());
    else
        need_packages(versions.name(),versions.op(),versions.version());
}

void Encoder::encode() {
    OUT ( cerr << "encode ()" << endl;  )
    refine_upgrade_request();
    
    // Compute needed units and features (slicing)
    slice();

    // top bound
    solver.set_top(get_top(lexicographic));

    //Sort versions
    InstallableUnitCmp cmp;
    FOR_EACH(PackageUnits::iterator,i,needed_units)
    {
        UnitVector &uv=*(i->second);
        sort(uv.begin(),uv.end(),cmp);
    }
    FOR_EACH(PackageToVersions::iterator,i,needed_feature_versions)
    {
        VersionVector &vv=*(i->second);
        sort(vv.begin(),vv.end());
    }

    ///  features
    encode_provides();

    // Package dependencies
    FOR_EACH(PackageUnits::const_iterator,i,needed_units)
    {
        CONSTANT UnitVector &uv = *(i->second);
        FOR_EACH(UnitVector::const_iterator,vi,uv)
        {
            CONSTANT InstallableUnit *unit = *vi;
            encode(unit);
        }
    }

    // Install
    OUT ( cerr << "install [" <<endl;  )
    FOR_EACH (PackageVersionsSet::const_iterator,i,install)
    {
        LiteralVector literals;
        request_to_clause(*i,literals);
        solver.output_clause(literals);
        if (opt_not_removed && !is_feature(i->name())) not_removed.add(i->name());
    }
    OUT ( cerr << "]" <<endl;  )
    // upgrade
    encode_upgrade ();
    // Remove
    encode_request_remove ();
    // preference
    encode_lexicographic();
    if (opt_not_removed) assert_not_removed();
    //interval variables
    process_interval_variables();
}

void Encoder::assert_not_removed()
{
    FOR_EACH( StringSet::const_iterator,ri,not_removed)
    {        
        const string& package_name=*ri;
        PackageUnits::const_iterator i = needed_units.find(package_name);
        if (i==needed_units.end()) continue;
        
        CONSTANT UnitVector *unit_vector=i->second;
        LiteralVector not_remove_clause;
        require_up_to_clause(package_name, 0, unit_vector, NULL, not_remove_clause);
        solver.output_clause(not_remove_clause);
    }
}


bool Encoder::is_any_installed_package(const UnitVector& unit_vector)
{
    bool any_installed = false;
    FOR_EACH (UnitVector::const_iterator, unit_index, unit_vector)
    {
        CONSTANT InstallableUnit* unit=*unit_index;
        if (unit->installed)
        {
            any_installed = true;
            break;
        }
    }
    return any_installed;
}

bool Encoder::is_installed_feature(const PackageVersion& feature)
{
    assert (is_feature(feature.name()));
    CONSTANT FeatureToUnits::const_iterator providers_index = feature_units.find(feature);
    if (providers_index==feature_units.end()) { assert(false); return false; }
    CONSTANT UnitVector& providers =*(providers_index->second);
    FOR_EACH (UnitVector::const_iterator, provider_index, providers)
    {
        CONSTANT InstallableUnit& provider =**provider_index;
        if (provider.installed) return true;
    }
    return false;
}

bool Encoder::find_latest_installed_version_in_original_problem(const string& name, Version& version)
{
    Version latest_installed = 0;
    bool any_installed = false;
    if (is_feature (name))
    {
        FeatureToVersions::const_iterator i = feature_versions.find(name);
        if (i!=feature_versions.end())
        {
            CONSTANT VersionSet& vs=*(i->second);
            FOR_EACH (VersionSet::const_iterator,vi,vs)
            {
                CONSTANT Version cv=*vi;
                if ((cv > latest_installed) && is_installed_feature (PackageVersion (name,cv)))
                {
                    any_installed = true;
                    latest_installed = cv;
                }
            }
        }
    } else {
        PackageUnits::const_iterator i=read_units.find(name);
        if (i!=read_units.end())
        {
            CONSTANT UnitVector& uv=*(i->second);
            for (int k=uv.size()-1;k>=0;--k)
            {
                CONSTANT Version cv=uv[k]->version;
                if ((cv>latest_installed) && uv[k]->installed)
                {
                    latest_installed = uv[k]->version;
                    any_installed=true;
                }
            }
        }
    }
    if (any_installed)
    {
        version = latest_installed;
        return true;
    } else return false;
}

void Encoder::refine_upgrade_request()
{
    OUT(cerr << "upgrade refine" << endl << " from: ";
        collection_printing::print(upgrade,cerr);
        cerr << endl;
       )

    PackageVersionsList refined_list;
    FOR_EACH (PackageVersionsList::const_iterator, versions_index, upgrade)
    {
        CONSTANT PackageVersions& versions = *versions_index;

        if (versions.op()!=VERSIONS_NONE)
        {
            refined_list.push_back(versions);
            continue;
        }

        Version latest_version;
        CONSTANT string& name=versions.name();
        if (find_latest_installed_version_in_original_problem(name,latest_version))
            refined_list.push_back(PackageVersions(name,VERSIONS_GREATER_EQUALS,latest_version));
        else
            refined_list.push_back(versions);
    }
    upgrade.clear();
    upgrade.insert(upgrade.end(),refined_list.begin(),refined_list.end());
    OUT(cerr << " to: ";
        collection_printing::print(upgrade,cerr);
        cerr << endl;
       )
}

bool Encoder::latest_installed_package_version (CONSTANT string &name,Version& latest_version)
{
    assert (!is_feature(name));
    PackageUnits::const_iterator i=needed_units.find(name);
    if (i==needed_units.end()) return false; //no versions of the package
    
    CONSTANT UnitVector& uv=*(i->second);
    for (int k=uv.size()-1;k>=0;--k)
    {
        if (uv[k]->installed)
        {
            latest_version = uv[k]->version;
            return true;
        }
    }
    return false; //no installed versions of the package
}

bool Encoder::latest_installed_feature_version (CONSTANT string &name,Version& version)
{
    assert (is_feature(name));
    PackageToVersions::const_iterator i = needed_feature_versions.find(name);
    if (i==needed_feature_versions.end()) return false;
    const VersionVector& vv=*(i->second);
    for (int k=vv.size()-1;k>=0;--k)
    {
        CONSTANT Version ver=vv[k];
        CONSTANT PackageVersion f(name,ver);
        if (is_installed_feature (f))
        {
            version = ver;
            return true;
        }
    }
    return false;
}

void Encoder::encode_upgrade()
{
    OUT(cerr << "upgrade () [" << endl;)
    // encode as the request semantics first,
    // this is needed if such requirement is stronger than the one given by upgrade semantics
    // TODO: possibly can be pruned
    FOR_EACH (PackageVersionsList::const_iterator,i,upgrade)
    {
        LiteralVector literals;
        request_to_clause(*i,literals);
        solver.output_clause(literals);
    }

    FOR_EACH (PackageVersionsList::const_iterator,index,upgrade)
    {
        CONSTANT PackageVersions& request =*index;
        CONSTANT string& name = request.name();
        if (is_feature (name))
        {
            Version latest_version;
            bool any_installed=latest_installed_feature_version(name,latest_version);
            if (any_installed)
            {
                solver.output_unary_clause (interval_vars.get_require_up(PackageVersion(name,latest_version)));
                atmost_1_feature(name,latest_version);
            }

            Version var;
            CONSTANT PackageVersion ipv(name,0);
            if (variables.find_variable(ipv,var))
            {
                solver.output_unary_clause(neg(var));
            }                
        } else {
            if (opt_not_removed) not_removed.add(name);
            Version latest_version = 0;          
            if (latest_installed_package_version(name, latest_version))
            {
                solver.output_unary_clause (interval_vars.get_require_up(PackageVersion(name,latest_version)));
                atmost_1(name,latest_version);
            }
        }
    }
    OUT(cerr << "]" << endl;)
}

void Encoder::atmost_1_feature (CONSTANT string& name, Version from)
{
    assert (is_feature (name));
    CONSTANT PackageToVersions::const_iterator i=needed_feature_versions.find(name);
    if (i==needed_feature_versions.end ()) return;
    CONSTANT VersionVector& vv=*(i->second);
    for (UINT i=0;i<vv.size();++i) {
        CONSTANT Version version_i = vv[i];
        CONSTANT Variable variable_i = get_package_version_variable (PackageVersion(name, version_i));
        if (version_i < from)
        {
            //disable versions less than {from}
            solver.output_unary_clause(neg(variable_i));
            continue;
        }
        for (UINT j=i+1;j<vv.size();++j) {
            CONSTANT Version version_j = vv[j];
            assert (version_j >= from);
            CONSTANT Variable variable_j = get_package_version_variable (PackageVersion(name, version_j));
            solver.output_binary_clause(neg(variable_i), neg(variable_j));
        }
    }
}

void Encoder::atmost_1(CONSTANT string& package_name, Version from)
{
    assert (!is_feature(package_name));
    PackageUnits::const_iterator fi=needed_units.find(package_name);
    if (fi==needed_units.end()) return;
    CONSTANT UnitVector& vv=*(fi->second);
    for (UINT i=0;i<vv.size();++i) {
        CONSTANT InstallableUnit& ui=*(vv[i]);
        if (ui.version < from)
        {
            //disable versions less than {from}
            solver.output_unary_clause(neg(ui.variable));
            continue; 
        }
        for (UINT j=i+1;j<vv.size();++j) {
            CONSTANT InstallableUnit& uj=*(vv[j]);
            assert (uj.version >= from);
            solver.output_binary_clause(neg(ui.variable), neg(uj.variable));
        }
    }
}

void Encoder::encode_request_remove()
{
    OUT(cerr << "request_remove ()" << endl;)

    PackageVersionsList::const_iterator index = remove.begin();
    CONSTANT PackageVersionsList::const_iterator e = remove.end();
    for (; index != e; ++index) {
        const PackageVersions& pvs = *index;
        LiteralVector term;
        disable_to_term(pvs,term);     
        FOR_EACH(LiteralVector::const_iterator, i, term)
           solver.output_unary_clause(*i);
    }
}

void Encoder::encode_new(XLINT& total_weight,bool maximize)
{
    OUT(cerr<< "new[" << endl;)
    const XLINT new_weight=total_weight+1;
    FOR_EACH(PackageUnits::const_iterator,index,needed_units)
    {
        CONSTANT UnitVector& units=*(index->second);
        if (units.empty() || is_any_installed_package(units)) continue;
        const string& package_name=index->first;
        CONSTANT PackageVersion first_version(package_name, units[0]->version);
        if (maximize) {//maximization requires at least one version to be installed
            solver.output_unary_weighted_clause(
                                            interval_vars.get_require_up(first_version),
                                            new_weight);

        } else {//minimization requires all of them to be uninstalled
                    solver.output_unary_weighted_clause(
                                            interval_vars.get_disable_up(first_version),
                                            new_weight);
        }
        total_weight+=new_weight;
    }
    OUT(cerr<< "]";)
}

void Encoder::encode_removed(XLINT& total_weight,bool maximize)
{
    OUT(cerr<< "removed [" << endl;)
    assert(solver.get_soft_clauses_weight()==total_weight);
    CONSTANT XLINT not_rm_weight=total_weight+1;
    FOR_EACH(PackageUnits::const_iterator,index,needed_units)
    {
        CONSTANT UnitVector& unit_vector=*(index->second);
        const string& package_name=index->first;

        bool any_installed = is_any_installed_package(unit_vector);
        if (!any_installed) continue;//if nothing was installed, it cannot be removed
        if (opt_not_removed && not_removed.is_not_removed(package_name)) continue; //package detected unremovable

        if (maximize) {
            // disable all versions of this package
            Variable v=interval_vars.get_disable_up(PackageVersion(package_name,unit_vector[0]->version));
            solver.output_unary_weighted_clause(v, not_rm_weight);
        } else {
            // minimization---require at least one version of this package
            LiteralVector ls;            
            require_up_to_clause(package_name,0,index->second,NULL,ls);
            solver.output_weighted_clause(ls, not_rm_weight);            
        }
        total_weight+=not_rm_weight;
    }
    OUT(cerr<< "]" << endl;)
}

void Encoder::encode_unmet_recommends(XLINT& total_weight,bool maximize)
{
    OUT(cerr<< "recommends[" << endl;)
    CONSTANT XLINT recommends_weight=total_weight+1;
    assert(solver.get_soft_clauses_weight()==total_weight);
    if (maximize) {
       //when  maximizing made the clauses of unsat
        FOR_EACH(PackageUnits::const_iterator,index,needed_units)
        {
            CONSTANT UnitVector& units=*(index->second);
            FOR_EACH (UnitVector::const_iterator, unit_index, units)
            {
                InstallableUnit *unit =*unit_index;
                if (unit->recommends_cnf.empty()) continue;
                FOR_EACH (PackageVersionsCNF::const_iterator, clause_index, unit->recommends_cnf)
                {
                    PackageVersionsList& clause =**clause_index;
                    Variable is_broken = id_manager.new_id();
                    //encode the condition for the clauses to be broken
                    FOR_EACH (PackageVersionsList::const_iterator, literal_index, clause)
                    {
                        CONSTANT PackageVersions literal =*literal_index;
                        vector<LINT> term;
                        disable_to_term(literal, term);
                        FOR_EACH (vector<LINT>::const_iterator,t, term)
                           solver.output_binary_clause(neg(is_broken),*t);
                    }
                    Variable v = id_manager.new_id();
                    solver.output_binary_clause(neg(v), is_broken);
                    solver.output_binary_clause(neg(v), unit->variable);
                    solver.output_unary_weighted_clause(v,recommends_weight);//we are getting a point if the unit is installed and the recommends is broken
                }
            }
        }
    } else {
        //when minimizing try to satisfy the clauses
        FOR_EACH(vector<ClausePointer>::const_iterator,i,recommends_clauses_pointers)
        {
            ClausePointer cp=*i;
            solver.increase_weight(cp, recommends_weight);
            total_weight+=recommends_weight;
        }
    }
    OUT(cerr<< "]";)
}

void Encoder::encode_not_up_to_date(XLINT& total_weight,bool maximize)
{
    OUT(cerr<< "not up to date[" << endl;)
    assert(solver.get_soft_clauses_weight()==total_weight);
    CONSTANT XLINT up_to_date_weight=total_weight+1;
    FOR_EACH(PackageUnits::const_iterator,index,needed_units)
    {
        CONSTANT UnitVector& units=*(index->second);
        if (units.size() <= 1) continue; // If only one version of the package exists, the criterion  is  satisfied automatically
        CONSTANT InstallableUnit*  latest_version = units[units.size()-1];
        CONSTANT Variable v_latest = latest_version->variable;
        if (opt_not_removed && not_removed.is_not_removed(index->first))
        {//at least one version of this package will be installed
            if (maximize) {//disable the latest version
                solver.output_unary_weighted_clause(neg(v_latest), up_to_date_weight);
            } else {//require the latest version
                solver.output_unary_weighted_clause(v_latest, up_to_date_weight);
            }
        }
        else {
            CONSTANT Variable any_will_be_installed = new_variable();
            vector<LINT> literals;
            // Mikolas' encoding (upper bound of number of notuptodate packages):
            /*
            if (maximize) literals.push_back(neg(any_will_be_installed));
            for (UINT vi=0;vi<units.size()-1;++vi)
            {
                CONSTANT Variable vv=units[vi]->variable;
                if (maximize) literals.push_back(vv);
                else solver.output_binary_clause(neg(vv),any_will_be_installed);
            }
             if (maximize) {//require that at least one version is installed and the last one isn't
                 Variable auxiliary = id_manager.new_id();
                 solver.output_clause(literals);
                 solver.output_binary_clause(neg(auxiliary), any_will_be_installed);
                 solver.output_binary_clause(neg(auxiliary), neg(v_latest));
                 solver.output_unary_weighted_clause(auxiliary, up_to_date_weight);
             } else { //if any version is installed, then the latest one should be installed as well
                 solver.output_binary_weighted_clause(neg(any_will_be_installed), v_latest, up_to_date_weight);
             }
             */
             // Miguel's encoding (exactly the number of notuptodate packages):
             // encode any_will_be_installed (it is equivalent to the disjunction of versions)
             vector<LINT> implication1;
            implication1.push_back(neg(any_will_be_installed));
            for (UINT vi=0;vi<units.size();++vi)
            {
                CONSTANT Variable vv=units[vi]->variable;
                implication1.push_back(vv);
                vector<LINT> implication2 {any_will_be_installed, neg(vv)};
                solver.output_clause(implication2);
            }
            solver.output_clause(implication1);
            // encode package is notuptodate (any_will_be_installed and neg(last version))
            const Variable notuptodate = new_variable();
            {
                vector<LINT> implication1 {notuptodate, neg(any_will_be_installed), v_latest};
                solver.output_clause(implication1);
            }
            // other implication
            solver.output_binary_clause(neg(notuptodate), any_will_be_installed);
            solver.output_binary_clause(neg(notuptodate), neg(v_latest));
            if (maximize) solver.output_unary_weighted_clause(notuptodate, up_to_date_weight);
            else solver.output_unary_weighted_clause(neg(notuptodate), up_to_date_weight);
            
        }
        total_weight+=up_to_date_weight;
    }
    OUT(cerr<< "]";)
}

void Encoder::encode_changed(XLINT& total_weight,bool maximize)
{
    CONSTANT XLINT w=total_weight+1;
    vector<LINT> literals;
    FOR_EACH(PackageUnits::const_iterator,index,needed_units)
    {
        CONSTANT UnitVector &unit_vector=*(index->second);
        // Mikolas' encoding: the soft clauses are not exactly the quantity we want to optimise
        /*
        Variable unchanged=-1; // variables that represents that all versions stayed the same, used only for minimization
        if (!maximize) {
           unchanged=id_manager.new_id();
#ifdef CONV_DBG
        string nm="unchanged_" + index->first;
        variable_names[unchanged] = nm;
        mapping << unchanged << "->" << nm << endl;
#endif                 
        } else {
            literals.clear();
        }
        FOR_EACH (UnitVector::const_iterator, unit_index, unit_vector)
        {
            CONSTANT InstallableUnit& unit =**unit_index;
            LINT l = unit.installed ? unit.variable : neg(unit.variable);
            if (!maximize) solver.output_binary_clause(neg(unchanged),l); // each version needs to stay the same
            else literals.push_back(-l);//at least one version changed
        }
        if (!maximize) solver.output_unary_weighted_clause(unchanged,w);
        else solver.output_weighted_clause(literals,w);
        */
        // Miguel's encoding: The objective function is EXACTLY the number of changed packages
        const Variable unchanged=id_manager.new_id();
#ifdef CONV_DBG
        string nm="unchanged_" + index->first;
        variable_names[unchanged] = nm;
        mapping << unchanged << "->" << nm << endl;
#endif
        // changed is equivalent to the disjunction of literals
        literals.clear();
        // each literal i means version i changed
        FOR_EACH (UnitVector::const_iterator, unit_index, unit_vector)
        {
            CONSTANT InstallableUnit& unit =**unit_index;
            LINT l = unit.installed ? neg(unit.variable) : unit.variable;
            literals.push_back(l);
            // unchanged implies negation of disjunction
            solver.output_binary_clause(neg(unchanged),neg(l));
        }
        literals.push_back(unchanged);
        // changed implies disjunction
        solver.output_clause(literals);
        solver.output_unary_weighted_clause( maximize ? neg(unchanged) : unchanged, w);
        total_weight+=w;
    }
}

XLINT Encoder::get_top(CONSTANT Objective& objective) const
{
    switch (objective.first)
    {
        case COUNT_NEW:
            return needed_units.size();
        case COUNT_UNMET_RECOMMENDS:
        {
            UINT recommend_cl_counter=0;
            FOR_EACH(PackageUnits::const_iterator,index,needed_units)
            {
                CONSTANT UnitVector& units=*(index->second);
                for (UINT vi=0;vi<units.size();++vi)
                    recommend_cl_counter+=units[vi]->recommends_cnf.size();
           }
            return recommend_cl_counter;
        }
        case COUNT_REMOVED:
            return needed_units.size();
        case COUNT_NOT_UP_TO_DATE:
            return needed_units.size();
        case COUNT_CHANGED:
            return needed_units.size();
    }
    assert (false);
    return 0;
}

XLINT Encoder::get_top(const vector<Objective>& lexicographic) const
{
    XLINT total_weight = 0;
    FOR_EACH(vector<Objective>::const_iterator,objective_index,lexicographic)
    {
        CONSTANT Objective objective = *objective_index;
        total_weight += (total_weight + 1) * get_top(objective);
    }
    return total_weight + 1;
}

void Encoder::encode_lexicographic() {
    XLINT total_weight = 0;
    //FOR_EACH_REV(vector<Objective>::const_reverse_iterator,objective_index,lexicographic)
    for (size_t i=lexicographic.size(); i>0; --i) {
        CONSTANT Objective objective = lexicographic[i-1];
        solver.register_weight(total_weight+1);
        switch (objective.first)
        {
            case COUNT_NEW:
                encode_new(total_weight, objective.second);
                break;
            case COUNT_UNMET_RECOMMENDS:              
                encode_unmet_recommends(total_weight, objective.second);
                break;
            case COUNT_REMOVED:
                encode_removed(total_weight, objective.second);
                break;
            case COUNT_NOT_UP_TO_DATE:
                encode_not_up_to_date(total_weight, objective.second);
                break;
            case COUNT_CHANGED:
                encode_changed(total_weight, objective.second);
                break;
        }        
    }
}

void Encoder::encode_provides()
{
    FOR_EACH (PackageToVersions::const_iterator, versions_index, needed_feature_versions)
    {
        CONSTANT VersionVector& vs = *(versions_index->second);
        CONSTANT string& name= versions_index->first;
        Variable infinite_variable=-1;
        bool infinite_provided = variables.find_variable(PackageVersion (name, 0), infinite_variable);
        FOR_EACH (VersionVector::const_iterator,version_index,vs)
        {
            CONSTANT Version ver =*version_index;
            CONSTANT PackageVersion pv(name,ver);
            CONSTANT Variable var=get_package_version_variable (pv);
            LiteralVector literals;
            literals.push_back(neg(var));
            if (infinite_provided && ver != 0)
            {
                solver.output_binary_clause (neg(infinite_variable),var);
                literals.push_back(infinite_variable);
            }
            CONSTANT FeatureToUnits::const_iterator providers_index = feature_units.find(pv);
            if (providers_index!= feature_units.end())
            {
                CONSTANT UnitVector& providers =*(providers_index->second);
                FOR_EACH (UnitVector::const_iterator, provider_index, providers)
                {
                    CONSTANT InstallableUnit& provider =**provider_index;
                    CONSTANT Variable provider_variable=provider.variable;
                    if (!provider.needed) continue;
                    solver.output_binary_clause (neg (provider_variable),var);
                    literals.push_back(provider_variable);
                }
            }
            solver.output_clause (literals);
        }
    }
}

void Encoder::encode(const InstallableUnit* unit)
{
    OUT ( cerr << "encode " << unit->name  << " " << unit->version <<"[" <<endl;  )
    encode_depends (unit);
    encode_conflicts (unit);
    if (process_recommends) encode_recommends (unit);
    OUT ( cerr << "]" <<endl;  )
}

void Encoder::encode_recommends(const InstallableUnit* unit)
{
    CONSTANT PackageVersionsCNF& depends = unit->recommends_cnf;
    FOR_EACH (PackageVersionsCNF::const_iterator,ci,depends)
    {
       CONSTANT PackageVersionsList* clause=*ci;
       CONSTANT PackageVersionsList::const_iterator e=clause->end();
       CONSTANT PackageVersionsList::const_iterator b=clause->begin();
       LiteralVector literals;
       literals.push_back(neg(unit->variable));
       requests_to_clause(b,e, literals);
       ClausePointer p=solver.record_clause(literals);
       recommends_clauses_pointers.push_back(p);
    }
}

void Encoder::encode_depends(const InstallableUnit* unit)
{
    CONSTANT PackageVersionsCNF& depends = unit->depends_cnf;
    FOR_EACH (PackageVersionsCNF::const_iterator,ci,depends)
    {
       CONSTANT PackageVersionsList* clause=*ci;
       CONSTANT PackageVersionsList::const_iterator e=clause->end();
       CONSTANT PackageVersionsList::const_iterator b=clause->begin();
       LiteralVector literals;
       literals.push_back(neg(unit->variable));
       requests_to_clause(b,e, literals);
       solver.output_clause(literals);
    }
}

void Encoder::encode_conflicts(const InstallableUnit* unit)
{
    OUT (cerr<< "local conflicts[" <<endl;)
    assert (unit->version > 0);
    assert (unit->needed);
    FOR_EACH (PackageVersionsList::const_iterator, index, unit->conflicts)
    {
        CONSTANT PackageVersions pvs=*index;
        CONSTANT bool pvs_is_feature = is_feature(pvs.name());

        if (pvs_is_feature)
        {
            bool self_feature_conflict = false;
            FOR_EACH (PackageVersionList::const_iterator,feature_index,unit->provides)
            {
                CONSTANT PackageVersion& feature =*feature_index;
                if (SAME_PACKAGE_NAME(pvs.name(),feature.name()))
                {
                    self_feature_conflict = true;
                    break;
                }
            }
            if (self_feature_conflict)
            {
                VariableSet providers;
                add_providers(pvs,providers);
                providers.erase(unit->variable);
                conflict_all (unit->variable,providers);
                continue;
            }
            // otherwise treat in not_self_conflict

        }

        if (SAME_PACKAGE_NAME(pvs.name(),unit->name))
        {
            encode_package_self_conflict(unit,pvs);
            continue;
        }

        encode_not_self_conflict (unit,pvs);
    }


    OUT (cerr<< "]"<<endl;)
}

void Encoder::encode_package_self_conflict(const InstallableUnit* unit, const PackageVersions& pvs)
{
    PackageUnits::const_iterator i = needed_units.find(unit->name);
    CONSTANT UnitVector& units = *(i->second);

    if (pvs.op()==VERSIONS_NONE)
    {
        LiteralVector term;
        disable_up_to_term(unit->name, unit->version+1, i->second, NULL, term);
        disable_down_to_term(unit->name, unit->version-1, i->second, NULL, term);
        FOR_EACH(LiteralVector::const_iterator, i, term)
           solver.output_binary_clause(neg(unit->variable),*i);
        return;
    }

    FOR_EACH (UnitVector::const_iterator, unit_index, units)
    {
        InstallableUnit* conflicting_unit = *unit_index;
        if (conflicting_unit->version==unit->version) continue;
        if (evaluate(conflicting_unit->version,pvs.op(),pvs.version()))
            solver.output_binary_clause (neg(unit->variable),neg(conflicting_unit->variable));
    }
}

void Encoder::disable_to_term(const PackageVersions& pvs, LiteralVector& term)
{
   CONSTANT string& name = pvs.name();
   //Check whether it is a feature or not
   CONSTANT PackageToVersions::const_iterator fi = needed_feature_versions.find(name);
   CONSTANT bool is_feature = fi!=needed_feature_versions.end();

   VersionVector* version_vector = NULL;
   UnitVector *unit_vector = NULL;
   if (is_feature)
   {
      version_vector = fi->second;
   } else
   {   //If it is not a feature, look up the pertaining units
       CONSTANT PackageUnits::const_iterator pi = needed_units.find(name);
       if (pi==needed_units.end())
           return;//there is no feature or package of that name

        unit_vector = pi->second;
   }

   UINT old_sz=term.size();
    switch (pvs.op()) {
        case VERSIONS_NONE:
            disable_up_to_term(name, 0, unit_vector, version_vector, term);
            break;
        case VERSIONS_EQUALS:
        {
            CONSTANT PackageVersion pv=PackageVersion(pvs.name(), pvs.version());
            Variable v;
            if (variables.find_variable(pv,v))
            {   // generated only if the exact version exists
                term.push_back(neg(v));
            }
        } break;
        case VERSIONS_NOT_EQUALS:
            disable_up_to_term(name, pvs.version()+1, unit_vector, version_vector, term);
            disable_down_to_term(name, pvs.version()-1, unit_vector, version_vector, term);
            break;
        case VERSIONS_GREATER_EQUALS:
            disable_up_to_term(name, pvs.version(), unit_vector, version_vector, term);
            break;
        case VERSIONS_GREATER:
            disable_up_to_term(name, pvs.version()+1, unit_vector, version_vector, term);
            break;
        case VERSIONS_LESS_EQUALS:
            disable_down_to_term(name, pvs.version(), unit_vector, version_vector, term);
            break;
        case VERSIONS_LESS:
            disable_down_to_term(name, pvs.version()-1, unit_vector, version_vector, term);
    }
    assert (term.size() >= old_sz);
    bool generated = old_sz!=term.size();
    if (is_feature && !generated)
    {
        CONSTANT PackageVersion pv(pvs.name(), 0);
        Variable v;
        if (variables.find_variable(pv,v))
          term.push_back(neg(v));
    }
}

void Encoder::encode_not_self_conflict(const InstallableUnit* unit, const PackageVersions& pvs)
{
    LiteralVector term;
    disable_to_term(pvs,term);
    CONSTANT Variable current_variable = unit->variable;
    FOR_EACH(LiteralVector::const_iterator, i, term)
       solver.output_binary_clause(neg(current_variable),*i);
}

void Encoder::requests_to_clause (CONSTANT PackageVersionsList::const_iterator begin,
                                   CONSTANT PackageVersionsList::const_iterator end,
                                   LiteralVector& literals)
{
   for (PackageVersionsList::const_iterator i=begin;i!=end; ++i)
   {
       CONSTANT PackageVersions& pvs=*i;
       request_to_clause (pvs, literals);
   }
}

bool Encoder::require_up_to_clause(const string& name, Version from,
        const UnitVector* unit_vector, const VersionVector* version_vector,
        LiteralVector& literals)
{
    assert ((version_vector != NULL) ^ (unit_vector != NULL));
    CONSTANT bool is_feature = version_vector != NULL;
    if (is_feature)
    {
         int pos;
         UINT count=0;
         for (pos=version_vector->size() - 1; pos >= 0 && (version_vector->at(pos) >= from); --pos)
            ++count;    
         if (count==0) return false;//no versions greater or equal to from
         if (count > iv) {// use interval variable
           literals.push_back(interval_vars.get_require_up(PackageVersion(name,version_vector->at(pos+1))));
         } else {
             // use directly the variables of the feature
             for (UINT i=pos+1;i<version_vector->size();++i)
             {
                 CONSTANT Version version=version_vector->at(i);
                 Variable var=-1;
                 const bool f=variables.find_variable(PackageVersion (name, version), var);
                 if (!f) assert(0);
                 literals.push_back(var);
             }
        }
    } else {
        int pos;
        UINT count=0;
        for (pos=unit_vector->size() - 1; pos >= 0 && (unit_vector->at(pos)->version >= from); --pos)
            ++count;
        if (count==0) return false; //no versions greater or equal to from
        if (count > iv) { // use interval variable
           literals.push_back(interval_vars.get_require_up(PackageVersion(name,unit_vector->at(pos+1)->version)));
        } else { // use directly the variables of the unit
             for (UINT i=pos+1;i<unit_vector->size();++i)
                 literals.push_back(unit_vector->at(i)->variable);
        }
    }
    return true;
}


bool Encoder::disable_up_to_term(const string& name, Version from,
        const UnitVector* unit_vector, const VersionVector* version_vector,
        LiteralVector& literals)
{
    assert ((version_vector != NULL) ^ (unit_vector != NULL));
    CONSTANT bool is_feature = version_vector != NULL;
    if (is_feature)
    {
         int pos;
         UINT count=0;
         for (pos=version_vector->size() - 1; pos >= 0 && (version_vector->at(pos) >= from); --pos)
            ++count;
         if (count==0) return false;//no versions greater or equal to from
         if (count > iv) {// use interval variable
           literals.push_back(interval_vars.get_disable_up(PackageVersion(name,version_vector->at(pos+1))));
         } else {
             // use directly the variables of the feature
             for (UINT i=pos+1;i<version_vector->size();++i)
             {
                 CONSTANT Version version=version_vector->at(i);
                 Variable var=-1;
                 bool f=variables.find_variable(PackageVersion (name, version), var);
                 if (!f) assert(0);
                 literals.push_back(neg(var));
             }
        }
    } else {
        int pos;
        UINT count=0;
        for (pos=unit_vector->size() - 1; pos >= 0 && (unit_vector->at(pos)->version >= from); --pos)
            ++count;
        if (count==0) return false; //no versions greater or equal to from
        if (count > iv) { // use interval variable
           literals.push_back(interval_vars.get_disable_up(PackageVersion(name,unit_vector->at(pos+1)->version)));
        } else { // use directly the variables of the unit
             for (UINT i=pos+1;i<unit_vector->size();++i)
                 literals.push_back(neg(unit_vector->at(i)->variable));
        }
    }
    return true;
}

bool Encoder::require_down_to_clause(const string& name, Version from,
        const UnitVector* unit_vector, const VersionVector* version_vector,
        LiteralVector& literals)
{
    assert ((version_vector != NULL) ^ (unit_vector != NULL));
    CONSTANT bool is_feature = version_vector != NULL;
    if (is_feature)
    {        
         UINT pos;
         UINT count=0;
         for (pos=0; (pos < version_vector->size()) && (version_vector->at(pos) <= from); ++pos)
           ++count;
         if (count==0) return false;//no versions greater or equal to from
         assert(pos>0);
         if (count > iv) {// use interval variable
           literals.push_back(interval_vars.get_require_down(PackageVersion(name,version_vector->at(pos-1))));
         } else {
             // use directly the variables of the feature
             for (int i=pos-1;i>=0;--i)
             {  
                 CONSTANT Version version=version_vector->at(i);
                 Variable var=-1;
                 bool f=variables.find_variable(PackageVersion (name, version), var);
                 if (!f) assert(0);
                 literals.push_back(var);
             }
        }
    } else {
        UINT pos;
        UINT count=0;
        for (pos=0; (pos < unit_vector->size()) && (unit_vector->at(pos)->version <= from); ++pos)
            ++count;
        if (count==0) return false; //no versions greater or equal to from
        assert(pos>0);
        if (count > iv) { // use interval variable
           literals.push_back(interval_vars.get_require_down(PackageVersion(name,unit_vector->at(pos-1)->version)));
        } else { // use directly the variables of the unit
             for (int i=pos-1;i>=0;--i)
                 literals.push_back(unit_vector->at(i)->variable);
        }
    }
    return true;
}


bool Encoder::disable_down_to_term(const string& name, Version from,
        const UnitVector* unit_vector, const VersionVector* version_vector,
        LiteralVector& literals)
{
    assert ((version_vector != NULL) ^ (unit_vector != NULL));
    CONSTANT bool is_feature = version_vector != NULL;
    if (is_feature)
    {
         UINT pos;
         UINT count=0;
         for (pos=0; (pos < version_vector->size()) && (version_vector->at(pos) <= from); ++pos)
           ++count;
         if (count==0) return false;//no versions greater or equal to from
         assert(pos>0);
         if (count > iv) {// use interval variable
           literals.push_back(interval_vars.get_disable_down(PackageVersion(name,version_vector->at(pos-1))));
         } else {
             // use directly the variables of the feature
             for (int i=pos-1;i>=0;--i)
             {
                 CONSTANT Version version=version_vector->at(i);
                 Variable var=-1;
                 bool f=variables.find_variable(PackageVersion (name, version), var);
                 if (!f) assert(0);
                 literals.push_back(neg(var));
             }
        }
    } else {
        UINT pos;
        UINT count=0;
        for (pos=0; (pos < unit_vector->size()) && (unit_vector->at(pos)->version <= from); ++pos)
            ++count;
        if (count==0) return false; //no versions greater or equal to from
        assert(pos>0);
        if (count > iv) { // use interval variable
           literals.push_back(interval_vars.get_disable_down(PackageVersion(name,unit_vector->at(pos-1)->version)));
        } else { // use directly the variables of the unit
             for (int i=pos-1;i>=0;--i)
                 literals.push_back(neg(unit_vector->at(i)->variable));
        }
    }
    return true;
}


void Encoder::request_to_clause (CONSTANT PackageVersions& pvs,LiteralVector& literals) {
   CONSTANT string& name = pvs.name();
   UnitVector *unit_vector = NULL;
   VersionVector *version_vector = NULL;
   //Check whether it is a feature or not
   CONSTANT PackageToVersions::const_iterator fi = needed_feature_versions.find(name);
   CONSTANT bool is_feature = fi!=needed_feature_versions.end();

   if (is_feature)
   {
      version_vector = fi->second;
   } else
   {   //if it is not a feature, look up the pertaining installable units
       CONSTANT PackageUnits::const_iterator pi = needed_units.find(name);
       if (pi==needed_units.end())
           return;//there is no feature or package of that name
       unit_vector = pi->second;
   }

   bool generated=false;
#ifndef NDEBUG
   const size_t old_sz=literals.size();
#endif
   switch (pvs.op()) {
       case VERSIONS_NONE:
            generated = require_up_to_clause(name, 0, unit_vector, version_vector, literals);
        break;
        case VERSIONS_EQUALS: {
            CONSTANT PackageVersion pv=PackageVersion(name,pvs.version());
            Variable variable;
            if (variables.find_variable(pv,variable)) {
               generated = true;
               literals.push_back(variable);// insert the requirement only if such version exists
            }
        }
        break;
        case VERSIONS_NOT_EQUALS: {
            const bool ug=require_up_to_clause(name, pvs.version()+1, unit_vector, version_vector, literals);
            const bool dg=require_down_to_clause(name, pvs.version()-1, unit_vector, version_vector, literals);
            generated=ug||dg;
        }
        break;
        case VERSIONS_GREATER_EQUALS:
             generated = require_up_to_clause(name, pvs.version(), unit_vector, version_vector, literals);
        break;
        case VERSIONS_GREATER:
             generated = require_up_to_clause(name, pvs.version()+1, unit_vector, version_vector, literals);
             break;
        case VERSIONS_LESS_EQUALS:
             generated = require_down_to_clause(name, pvs.version(), unit_vector, version_vector, literals);
             break;
        case VERSIONS_LESS:
            generated = require_down_to_clause(name, pvs.version()-1, unit_vector, version_vector, literals);
   }
   assert (literals.size() >= old_sz);
   assert (generated == (old_sz!=literals.size()));
   if (is_feature && !generated) {
       //if infinite version provided, use that as an option
       Variable variable=-1;
       if (variables.find_variable(PackageVersion(name,0), variable))
           literals.push_back(variable);//if infinite version provided, use that as an option
   }
}

void Encoder::print_time()
{
    double parent_time (RUSAGE::read_cpu_time_self());
    double children_time (RUSAGE::read_cpu_time_children());
    double total_time (parent_time + children_time);
    std::cerr << "# solving time: ";
    //std::cerr << "#\t parent process: " << parent_time << "s\n";
    //std::cerr << "#\t child processes: " << children_time << "s\n";
    std::cerr /*<< "#\t total time: " */ << total_time << "s\n";
}

bool Encoder::solution() {
    CNF_OUT(cout << "p wcnf " << id_manager.top_id() << " " << solver.get_clause_count() << " " << (solver.get_soft_clauses_weight() + 1) << endl;)
    if (!should_solve) {
        cerr << "not solving" << endl;
        solver.dump(cout);
        return false;
    } else if (cs) {
        cerr << "not solving,cs" << endl;
        check_solution();
        return false;
    }
    cerr << "# solving " << endl;
    const bool has_solution = solver.solve();
    print_time();
    print_solution();
    return has_solution;
}

void Encoder::print_solution() {
    if (printing_started_or_finished) return;
    printing_started_or_finished = true;
    ostream& solution_file((psolution_file==NULL) ? cout : *psolution_file);
    cerr << "# sol printing" << endl;
    const bool has_solution = solver.has_solution();
    if (has_solution) {
        solution_file << "# beginning of solution from packup" << endl;
        CONSTANT IntVector& solution = solver.get_model();
        OUT(cerr << "solution:"; printer.print_solution(solution, cerr, true); cerr << endl;)
        /*if (valuation) {
            print_valuation(cerr, solver.get_model());
            cerr << endl;
            cerr << "MSUNCore min unsat: " << solver.get_min_unsat_cost() << endl;
        }*/
        print_valuation(cerr, solver.get_model());
        cerr << endl;

        FOR_EACH(PackageUnits::const_iterator, i, needed_units) {
            CONSTANT UnitVector &vs = *(i->second);
            FOR_EACH(UnitVector::const_iterator, vi, vs) {
                CONSTANT InstallableUnit& unit = **vi;
                CONSTANT Variable pvv = unit.variable;
                assert(pvv < solution.size());
                if (solution[pvv] > 0) {
                    solution_file << "package: " << unit.name << endl;
                    solution_file << "version: " << unit.version << endl;
                    solution_file << "installed: true" << endl;
                    solution_file << endl;
                }
            }
        }
        solution_file << "# end of solution from packup" << endl;
    } else {
        cerr << "no solution" << endl;
        solution_file << "FAIL" << endl << "no solution (UNSAT)" << endl;
    }
    cerr.flush();
    solution_file.flush();
    cerr << "# sol printed" << endl;
}

void Encoder::conflict_all(CONSTANT Variable& pv, CONSTANT VariableSet& set) {
    FOR_EACH(VariableSet::const_iterator, index, set) {
        Variable v = *index;
        solver.output_binary_clause(neg(pv), neg(v));
    }
}

void Encoder::add_providers(CONSTANT PackageVersion& feature, VariableSet& providers) {
    // Add all providers of the non-versioned feature
    CONSTANT FeatureToUnits::const_iterator iii = feature_units.find(feature);
    if (iii != feature_units.end()) {
        CONSTANT UnitVector& units = *(iii->second);

        FOR_EACH(UnitVector::const_iterator, ui, units) {
            InstallableUnit *unit = *ui;
            if (unit->needed) {
                Variable v=-1;
                const bool f = variables.find_variable(PackageVersion(unit->name, unit->version), v);
                if (!f) assert(0);
                providers.insert(v);
            }
        }

    }
}

void Encoder::add_providers(CONSTANT PackageVersions& features, VariableSet& providers) {
    // Add all providers of the non-versioned feature
    add_providers(PackageVersion(features.name(), 0), providers);

    // Look at all the features corresponding to {@code features} and had their providers to the set
    PackageToVersions::const_iterator i = needed_feature_versions.find(features.name());
    if (i == needed_feature_versions.end()) return;
    CONSTANT VersionVector& vs = *(i->second);

    FOR_EACH(VersionVector::const_iterator, index, vs) {
        CONSTANT Version version = *index;
        if (evaluate(version, features.op(), features.version())) {
            add_providers(PackageVersion(features.name(), version), providers);
        }
    }
}

void to_versions(CONSTANT UnitVector& units, VersionVector& versions) {
    FOR_EACH(UnitVector::const_iterator, i, units)
    versions.push_back((*i)->version);
}

void Encoder::process_interval_variables ()
{
    OUT (cerr << "Interval variables[" << endl;)

    CONSTANT PackageToVersions& ru=interval_vars.get_require_up ();
    CONSTANT PackageToVersions& rd=interval_vars.get_require_down();    
    CONSTANT PackageToVersions& du=interval_vars.get_disable_up();
    CONSTANT PackageToVersions& dd=interval_vars.get_disable_down();

    CONSTANT VersionVector empty_vector;
    FOR_EACH (PackageToVersions::const_iterator, index, ru)
    {        
        CONSTANT string name = index->first; 
        VersionVector& intervals=*(index->second); 
        sort (intervals.begin(),intervals.end());
        OUT (  cerr << "intervals " << name;
               FOR_EACH (VersionVector::const_iterator,i,intervals) cerr << " " << *i;
               cerr<<endl;
            )

        if (is_feature(name))
        {
            PackageToVersions::const_iterator i=needed_feature_versions.find (name);
            CONSTANT VersionVector& vers = (i!= needed_feature_versions.end()) ? *(i->second)  : empty_vector ;
            process_require_up(name, intervals, vers, true);
        } else {
             PackageUnits::const_iterator i = needed_units.find(name);
             assert (i!= needed_units.end ());
             VersionVector versions;
             to_versions(*(i->second), versions);
             process_require_up(name, intervals, versions, false);
        }
    }

    FOR_EACH (PackageToVersions::const_iterator, index, rd)
    {
        CONSTANT string name = index->first;
        VersionVector& intervals=*(index->second);
        sort (intervals.begin(),intervals.end());
        OUT (  cerr << "intervals " << name;
               FOR_EACH (VersionVector::const_iterator,i,intervals) cerr << " " << *i; cerr<<endl;
            )
        

        if (is_feature(name))
        {
            PackageToVersions::const_iterator i=needed_feature_versions.find (name);
            CONSTANT VersionVector& vers = (i!= needed_feature_versions.end()) ? *(i->second)  : empty_vector ;
            process_require_down(name, intervals, vers, true);
        } else {
             PackageUnits::const_iterator i = needed_units.find(name);             
             VersionVector versions;
             if (i!=needed_units.end ()) to_versions(*(i->second), versions);
             process_require_down(name, intervals, versions, false);
        }
    }
    FOR_EACH (PackageToVersions::const_iterator, index, du)
    {
        CONSTANT string name = index->first;
        VersionVector& intervals=*(index->second);
        sort (intervals.begin(),intervals.end());
        OUT (  cerr << "intervals " << name;
               FOR_EACH (VersionVector::const_iterator,i,intervals) cerr << " " << *i;
               cerr<<endl;
            )

        

        if (is_feature(name))
        {
            PackageToVersions::const_iterator i=needed_feature_versions.find (name);
            CONSTANT VersionVector& vers = (i!= needed_feature_versions.end()) ? *(i->second)  : empty_vector ;
            process_disable_up(name, intervals, vers, true);
        } else {
             PackageUnits::const_iterator i = needed_units.find(name);
             assert (i!= needed_units.end ());
             VersionVector versions;
             if (i!=needed_units.end ()) to_versions(*(i->second), versions);
             process_disable_up(name, intervals, versions, false);
        }
    }

    FOR_EACH (PackageToVersions::const_iterator, index, dd)
    {
        CONSTANT string name = index->first;
        VersionVector& intervals=*(index->second);
        sort (intervals.begin(),intervals.end());
        OUT (  cerr << "intervals " << name;
               FOR_EACH (VersionVector::const_iterator,i,intervals) cerr << " " << *i;
               cerr<<endl;
            )



        if (is_feature(name))
        {
            PackageToVersions::const_iterator i=needed_feature_versions.find (name);
            CONSTANT VersionVector& vers = (i!= needed_feature_versions.end()) ? *(i->second)  : empty_vector ;
            process_disable_down(name, intervals, vers, true);
        } else {
             PackageUnits::const_iterator i = needed_units.find(name);
             assert (i!= needed_units.end ());
             VersionVector versions;
             if (i!=needed_units.end ()) to_versions(*(i->second), versions);
             process_disable_down(name, intervals, versions, false);
        }
    }

    if (opt_edges) encode_edges();
    OUT (cerr << "]" << endl;)
}


void Encoder::encode_edges()
{
    CONSTANT PackageToVersions& ru=interval_vars.get_require_up ();
    CONSTANT PackageToVersions& rd=interval_vars.get_require_down();
    CONSTANT PackageToVersions& du=interval_vars.get_disable_up();
    CONSTANT PackageToVersions& dd=interval_vars.get_disable_down();

   FOR_EACH (PackageToVersions::const_iterator, index, ru)
    {
        CONSTANT string name = index->first;
        assert((index->second)!=NULL);
        VersionVector& ru_intervals=*(index->second);
        PackageToVersions::const_iterator i=du.find(name);
        if (i == du.end()) continue;
        assert((i->second)!=NULL);
        VersionVector& du_intervals =*(i->second);
        UINT rui=0;
        UINT dui=0;
        while ((rui < ru_intervals.size()) && (dui<du_intervals.size ()))
        {
            if (ru_intervals[rui]<du_intervals[dui]) ++rui;
            else {
                solver.output_binary_clause(neg(interval_vars.get_require_up(PackageVersion(name,ru_intervals[rui]))),
                                            neg(interval_vars.get_disable_up(PackageVersion(name,du_intervals[dui]))));
                ++dui;
            }
        }
    }
   FOR_EACH (PackageToVersions::const_iterator, index, rd)
    {
        CONSTANT string name = index->first;
        assert((index->second)!=NULL);
        VersionVector& rd_intervals=*(index->second);
        PackageToVersions::const_iterator i=dd.find(name);
        if (i == dd.end()) continue;
        assert((i->second)!=NULL);
        VersionVector& dd_intervals =*(i->second);
        int  rdi=rd_intervals.size()-1;
        int  ddi=dd_intervals.size ()-1;
        while ((rdi>=0) && (ddi>=0))
        {
            if (rd_intervals[rdi]>dd_intervals[ddi]) --rdi;
            else {
                solver.output_binary_clause(neg(interval_vars.get_require_down(PackageVersion(name,rd_intervals[rdi]))),
                                            neg(interval_vars.get_disable_down(PackageVersion(name,dd_intervals[ddi]))));
                --ddi;
            }
        }
    }
//   FOR_EACH (PackageToVersions::const_iterator, index, ru)
//   {
//        CONSTANT string name = index->first;
//        assert((index->second)!=NULL);
//        VersionVector& ru_intervals=*(index->second);
//   }
}

void Encoder::process_require_down(const string& package,
        const VersionVector& intervals,
        const VersionVector& package_versions,
        bool is_feature)
{
    if (intervals.empty()) return;//No intervals to process, nothing to do

    const UINT vsz=package_versions.size();
    const UINT isz=intervals.size();

    int current_interval_index=isz-1;
    int current_version_index=vsz-1;

    // find first package version that is within the first interval within an interval
    for (  ;current_version_index >= 0;  --current_version_index)
    {
        if (intervals[current_interval_index]>=package_versions[current_version_index])
            break;
    }

    LiteralVector literals;
    //Initialize literals with intervals[0] => ...
    {
      Variable v=interval_vars.get_require_down(PackageVersion(package,intervals[current_interval_index]));
      literals.push_back(neg(v));
    }
    int next_interval_index=  current_interval_index>0 ? current_interval_index-1 : -1;
    while (current_interval_index>=0) {
        // Flush the current literals IF
        //      all package versions have been processed
        //  OR  the current package version belongs into the next interval
        while (current_version_index<0
            ||
           (next_interval_index>=0 && intervals[next_interval_index]>=package_versions[current_version_index])
           )
        {
            Variable next_v=0;
            if (next_interval_index>=0) {// link to the next interval (if there is such)
                next_v=interval_vars.get_require_down(PackageVersion(package,intervals[next_interval_index]));
                literals.push_back(next_v);
            } else if (is_feature) {
                // PackageVersion (package,0) has already encoutered, if present
                // Unversioned feature  makes everything true
                //Variable infinite_var;
                // if (variables.find_variable(PackageVersion (package,0), infinite_var))
                //   literals.push_back(infinite_var);
            }

            solver.output_clause(literals); // flush current clause
            literals.clear();

            // move to the next interval
            current_interval_index=next_interval_index;
            next_interval_index=current_interval_index>0 ? current_interval_index-1 : -1;
            if (current_interval_index>=0) {
                literals.push_back(neg(next_v)); // Initialize literals for the next interval
            } else {
                assert (current_version_index<0);
                return;
            }
        }

        // Add current package version to the current interval and move to the next package version
        if (current_interval_index>=0 && current_version_index>=0) {
            Variable v=get_package_version_variable(
                    PackageVersion (package,package_versions[current_version_index]));
            literals.push_back(v);
            --current_version_index;
        }
    }
}

void Encoder::process_require_up(const string& package,
        const VersionVector& intervals,
        const VersionVector& package_versions,
        bool is_feature/*=false*/)
{   
    if (intervals.empty()) return;//No intervals to process, nothing to do

    UINT current_interval_index=0;
    UINT current_version_index=0;
    const UINT vsz=package_versions.size();
    // find first package version that is within the first interval within an interval
    for (  ;current_version_index < vsz;  ++current_version_index)
    {
        if (package_versions[current_version_index]>=intervals[current_interval_index])
            break;
    }
    const UINT isz=intervals.size();
    LiteralVector literals;
    //Initialize literals with intervals[0] => ...
    {
      Variable v=interval_vars.get_require_up(PackageVersion(package,intervals[current_interval_index]));
      literals.push_back(neg(v));
    }
    UINT next_interval_index= (current_interval_index+1)<isz ? current_interval_index+1 : isz;
    while (current_interval_index<isz) {
        // Flush the current literals IF
        //      all package versions have been processed
        //  OR  the current package version belongs into the next interval
        while (current_version_index>=vsz
            ||
           (next_interval_index<isz && package_versions[current_version_index]>=intervals[next_interval_index])
           )
        {
            Variable next_v=0;
            if (next_interval_index<isz) {// link to the next interval (if there is such)
                next_v=interval_vars.get_require_up(PackageVersion(package,intervals[next_interval_index]));
                literals.push_back(next_v);
            }else if (is_feature) {
                // Unversioned feature makes everything true
                Variable infinite_var=-1;
                if (variables.find_variable(PackageVersion (package,0), infinite_var))
                   literals.push_back(infinite_var);
            }

            solver.output_clause(literals); // flush current clause
            literals.clear();

            // move to the next interval
            current_interval_index=next_interval_index;
            next_interval_index=(current_interval_index+1) < isz ? current_interval_index+1 : isz;
            if (current_interval_index<isz) {
                literals.push_back(neg(next_v)); // Initialize literals for the next interval
            } else {
                assert (current_version_index>=vsz);
                return;
            }
        }

        // Add current package version to the current interval and move to the next package version
        if (current_interval_index<isz && current_version_index<vsz) {
            Variable v=get_package_version_variable(
                    PackageVersion (package,package_versions[current_version_index]));
            literals.push_back(v);
            ++current_version_index;
        }
    }
}


void Encoder::process_disable_up(const string& package,
        const VersionVector& intervals,
        const VersionVector& package_versions,
        bool is_feature/*=false*/)
{
    if (intervals.empty()) return;//No intervals to process, nothing to do

    UINT current_interval_index=0;
    UINT current_version_index=0;
    const UINT vsz=package_versions.size();
    // find first package version that is within the first interval within an interval
    for (  ;current_version_index < vsz;  ++current_version_index)
    {
        if (package_versions[current_version_index]>=intervals[current_interval_index])
            break;
    }
    const UINT isz=intervals.size();
    UINT next_interval_index= (current_interval_index+1)<isz ? current_interval_index+1 : isz;
    Variable current_interval_variable = interval_vars.get_disable_up(PackageVersion(package,intervals[current_interval_index]));
    while (current_interval_index<isz) {
        // Moved to next interval IF
        //      all package versions have been processed
        //  OR  the current package version belongs into the next interval
        while (current_version_index>=vsz
            ||
           (next_interval_index<isz && package_versions[current_version_index]>=intervals[next_interval_index])
           )
        {
            Variable next_v;
            if (next_interval_index<isz) {// link to the next interval (if there is such)
                next_v=interval_vars.get_disable_up(PackageVersion(package,intervals[next_interval_index]));
                solver.output_binary_clause(neg(current_interval_variable), next_v);
            } else if (is_feature) {
                // unversioned feature must be disabled in any case
                Variable infinite_var=-1;
                if (variables.find_variable(PackageVersion (package,0), infinite_var))
                   solver.output_binary_clause(neg(current_interval_variable), neg(infinite_var));
            }

            // move to the next interval
            current_interval_index=next_interval_index;
            current_interval_variable=next_v;
            next_interval_index=(current_interval_index+1) < isz ? current_interval_index+1 : isz;
            if (current_interval_index>=isz) {
                assert (current_version_index>=vsz);
                return;
            }
        }

        // Add current package version to the current interval and move to the next package version
        if (current_interval_index<isz && current_version_index<vsz) {
            Variable v=get_package_version_variable(
                    PackageVersion (package,package_versions[current_version_index]));
            solver.output_binary_clause(neg(current_interval_variable), neg(v));
            ++current_version_index;
        }
    }
}

void Encoder::process_disable_down(const string& package,
        const VersionVector& intervals,
        const VersionVector& package_versions,
        bool is_feature/*=false*/)
{
    if (intervals.empty()) return;//No intervals to process, nothing to do

    const int isz=intervals.size();
    const int vsz=package_versions.size();

    int current_interval_index=isz-1;
    int current_version_index=vsz-1;

    // find first package version that is within the first interval
    for (  ;current_version_index>=0;  --current_version_index)
    {
        if (intervals[current_interval_index]>=package_versions[current_version_index])
            break;
    }

    int next_interval_index= (current_interval_index-1)>=0 ? current_interval_index-1 : -1;
    Variable current_interval_variable = interval_vars.get_disable_down(PackageVersion(package,intervals[current_interval_index]));
    while (current_interval_index>=0) {
        // Moved to next interval IF
        //      all package versions have been processed
        //  OR  the current package version belongs into the next interval
        while (current_version_index<0
            ||
           (next_interval_index>=0 && intervals[next_interval_index]>=package_versions[current_version_index])
           )      {
            Variable next_v;
            if (next_interval_index>=0) {// link to the next interval (if there is such)
                next_v=interval_vars.get_disable_down(PackageVersion(package,intervals[next_interval_index]));
                solver.output_binary_clause(neg(current_interval_variable), next_v);
            } else if (is_feature){
                // unversioned feature must be disabled in any case
                Variable infinite_var=-1;
                if (variables.find_variable(PackageVersion (package,0), infinite_var))
                   solver.output_binary_clause(neg(current_interval_variable), neg(infinite_var));
            }

            // move to the next interval
            current_interval_index=next_interval_index;
            current_interval_variable=next_v;
            next_interval_index=(current_interval_index-1)>=0 ? current_interval_index-1 : -1;
            if (current_interval_index<0) {
                return;
            }
        }

        // Add current package version to the current interval and move to the next package version
        if (current_interval_index>=0 && current_version_index>=0) {
            Variable v=get_package_version_variable(
                    PackageVersion (package,package_versions[current_version_index]));
            solver.output_binary_clause(neg(current_interval_variable), neg(v));
            --current_version_index;
        }
    }
}

int Encoder::new_score(IntVector& model) {
    int new_packages = 0;
    FOR_EACH(PackageUnits::const_iterator, index, read_units) {
        UnitVector &unit_vector = *(index->second);
        bool was_any_installed = false;
        bool is_any_installed = false;

        FOR_EACH(UnitVector::iterator, unit_index, unit_vector) {
            InstallableUnit *unit = *unit_index;
            CONSTANT bool installed = is_installed(unit, model);
            if (unit->installed) was_any_installed = true;
            if (installed) is_any_installed = true;
        }
        if (!was_any_installed && is_any_installed) ++new_packages;
    }
    return new_packages;
}

int Encoder::removed_score(IntVector& model)
{
    int removed_packages = 0;
    FOR_EACH(PackageUnits::const_iterator,index,read_units)
    {
        UnitVector &unit_vector=*(index->second);
        bool was_any_installed = false;
        bool is_any_installed = false;
        FOR_EACH (UnitVector::iterator, unit_index, unit_vector)
        {
            InstallableUnit *unit =*unit_index;
            CONSTANT bool is_installed = (unit->needed && (model[unit->variable]>0));
            if (unit->installed) was_any_installed = true;
            if (is_installed) is_any_installed = true;
        }
        if (was_any_installed && !is_any_installed) ++removed_packages;
    }
    return removed_packages;
}

int Encoder::notuptodate_score(IntVector& model)
{
    int violated = 0;
    FOR_EACH(PackageUnits::const_iterator,index,read_units)
    {
        UnitVector &unit_vector=*(index->second);
        bool is_any_installed  = false;
        bool latest_installed = false;
        Version latest_version = 0;
        FOR_EACH (UnitVector::iterator, unit_index, unit_vector)
        {
            CONSTANT InstallableUnit *unit = *unit_index;
            CONSTANT bool installed = is_installed(unit, model);
            if (unit->version > latest_version) {
               latest_installed = installed; 
               latest_version = unit->version;
            }
            if (installed) is_any_installed = true;
        }
        if (is_any_installed && !latest_installed) { OUT(cerr << "nu: " << index->first << endl;) ++violated; }
    }
    return violated;
}

int Encoder::changed_score(IntVector& model)
{
    int changed_version_sets = 0;
    FOR_EACH(PackageUnits::const_iterator,index,read_units)
    {
        UnitVector &unit_vector=*(index->second);
        bool version_set_changed = false;
        FOR_EACH (UnitVector::iterator, unit_index, unit_vector)
        {
            InstallableUnit *unit =*unit_index;
            CONSTANT bool is_installed = (unit->needed && (model[unit->variable]>0));
            if ((unit->installed) != is_installed) version_set_changed=true;
        }
        if (version_set_changed) ++changed_version_sets;
    }
    return changed_version_sets;
}

int Encoder::recommends_score(IntVector& model)
{
    int score = 0;
    FOR_EACH(PackageUnits::const_iterator,index,read_units)
    {
        const UnitVector &unit_vector=*(index->second);
        FOR_EACH (UnitVector::const_iterator, unit_index, unit_vector)
        {
            CONSTANT InstallableUnit *unit =*unit_index;
            if (!is_installed(unit, model)) continue;//ignore things that are not installed
            CONSTANT PackageVersionsCNF& cnf = unit->recommends_cnf;
            FOR_EACH (PackageVersionsCNF::const_iterator, clause_index, cnf)
            {
                CONSTANT PackageVersionsList& clause = **clause_index;
                bool sat = false;
                FOR_EACH (PackageVersionsList::const_iterator,literal_index, clause)
                {
                    PackageVersions literal = *literal_index;
                    if (sat || is_any_installed(literal, model)) sat = true;
                }
                if (!sat) {
                    OUT (cerr << "unsatisfied recommends: ["<< unit->name  <<":" << unit->version<<"]";
                         collection_printing::print(clause,cerr); cerr<<endl;)
                    ++score;
                }
            }
        }
    }
    return score;
}

bool Encoder::is_installed(const InstallableUnit* unit, IntVector& model)
{
    return unit->needed && (model[unit->variable]>0);
}

bool Encoder::is_any_installed_feature(CONSTANT string &feature_name, 
                                       Operator version_operator, 
                                       Version operand,
                                       IntVector& model)
{
    // Process unversioned providers
    if (is_installed_feature(PackageVersion(feature_name,0))) return true;

    CONSTANT FeatureToVersions::const_iterator i = feature_versions.find(feature_name);
    if (i==feature_versions.end()) return false;
    CONSTANT VersionSet &vs=*(i->second);
    FOR_EACH (VersionSet::const_iterator,vi,vs)
    {
        CONSTANT Version version=*vi;
        if (evaluate(version, version_operator, operand))
           if (is_installed_feature(PackageVersion(feature_name,version)))
               return true;
    }
    return false;
}

bool Encoder::is_any_installed(PackageVersions& package, IntVector& model)
{
    if (is_feature (package.name())) 
      return is_any_installed_feature(package.name(), package.op(), package.version(), model);
    const PackageUnits::const_iterator i=needed_units.find(package.name());
    if (i==needed_units.end()) return false;
    UnitVector& unit_vector = *(i->second);
    FOR_EACH (UnitVector::const_iterator, unit_index, unit_vector)
    {
        InstallableUnit* unit =*unit_index;
        if (evaluate(unit->version, package.op(), package.version()))
           if (is_installed (unit, model)) return true;
    }
    return false;
}

bool Encoder::is_installed_feature(const PackageVersion& feature, IntVector& model)
{
    assert (is_feature(feature.name()));
    CONSTANT FeatureToUnits::const_iterator providers_index = feature_units.find(feature);
    if (providers_index==feature_units.end()) { assert(false); return false; }
    CONSTANT UnitVector& providers =*(providers_index->second);
    FOR_EACH (UnitVector::const_iterator, provider_index, providers)
    {
        CONSTANT InstallableUnit* provider =*provider_index;
        if (is_installed(provider,model)) return true;
    }
    return false;
}


void Encoder::trendy_score(IntVector& model)
{
   cerr << "trendy score[" <<
           removed_score(model) << "," <<
           notuptodate_score(model) << "," <<
           recommends_score(model)  << "," <<
           new_score(model)
       << "]"  << endl;
}

void Encoder::paranoid_score(IntVector& model)
{
    const int removed_packages = recommends_score(model);
    const int changed_version_sets = changed_score(model);
    cerr << "score[" << removed_packages << "," << changed_version_sets << "]"  << endl;
}

void Encoder::print_valuation(ostream& output,IntVector& model)
{
    output <<"# removed: " << removed_score(model) << '\n'
           <<"# not up-to-date: " << notuptodate_score(model) << '\n'
           <<"# recommends: " << recommends_score(model)  << '\n' 
           <<"# new: " << new_score(model) <<'\n'
           <<"# changed: " << changed_score(model);
}

void Encoder::check_solution() {
    cerr << "cheking solution" <<endl;
    // Package dependencies
    FOR_EACH(PackageUnits::const_iterator,i,needed_units) {
        CONSTANT UnitVector &uv = *(i->second);
        FOR_EACH(UnitVector::const_iterator,vi,uv)
        {
            CONSTANT InstallableUnit *unit = *vi;
            const Version v=unit->version;
            PackageVersion pv=PackageVersion(unit->name, v);
            const bool c=(stc->find(pv) != stc->end());
            // cerr <<  pv.to_string() << ( c ? "  installed" : " not installed"  ) << endl;
            if (!c) solver.output_unary_clause(neg(unit->variable));
        }
    }

    FOR_EACH(PackageVersionSet::const_iterator, i, (*stc))  {
        const PackageVersion &pv=*i;
        Variable var;
        bool f=variables.find_variable(pv,var);
        if (f) {
            solver.output_unary_clause(var);
        } else {
            cerr << "using sliced pck: " << pv.to_string() << endl;
        }
    }
    bool has_solution = solver.solve();
    if (has_solution) {
        cerr << "is sol" << endl;
        cerr << "MSUNCore min unsat: " << solver.get_min_unsat_cost() << endl;
    }
    else {
        cerr << "is not sol" << endl;
    }
}
