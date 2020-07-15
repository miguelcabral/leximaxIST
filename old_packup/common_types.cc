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
/* Copyright (C) 2011, Mikolas Janota */
#include <assert.h>
#include <iostream>
#include "common_types.hh"

using namespace version_operators;

bool version_operators::evaluate(Version version_1, Operator op, Version version_2) {
    switch (op) {
        case VERSIONS_NONE: return true;
        case VERSIONS_EQUALS: return version_1 == version_2;
        case VERSIONS_NOT_EQUALS: return version_1 != version_2;
        case VERSIONS_GREATER_EQUALS: return version_1 >= version_2;
        case VERSIONS_GREATER: return version_1 > version_2;
        case VERSIONS_LESS_EQUALS: return version_1 <= version_2;
        case VERSIONS_LESS: return version_1 < version_2;
    }
    assert(false);
    return NULL;
}

const char* version_operators::to_string(Operator op) {
    switch (op) {
        case VERSIONS_NONE: return "";
        case VERSIONS_EQUALS: return "=";
        case VERSIONS_NOT_EQUALS: return "!=";
        case VERSIONS_GREATER_EQUALS: return ">=";
        case VERSIONS_GREATER: return ">";
        case VERSIONS_LESS_EQUALS: return "<=";
        case VERSIONS_LESS: return "<";
    }
    assert(false);
    return NULL;
}

const string to_string(const Objective& o) {
    return string(o.second ? "+" : "-") + to_string(o.first);
}

const char* to_string(OBJECTIVE_FUNCTION f) {
    switch (f) {
        case COUNT_NEW: return "new";
        case COUNT_UNMET_RECOMMENDS: return "unmet_recommends";
        case COUNT_REMOVED: return "removed";
        case COUNT_NOT_UP_TO_DATE: return "notuptodate";
        case COUNT_CHANGED: return "changed";
    }
    assert(false);
    return NULL;
}

void print(const vector<Objective>& ls, ostream& o) {
    bool f = true;

    FOR_EACH(vector<Objective>::const_iterator, i, ls) {
        if (!f) o << ',';
        f = false;
        o << to_string(*i);
    }
}

const char* to_string(KeepValue value) {
    switch (value) {
        case KEEP_NONE: return "KEEP_NONE";
        case KEEP_VERSION: return "KEEP_VERSION";
        case KEEP_PACKAGE: return "KEEP_PACKAGE";
        case KEEP_FEATURE: return "KEEP_FEATURE";
    }
    assert(false);
    return NULL;
}

const char* to_string(Criterion value) {
    switch (value) {
        case NO_OPTIMIZATION: return "no_optimization";
        case PARANOID: return "paranoid";
        case TRENDY: return "trendy";
    }
    assert(false);
    return NULL;
}
