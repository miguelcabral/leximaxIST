/* 
 * File:   ReadCNF.hh
 * Author: mikolas
 *
 * Created on November 11, 2010, 1:19 PM
 */

#ifndef READCNF_HH
#define	READCNF_HH

#include <zlib.h>
#include <vector>
#include "basic_types.h"
#include "basic_clset.hh"
#include "fmtutils.hh"
/*
    gzFile in = gzopen(file_name, "rb");
    if (in == Z_NULL) {
       exit(0);
    }
    ReadCNF reader(in);
    reader.read();
    gzclose(in);
*/


class ReadCNF {
public:
    explicit ReadCNF(gzFile& input_file);
    virtual ~ReadCNF();
    void read();
    LINT get_max_id() {return mxid;}
    BasicClauseSet get_clauses()  {return clauses;}
    std::vector<BasicClause*>& get_clause_vector() {return clause_vector;}
private:
     gzFile input_file;
     LINT mxid;
     std::vector<BasicClause*>  clause_vector;
     BasicClauseSet clauses;
     void read_cnf_clause(StreamBuffer& in, std::vector<LINT>& lits);
};
#endif	/* READCNF_HH */

