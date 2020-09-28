
/*******************************************************/
/* CUDF solver: unaligned_criteria.c                   */
/* Implementation of the unaligned criteria            */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <unaligned_criteria.h>

// Check property availability
void unaligned_criteria::check_property(CUDFproblem *problem) {
  CUDFPropertiesIterator source_prop        =  problem->properties->find(string(source_name));
  CUDFPropertiesIterator sourceversion_prop =  problem->properties->find(string(sourceversion_name));
  CUDFPropertiesIterator version_prop       =  problem->properties->find(string(version_name));

  has_properties = false;

  if (source_prop == problem->properties->end())
    printf("WARNING: cannot find \"%s\" property definition: criteria unaligned not used.\n", source_name);
  else if (sourceversion_prop == problem->properties->end())
    printf("WARNING: cannot find \"%s\" property definition: criteria unaligned not used.\n", sourceversion_name);
  else if (version_prop == problem->properties->end())
    printf("WARNING: cannot find \"%s\" property definition: criteria unaligned not used.\n", version_name);
  else if ((*source_prop).second->type_id !=  pt_string)
    printf("WARNING: Property \"%s\" has wrong type: criteria unaligned not used.\n", source_name);
  else if ((*sourceversion_prop).second->type_id !=  pt_string)
    printf("WARNING: Property \"%s\" has wrong type: criteria unaligned not used.\n", sourceversion_name);
  else if ((*version_prop).second->type_id !=  pt_string)
    printf("WARNING: Property \"%s\" has wrong type: criteria unaligned not used.\n", version_name);
  else {
    has_properties = true;
  }
}

// Get n number from +b<n> version name
int get_n_from_bn(char *version_name) {
  int n = 0;

  if (version_name != (char *)NULL) {
    int l = strlen(version_name);
    char *ptr = version_name + l - 1;
    bool has_digit = false;

    for (; ('0' <= *ptr) && (*ptr <= '9'); ptr--, has_digit = true);

    if (has_digit && (ptr > version_name) && (*ptr == 'b') && (*(ptr - 1) == '+'))
      n = atoi(ptr+1);
  }

  return n;
}

// Criteria initialization
void unaligned_criteria::initialize(CUDFproblem *problem, abstract_solver *solver) {
  this->problem = problem;
  this->solver = solver;

  if (has_properties) {
    CUDFPropertiesIterator source_prop        =  problem->properties->find(string(source_name));
    CUDFPropertiesIterator sourceversion_prop =  problem->properties->find(string(sourceversion_name));
    CUDFPropertiesIterator version_prop       =  problem->properties->find(string(version_name));

    source_set = new a_source_set(1000);
    // Check properties availability for all packages
    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++) {
      char *pkg_source = (char *)NULL;
      char *pkg_sourceversion = (char *)NULL;
      char *pkg_version = (char *)NULL;

      for (CUDFPropertyValueListIterator propval = (*ipkg)->properties.begin();  propval != (*ipkg)->properties.end(); propval++) {
        if ((*propval)->property == (*source_prop).second)             pkg_source = (*propval)->strval;         // Here is package source name
        else if ((*propval)->property == (*sourceversion_prop).second) pkg_sourceversion = (*propval)->strval;  // Here is package source version name
        else if ((*propval)->property == (*version_prop).second)       pkg_version = (*propval)->strval;        // Here is package version name
      }

      //      printf("('%s', %llu) -> '%s', '%s', '%s'\n", (*ipkg)->name, (*ipkg)->version, pkg_source, pkg_sourceversion, pkg_version);

      if ((pkg_source != (char *)NULL) && (pkg_sourceversion != (char *)NULL)) { // Check if we've got something for this package
	a_source_set_iterator source_item;
	string source_hash_name = string(pkg_source);
	int bn = 0;

	if (pkg_version != (char *)NULL) bn = get_n_from_bn(pkg_version);

	source_item = source_set->find(source_hash_name);

	if (source_item != source_set->end()) { // source already available
	  a_sourceversion_set_iterator sourceversion_set_item = (source_item->second)->find(string(pkg_sourceversion));

	  if (sourceversion_set_item == (source_item->second)->end()) {
	    a_pkg_compil *pkg_compil;
	    a_pkg_compil_set *pkg_compil_set;

	    pkg_compil = new a_pkg_compil();
	    (*pkg_compil)[bn] = *ipkg;
	    pkg_compil_set = new a_pkg_compil_set();
	    (*pkg_compil_set)[string((*ipkg)->name)] = pkg_compil;
	    (*(source_item->second))[string(pkg_sourceversion)] = pkg_compil_set;
	  } else {
	    a_pkg_compil_set_iterator pkg_compil_set_item = (sourceversion_set_item->second)->find(string((*ipkg)->name));

	    if (pkg_compil_set_item == (sourceversion_set_item->second)->end()) {
	      a_pkg_compil *pkg_compil;

	      pkg_compil = new a_pkg_compil();
	      (*pkg_compil)[bn] = *ipkg;
	      (*(sourceversion_set_item->second))[string((*ipkg)->name)] = pkg_compil;
	    } else {
	      a_pkg_compil_iterator pkg_compil_item = (pkg_compil_set_item->second)->find(bn);

	      if (pkg_compil_item == (pkg_compil_set_item->second)->end()) {
		(*(pkg_compil_set_item->second))[bn] = *ipkg;
	      } else {
		fprintf(stderr, "unaligned_criteria.c: 2 packages sharing same cell: (%s, %llu) & (%s, %llu)", 
			(*ipkg)->name, (*ipkg)->version, (pkg_compil_item->second)->name, (pkg_compil_item->second)->version);
	      }
	    }
	  }
	} else { // source creation
	  a_pkg_compil *pkg_compil;
	  a_pkg_compil_set *pkg_compil_set;
	  a_sourceversion_set *sourceversion_set;

	  pkg_compil = new a_pkg_compil();
	  (*pkg_compil)[bn] = *ipkg;
	  pkg_compil_set = new a_pkg_compil_set();
	  (*pkg_compil_set)[string((*ipkg)->name)] = pkg_compil;
	  sourceversion_set = new a_sourceversion_set();
	  (*sourceversion_set)[string(pkg_sourceversion)] = pkg_compil_set;
	  (*source_set)[source_hash_name] = sourceversion_set;
	}
      }

    } // Check properties availability for all packages

    // Print it out
    if (verbosity > 2) { 
      printf("===================================================================================\n");
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	int srcsize = (int)((*isrc).second)->size();
	printf("source: %s\nsrcsize: %d\n", (*isrc).first.c_str(), srcsize);
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  int srcversize = (int)((*isrcver).second)->size();
	  printf("  source version: %s\n  srcversize: %d\n", (*isrcver).first.c_str(), srcversize);
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    printf("    package: %s\n", (*ipkgset).first.c_str());
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      printf("    b%d: (%s, %llu)\n", (*ipkg).first, ((*ipkg).second)->name, ((*ipkg).second)->version);
	    }
	  }
	}
      }
      printf("===================================================================================\n");
    }

    // Cleanning it out (throw out any source reduced to 1 package per version) and compute nb_sources and nb_versions
    if (1) {
      nb_versions = 0;
      nb_packages = 0;
      nb_pairs = 0;
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	int srcsize = (int)((*isrc).second)->size();
	int src_nb_versions = 0;
	int snb_packages = 0;
	vector<int> verszs;
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  int srcversize = (int)((*isrcver).second)->size();
	  int sverpkg = 0;
	  src_nb_versions += srcversize;
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    int size = ((*ipkgset).second)->size();
	    snb_packages += size;
	    sverpkg += size;
	  }
	  verszs.push_back(sverpkg);
	}
	if (srcsize == 1)
	  source_set->erase(isrc);
	else {
	  nb_versions += src_nb_versions;
	  nb_packages += snb_packages;
	  for (int i = 0; i < srcsize -1; i++) {
	    int sum = 0;
	    for (int j = i+1; j < srcsize; j++) sum += verszs[j];
	    nb_pairs += verszs[i]*sum;
	  }
	}
      }
      nb_sources = source_set->size();
      if (verbosity > 0) {
	printf("nb_sources = %d, nb_versions = %d, nb_packages = %d, nb_pairs = %d\n",
	       nb_sources, nb_versions, nb_packages, nb_pairs);
	printf("packages => %d cols\n", nb_packages+nb_versions);
	printf("pairs    => %d cols\n", nb_pairs);
	printf("clusters => %d cols\n", nb_sources*2+nb_versions);
	printf("changes  => %d cols\n", nb_sources*3+nb_versions);
      }
    }
      
    // Print it out
    if (verbosity > 2) 
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	int srcsize = (int)((*isrc).second)->size();
	printf("source: %s\nsrcsize: %d\n", (*isrc).first.c_str(), srcsize);
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  int srcversize = (int)((*isrcver).second)->size();
	  printf("  source version: %s\n  srcversize: %d\n", (*isrcver).first.c_str(), srcversize);
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    printf("    package: %s\n", (*ipkgset).first.c_str());
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      printf("    b%d: (%s, %llu)\n", (*ipkg).first, ((*ipkg).second)->name, ((*ipkg).second)->version);
	    }
	  }
	}
      }
  } // Has properties
}

// temp
void unaligned_criteria::display_struct() {
  for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
    bool installedver = false;
    int srcsize = (int)((*isrc).second)->size();
    printf("source: %s\nsrcsize: %d\n", (*isrc).first.c_str(), srcsize);
    for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
      bool has_installed = false;
      int srcversize = (int)((*isrcver).second)->size();
      printf("  source version: %s\n  srcversize: %d\n", (*isrcver).first.c_str(), srcversize);
      for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	printf("    package: %s\n", (*ipkgset).first.c_str());
	for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	  printf("      b%d: (%s, %llu)", (*ipkg).first, ((*ipkg).second)->name, ((*ipkg).second)->version);
	  if (solver->get_solution((*ipkg).second)) {
	    has_installed = true;
	    if (installedver) printf(" -> INSTALLED !!\n"); else printf(" -> installed\n");
	  } else
	    printf("\n");
	}
      }
      if (has_installed) installedver = true;
    }
  }
}

// Initialize integer variables
void unaligned_criteria::initialize_intvars() {
  if (has_properties) {
    int nbver;

    switch(alignment) {
    case ALIGNED_PACKAGES: break;
    case ALIGNED_PAIRS:    break;
    case ALIGNED_CLUSTERS:
      nbver = first_free_var + nb_sources;
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	int size =  (int)((*isrc).second)->size();
	solver->set_intvar_range(nbver++, 0, size);
      }
      break;
    case ALIGNED_CHANGES:
      int nbchange = first_free_var;
      nbver = first_free_var + nb_sources;
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	int size =  (int)((*isrc).second)->size();
	solver->set_intvar_range(nbver++, 0, size);
	solver->set_intvar_range(nbchange++, 0, size - 1);
      }
      break;
    }
  } 
}

// Computing the number of columns required to handle the criteria
int unaligned_criteria::set_variable_range(int first_free_var) {
  if (has_properties) {
    this->first_free_var = first_free_var;
    switch(alignment) {
    case ALIGNED_PACKAGES: return first_free_var + nb_packages + nb_versions;
    case ALIGNED_PAIRS:    return first_free_var + nb_pairs;
    case ALIGNED_CLUSTERS: return first_free_var + nb_sources*2 + nb_versions;
    case ALIGNED_CHANGES:  return first_free_var + nb_sources*3 + nb_versions;
    }
    return first_free_var;
  } else
    return first_free_var;
}

// Add the criteria to the current objective function
int unaligned_criteria::add_criteria_to_objective(CUDFcoefficient lambda) {
  if (has_properties) {
    switch(alignment) {
    case ALIGNED_PACKAGES:
      for (int i = 0; i < nb_packages; i++) solver->set_obj_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    case ALIGNED_PAIRS:
      for (int i = 0; i < nb_pairs; i++) solver->set_obj_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    case ALIGNED_CLUSTERS:
      for (int i = 0; i < nb_sources; i++) solver->set_obj_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    case ALIGNED_CHANGES:  
      for (int i = 0; i < nb_sources; i++) solver->set_obj_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    }
  }
  return 0;
}

// Add the criteria to the constraint set
int unaligned_criteria::add_criteria_to_constraint(CUDFcoefficient lambda) {
  if (has_properties) {
    switch(alignment) {
    case ALIGNED_PACKAGES:
      for (int i = 0; i < nb_packages; i++) solver->set_constraint_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    case ALIGNED_PAIRS:
      for (int i = 0; i < nb_pairs; i++) solver->set_constraint_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    case ALIGNED_CLUSTERS:
      for (int i = 0; i < nb_sources; i++) solver->set_constraint_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    case ALIGNED_CHANGES:  
      for (int i = 0; i < nb_sources; i++) solver->set_constraint_coeff(first_free_var + i, lambda_crit * lambda);
      break;
    }
  }
  return 0;
}

// Add the constraints required by the criteria
int unaligned_criteria::add_constraints() {
  if (has_properties) {
    if (alignment == ALIGNED_CHANGES) { // =======================================================================================
      int nbchange = first_free_var;
      int nbver = first_free_var + nb_sources;
      int cond = first_free_var + nb_sources*2;
      int version = first_free_var + nb_sources*3;
      // sum(pkg) - ver >= 0 
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  solver->new_constraint();
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      solver->set_constraint_coeff(((*ipkg).second), +1);
	    }
	  }
	  solver->set_constraint_coeff(version++, -1);
	  solver->add_constraint_geq(0);
	}
      }
      
      version = first_free_var + nb_sources*3;
      // for each pkg, ver - pkg >= 0 
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      solver->new_constraint();
	      solver->set_constraint_coeff(version, +1);
	      solver->set_constraint_coeff(((*ipkg).second), -1);
	      solver->add_constraint_geq(0);
	    }
	  }
	  version++;
	}
      }
      
      version = first_free_var + nb_sources*3;
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	// nbver = sum(versions)
	solver->new_constraint();
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++)
	  solver->set_constraint_coeff(version++, +1);
	solver->set_constraint_coeff(nbver, -1);
	solver->add_constraint_eq(0);
	// max nb version * cond - nbver >= 0 (set cond to 1 iff nbver >= 1)
	solver->new_constraint();
	solver->set_constraint_coeff(cond, (int)((*isrc).second)->size());
	solver->set_constraint_coeff(nbver, -1);
	solver->add_constraint_geq(0);
	// nbver - cond >= 0 (set cond to 0 iff nbver = 0)
	solver->new_constraint();
	solver->set_constraint_coeff(nbver, +1);
	solver->set_constraint_coeff(cond, -1);
	solver->add_constraint_geq(0);
	// nbchange = nbver - cond  == nbchange - nbver + cond = 0
	solver->new_constraint();
	solver->set_constraint_coeff(nbchange++, +1);
	solver->set_constraint_coeff(nbver++, -1);
	solver->set_constraint_coeff(cond++, +1);
	solver->add_constraint_eq(0);
      }
    } else if (alignment == ALIGNED_CLUSTERS) { // =======================================================================================
      int cluster = first_free_var;
      int nbver = first_free_var + nb_sources;
      int version = first_free_var + 2*nb_sources;
      // sum(pkg) - ver >= 0 
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  solver->new_constraint();
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      solver->set_constraint_coeff(((*ipkg).second), +1);
	    }
	  }
	  solver->set_constraint_coeff(version++, -1);
	  solver->add_constraint_geq(0);
	}
      }
      
      version = first_free_var + 2*nb_sources;
      // for each pkg, ver - pkg >= 0 
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      solver->new_constraint();
	      solver->set_constraint_coeff(version, +1);
	      solver->set_constraint_coeff(((*ipkg).second), -1);
	      solver->add_constraint_geq(0);
	    }
	  }
	  version++;
	}
      }
      
      version = first_free_var + 2*nb_sources;
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	// nbver = sum(versions)
	solver->new_constraint();
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++)
	  solver->set_constraint_coeff(version++, +1);
	solver->set_constraint_coeff(nbver, -1);
	solver->add_constraint_eq(0);
	// max nb version * cluster - nbver >= -1 (set cluster to 1 iff nbver >= 2)
	solver->new_constraint();
	solver->set_constraint_coeff(cluster, (int)((*isrc).second)->size());
	solver->set_constraint_coeff(nbver, -1);
	solver->add_constraint_geq(-1);
	// nbver - 2 * cluster >= 0 (set cluster to 0 iff nbver <= 1)
	solver->new_constraint();
	solver->set_constraint_coeff(nbver++, +1);
	solver->set_constraint_coeff(cluster++, -2);
	solver->add_constraint_geq(0);
      }
    } else if (alignment == ALIGNED_PACKAGES) { // =======================================================================================
      int nalignpkg = first_free_var;
      int version = first_free_var + nb_packages;
      // sum(pkg) - ver >= 0 
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  solver->new_constraint();
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      solver->set_constraint_coeff(((*ipkg).second), +1);
	    }
	  }
	  solver->set_constraint_coeff(version++, -1);
	  solver->add_constraint_geq(0);
	}
      }
      
      version = first_free_var + nb_packages;
      // for each pkg, ver - pkg >= 0 
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      solver->new_constraint();
	      solver->set_constraint_coeff(version, +1);
	      solver->set_constraint_coeff(((*ipkg).second), -1);
	      solver->add_constraint_geq(0);
	    }
	  }
	  version++;
	}
      }
      
      version = first_free_var + nb_packages;
      int sourceversion = version;
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	int nbver = ((*isrc).second)->size();
	int ver = 0;
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      // nalignpkg <= pkgi i.e. nalignpkg = 0 if pkgi not installed
	      solver->new_constraint();
	      solver->set_constraint_coeff(nalignpkg, -1);
	      solver->set_constraint_coeff(((*ipkg).second), +1);
	      solver->add_constraint_geq(0);
	      // nalignpkg >= version + pkgi -1 i.e. nalignpkg = 1 iff pkgi = 1 and version = 1
	      for (int v = 0; v < nbver; v++)
		if (v != ver) {
		  solver->new_constraint();
		  solver->set_constraint_coeff(nalignpkg, +1);
		  solver->set_constraint_coeff(sourceversion + v, -1);
		  solver->set_constraint_coeff(((*ipkg).second), -1);
		  solver->add_constraint_geq(-1);
		}
	      // nalignpkg <= sum(version != pkgi version) i.e. nalignpkg = 0 iff none other version installed
	      solver->new_constraint();
	      for (int v = 0; v < nbver; v++)
		if (v != ver) {
		  solver->set_constraint_coeff(sourceversion + v, +1);
		}
	      solver->set_constraint_coeff(nalignpkg, -1);
	      solver->add_constraint_geq(0);
	      nalignpkg++;
	    }
	  }
	  ver++;
	  version++;
	}
	sourceversion = version;
      }
    } else if (alignment == ALIGNED_PAIRS) { // =======================================================================================
      int nalignpair = first_free_var;
      
      //      printf("nb_packages = %d\n", nb_packages);
      for (a_source_set_iterator isrc = source_set->begin(); isrc != source_set->end(); isrc++) {
	for (a_sourceversion_set_iterator isrcver = ((*isrc).second)->begin(); isrcver != ((*isrc).second)->end(); isrcver++) {
	  for (a_pkg_compil_set_iterator ipkgset = ((*isrcver).second)->begin(); ipkgset != ((*isrcver).second)->end(); ipkgset++) {
	    for (a_pkg_compil_iterator ipkg = ((*ipkgset).second)->begin(); ipkg != ((*ipkgset).second)->end(); ipkg++) {
	      for (a_sourceversion_set_iterator isrcveri = isrcver; isrcveri != ((*isrc).second)->end(); isrcveri++) {
		if (isrcver != isrcveri)
		  for (a_pkg_compil_set_iterator ipkgseti = ((*isrcveri).second)->begin(); ipkgseti != ((*isrcveri).second)->end(); ipkgseti++) {
		    for (a_pkg_compil_iterator ipkgi = ((*ipkgseti).second)->begin(); ipkgi != ((*ipkgseti).second)->end(); ipkgi++) {
		      //	      printf("x%d = (%s, %llu) and (%s, %llu)\n", nalignpair,
		      //     ((*ipkg).second)->name, ((*ipkg).second)->version,
		      //     ((*ipkgi).second)->name, ((*ipkgi).second)->version);
		      // nalignpair <= ipkg i.e.  nalignpair = 0 if ipkg not installed
		      solver->new_constraint();
		      solver->set_constraint_coeff(((*ipkg).second), +1);
		      solver->set_constraint_coeff(nalignpair, -1);
		      solver->add_constraint_geq(0);
		      // nalignpair <= ipkgi i.e.  nalignpair = 0 if ipkgi not installed
		      solver->new_constraint();
		      solver->set_constraint_coeff(((*ipkgi).second), +1);
		      solver->set_constraint_coeff(nalignpair, -1);
		      solver->add_constraint_geq(0);
		      // nalignpair >= ipkg + ipkgi - 1 i.e.  nalignpair = 1 if ipkg and ipkgi are installed
		      solver->new_constraint();
		      solver->set_constraint_coeff(((*ipkg).second), -1);
		      solver->set_constraint_coeff(((*ipkgi).second), -1);
		      solver->set_constraint_coeff(nalignpair, +1);
		      solver->add_constraint_geq(-1);
		      nalignpair++;
		    }
		  }
	      }
	    }
	  }
	}
      }
    }
  }
  return 0;
}

// Compute the criteria range
CUDFcoefficient unaligned_criteria::bound_range() {
  if (has_properties) 
    switch(alignment) {
    case ALIGNED_PACKAGES: return lambda_crit * nb_packages;
    case ALIGNED_PAIRS:    return lambda_crit * nb_pairs;
    case ALIGNED_CLUSTERS: return lambda_crit * nb_sources;
    case ALIGNED_CHANGES:  return lambda_crit * nb_versions;
    }
  return 0; 
}

// Compute the criteria upper bound
CUDFcoefficient unaligned_criteria::upper_bound() { return bound_range(); }

// Compute the criteria lower bound
CUDFcoefficient unaligned_criteria::lower_bound() { return 0; }


