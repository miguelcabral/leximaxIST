PackUP --  PACKage UPgradability with Boolean formulations
=================================================================================
 authors: Mikolas Janota, Joao Marques-Silva
 contributors: Ines Lynce, Vasco Manquinho
 email mikolas AT sat DOT inesc-id DOT pt
 (C) 2011 Mikolas Janota
 Released under the GPL license.

Overview
--------------------------------------------------------------------------------
packup  is a solver for the package upgradability problem specified in CUDF
[TZ09].  It repeatedly invokes an optimization pseudo-Boolean solver in order
to solve the problem.  By default minisat+ [ES06] is used for that purpose, but
a different solver can be used by specifying the pertaining command line
option.

Usage
--------------------------------------------------------------------------------
packup  [OPTIONS] instance_file_name [output_file_name]
	-t 	 trendy
	-p 	 paranoid
	-u cs 	 user criteria
	--external-solver	  command for the external solver
				  default 'minisat+ -ansi'
	--multiplication-string	  string between coefficients and variables 
				  when communicating to the solver, default '*'
	--temporary-directory DIR where temporary files should be placed
                                  default if $TMPDIR if exists, '/tmp' otherwise
	--leave-temporary-files	 do not delete temporary files
  NOTE
  If the input file is '-', input is read from the standard input.
  If the output filename is omitted, output is produced to the standard output.

References
-------------------------------------------------------------------------------
[ES06] Niklas Een and Niklas Sorensson. Translating Pseudo-Boolean Constraints 
       into SAT. SAT, 2006
[TZ09] Ralf Treinen and Stefano Zacchiroli. Common upgradeability description 
       format (CUDF) 2.0. Technical Report 003, MANCOOSI, November 200

--
Mikolas Janota, May 2011
