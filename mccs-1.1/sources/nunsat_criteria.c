
/*******************************************************/
/* CUDF solver: nunsat_criteria.c                      */
/* Implementation of the nunsat criteria               */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <nunsat_criteria.h>
#include <cudf_reductions.h>
#include <iostream>

// Check property availability
void nunsat_criteria::check_property(CUDFproblem *problem) {
  CUDFPropertiesIterator prop =  problem->properties->find(string(property_name));

  has_property = false;

  if (prop == problem->properties->end())
    printf("WARNING: cannot find \"%s\" property definition: criteria nunsat not used.\n", property_name);
  else if ((*prop).second->type_id !=  pt_vpkgformula)
    printf("WARNING: Property \"%s\" has wrong type: criteria nunsat not used.\n", property_name);
  else {
    has_property = true;
    process_properties.push_back(prop);
  }
}

// Criteria initialization
void nunsat_criteria::initialize(CUDFproblem *problem, abstract_solver *solver) {
  CUDFPropertiesIterator prop =  problem->properties->find(string(property_name));
  this->problem = problem;
  this->solver = solver;
  disjuncts = 0;
  range = 0;

  if (has_property) {
    versioned_pkg_with_property = new CUDFVersionedPackageList();
    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++) {
      for (CUDFPropertyValueListIterator propval = (*ipkg)->properties.begin();  propval != (*ipkg)->properties.end(); propval++)
        if ((*propval)->property == (*prop).second) {
          CUDFVpkgFormula *property = (*propval)->vpkgformula;

	  versioned_pkg_with_property->push_back((*ipkg));

          for (CUDFVpkgFormulaIterator conjunct = (*property).begin();  conjunct != (*property).end(); conjunct++) {

            int nb = 0;
            bool self_depend = false;

            for (CUDFVpkgListIterator disjunct = (*conjunct)->begin(); disjunct != (*conjunct)->end(); disjunct++) {

	      if (((*disjunct) == vpkg_true) || ((*disjunct) == vpkg_false)) continue;

              CUDFVirtualPackage *vpackage = (*disjunct)->virtual_package;
              a_compptr comp = get_comparator((*disjunct)->op);
              if (vpackage->all_versions.size() > 0) {
                for (CUDFVersionedPackageSetIterator jpkg = vpackage->all_versions.begin(); jpkg != vpackage->all_versions.end(); jpkg++)
                  if (comp((*jpkg)->version, (*disjunct)->version)) {
                    if ((*jpkg) == (*ipkg)) { // Then, the dependency is always checked
                      self_depend = true;
                      nb = 0;
                      break;
                    } else {
                      nb++;
                    }
                  }
              }
	      if (with_providers) {
		// as well as from all the providers
		if ((! self_depend) && (vpackage->providers.size() > 0)) {
		  for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
		    if ((*jpkg) == (*ipkg)) { // Then, the dependency is always checked
		      self_depend = true;
		      nb = 0;
		      break;
		    } else {
		      nb++;
		    }
		}
		// as well as from all the versioned providers with the right version
		if (! self_depend)
		  for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin();
		       jpkg != vpackage->versioned_providers.end(); jpkg++)
		    if (self_depend)
		      break;
		    else if (comp(jpkg->first, (*disjunct)->version))
		      for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++)
			if ((*kpkg) == (*ipkg)) { // Then, the dependency is always checked
			  self_depend = true;
			  nb = 0;
			  break;
			} else {
			  nb++;
			}
	      }
            } // disjunct
	    if (nb > 0) {
	      range++;
	      if (nb > 1) disjuncts++;
	    }
	  } // conjunct
	  break;
	} // is propval
    }
  }
  std::cout << "# Recommends UB: " << range << std::endl;
}

// Computing the number of columns required to handle the criteria
int nunsat_criteria::set_variable_range(int first_free_var) {
  if (has_property) {
    this->first_free_var = first_free_var;
    return range + disjuncts + first_free_var;
  } else
    return first_free_var;
}

// Add the criteria to the current objective function
int nunsat_criteria::add_criteria_to_objective(CUDFcoefficient lambda) {
  if (has_property)
    for (int i = 0; i < range; i++) solver->set_obj_coeff(first_free_var + i, lambda_crit * lambda);

  return 0;
}

// Add the criteria to the constraint set
int nunsat_criteria::add_criteria_to_constraint(CUDFcoefficient lambda) {
  if (has_property) {
    for (int i = 0; i < range; i++) solver->set_constraint_coeff(first_free_var + i, lambda_crit * lambda);
  }
  return 0;
}

// Add the constraints required by the criteria
int nunsat_criteria::add_constraints() {
  if (has_property) {
    CUDFPropertiesIterator prop =  problem->properties->find(string(property_name));
    //    int ipkg_rank = gstart = first_free_var;
    //    int disjunct_rank = gend = ipkg_rank + range;
    int ipkg_rank = first_free_var;
    int disjunct_rank = ipkg_rank + range;

    //    printf("ipkg_rank = %d, disjunct_rank = %d, range = %d, disjuncts = %d, size = %u\n", ipkg_rank, disjunct_rank, 
    //	   range, disjuncts, versioned_pkg_with_property->size());

    for (CUDFVersionedPackageListIterator ipkg = versioned_pkg_with_property->begin();
         ipkg != versioned_pkg_with_property->end(); ipkg++) {
      //      printf("handling (%s, "CUDFflags")\n", (*ipkg)->name, (*ipkg)->version);
      for (CUDFPropertyValueListIterator propval = (*ipkg)->properties.begin();  propval != (*ipkg)->properties.end(); propval++)
        if ((*propval)->property == (*prop).second) {
          CUDFVpkgFormula *property = (*propval)->vpkgformula;
	  int last_pkg = 0;

          for (CUDFVpkgFormulaIterator conjunct = (*property).begin();  conjunct != (*property).end(); conjunct++) {

            int nb = 0;
            bool self_depend = false;
	    bool has_false = false;

            solver->new_constraint();
            for (CUDFVpkgListIterator disjunct = (*conjunct)->begin(); disjunct != (*conjunct)->end(); disjunct++) {

	      if ((*disjunct) == vpkg_true) { // The disjunct contains true
		has_false = false; // False is ignored
		nb = 0; // Other packages are ignored
		break; // Skip next packages
	      }

	      if ((*disjunct) == vpkg_true) { // The disjunct contains false
		has_false = true;
		continue; // Directly handle next package
	      }

              CUDFVirtualPackage *vpackage = (*disjunct)->virtual_package;
              a_compptr comp = get_comparator((*disjunct)->op);
              if (vpackage->all_versions.size() > 0) {
                for (CUDFVersionedPackageSetIterator jpkg = vpackage->all_versions.begin(); jpkg != vpackage->all_versions.end(); jpkg++)
                  if (comp((*jpkg)->version, (*disjunct)->version)) {
                    if ((*jpkg) == (*ipkg)) { // Then, the dependency is always checked
                      self_depend = true;
                      nb = 0;
                      break;
                    } else if (solver->get_constraint_coeff(*jpkg) == 0) {
                      nb++;
                      solver->set_constraint_coeff(*jpkg, +1);
		      last_pkg = (*jpkg)->rank;
                    }
                  }
              }
	      if (with_providers) {
		// as well as from all the providers
		if ((! self_depend) && (vpackage->providers.size() > 0)) {
		  for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
		    if ((*jpkg) == (*ipkg)) { // Then, the dependency is always checked
		      self_depend = true;
		      nb = 0;
		      break;
		    } else if (solver->get_constraint_coeff(*jpkg) == 0) {
		      nb++;
		      solver->set_constraint_coeff(*jpkg, +1);
		      last_pkg = (*jpkg)->rank;
		    }
		}
		// as well as from all the versioned providers with the right version
		if (! self_depend)
		  for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin();
		       jpkg != vpackage->versioned_providers.end(); jpkg++)
		    if (self_depend)
		      break;
		    else if (comp(jpkg->first, (*disjunct)->version))
		      for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++)
			if ((*kpkg) == (*ipkg)) { // Then, the dependency is always checked
			  self_depend = true;
			  nb = 0;
			  break;
			} else if (solver->get_constraint_coeff(*kpkg) == 0) {
			  nb++;
			  solver->set_constraint_coeff(*kpkg, +1);
			  last_pkg = (*kpkg)->rank;
			}
	      }
            } // disjunct

	    if (nb == 0) {
	      if (has_false) {
		solver->new_constraint();  // ipkg_rank == (*ipkg)
		solver->set_constraint_coeff((*ipkg), -1);
		solver->set_constraint_coeff(ipkg_rank, +1);
		solver->add_constraint_eq(0);
	      }
            } else if (nb > 0) {  // disjunct_rank = 1 if unsat disjunct, 0 otherwise
	      if (nb == 1) {
		solver->new_constraint();  // ipkg_rank = 0 if package is not installed
		solver->set_constraint_coeff((*ipkg), -1);
		solver->set_constraint_coeff(ipkg_rank, +1);
		solver->add_constraint_leq(0);
		
		solver->new_constraint();  // ipkg_rank = 0 if disjunct is satisfied (i.e. = 0)
		solver->set_constraint_coeff(last_pkg, +1);
		solver->set_constraint_coeff(ipkg_rank, +1);
		solver->add_constraint_leq(+1);

		solver->new_constraint();  // if package is installed and disjunct_rank is unsatified, ipkg_rank = 1
		solver->set_constraint_coeff((*ipkg), -1);
		solver->set_constraint_coeff(last_pkg, +1);
		solver->set_constraint_coeff(ipkg_rank, +1);
		solver->add_constraint_geq(0);
	      } else {
		solver->set_constraint_coeff(disjunct_rank, +1);
		solver->add_constraint_geq(+1);
		solver->set_constraint_coeff(disjunct_rank, +nb);
		solver->add_constraint_leq(+nb);
	      
		solver->new_constraint();  // ipkg_rank = 0 if package is not installed
		solver->set_constraint_coeff((*ipkg), -1);
		solver->set_constraint_coeff(ipkg_rank, +1);
		solver->add_constraint_leq(0);
		
		solver->new_constraint();  // ipkg_rank = 0 if disjunct is satisfied (i.e. = 0)
		solver->set_constraint_coeff(disjunct_rank, -1);
		solver->set_constraint_coeff(ipkg_rank, +1);
		solver->add_constraint_leq(0);
		
		solver->new_constraint();  // if package is installed and disjunct_rank is unsatified, ipkg_rank = 1
		solver->set_constraint_coeff((*ipkg), -1);
		solver->set_constraint_coeff(disjunct_rank, -1);
		solver->set_constraint_coeff(ipkg_rank, +1);
		solver->add_constraint_geq(-1);

		disjunct_rank++;
	      }

	      ipkg_rank++;
            } // nb > 0
          } // conjunct

          // We've got the requested property. Don't need to go further.
          break;
        } // is propval
    } // ipkgs
    //    printf("ipkg_rank = %d, disjunct_rank = %d\n", ipkg_rank, disjunct_rank); fflush(stdout);
  } // has_property

  return 0;
}

// Compute the criteria range
CUDFcoefficient nunsat_criteria::bound_range() { if (has_property) return CUDFabs(lambda_crit) * range; else return 0; }

// Compute the criteria upper bound
CUDFcoefficient nunsat_criteria::upper_bound() { 
  if ((has_property) && (lambda_crit >= 0))
    return lambda_crit * range; 
  else
    return 0;
}

// Compute the criteria lower bound
CUDFcoefficient nunsat_criteria::lower_bound() { 
  if (has_property) {
    if (lambda_crit >= 0)
      return 0;
    else
      return lambda_crit * range; 
  } else
    return 0;
}


