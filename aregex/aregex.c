/*
 * aregex.c - Gawk extension to access the TRE approximate regex.
 * Copyright (C) 2018-9 Cam Webb, <cw@camwebb.info>
 * Distributed under the GNU Pulbic Licence v3
 */

// Minimal headers:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <gawkapi.h>
#include <tre/tre.h>
#include "common.h"

#define MAXNSUBMATCH 20 // Max Number of parenthetical substring matches
#define DEFMAXCOST 5    // Default max_cost for match 
#define DEBUG 0         // Print debug info


// Main amatch() function definition
static awk_value_t * do_amatch(int nargs, awk_value_t *result \
                               API_FINFO_ARG)
{
  int i; 
  
  // 1. Set default costs
  const char *parami[8];
  int paramv[8];
  parami[0] = "cost_ins";   paramv[0] = 1;
  parami[1] = "cost_del";   paramv[1] = 1;
  parami[2] = "cost_subst"; paramv[2] = 1;
  parami[3] = "max_cost";   paramv[3] = DEFMAXCOST;
  parami[4] = "max_del";    paramv[4] = DEFMAXCOST;
  parami[5] = "max_ins";    paramv[5] = DEFMAXCOST;
  parami[6] = "max_subst";  paramv[6] = DEFMAXCOST;
  parami[7] = "max_err";    paramv[7] = DEFMAXCOST;
  
  // 2. Read 3rd, 'costs' argument, if present
  //   (these variable declarations outside, because needed during output: )
  awk_value_t costs;
  awk_value_t simplecost;
  awk_value_t costindex;
  awk_value_t costval;
  awk_bool_t hascostarr = 0;
  
  if (nargs > 2) {
    // if just a simple integer for 3rd argument:
    if (get_argument(2, AWK_NUMBER, &simplecost)) {
      paramv[3] = (int) simplecost.num_value;
      paramv[4] = (int) simplecost.num_value;
      paramv[5] = (int) simplecost.num_value;
      paramv[6] = (int) simplecost.num_value;
      paramv[7] = (int) simplecost.num_value;
    }
    else if (get_argument(2, AWK_ARRAY, &costs)) {
      hascostarr = 1;
    
      char c[30];
      for (i = 0; i < 8; i++) {
        // create an index for reading array
        make_const_string(parami[i], strlen(parami[i]), &costindex);
        // if there is an array element with that index
        if (get_array_element(costs.array_cookie, &costindex,   \
                              AWK_STRING, &costval)) {
          // update the cost value
          paramv[i] = atoi(costval.str_value.str);
          if (DEBUG) {
            strcpy(c,"") ;
            sprintf(c, "cost %s = %d", parami[i], atoi(costval.str_value.str));
            warning(ext_id, c);
          }
        }
      }
    }
    else fatal(ext_id, "amatch: 3rd argument present, but could not be read.");
  }

  // 3. Read the string and regex arguments (1st and 2nd)
  awk_value_t re;
  awk_value_t str;
  if (!get_argument(0, AWK_STRING, &str))
    fatal(ext_id, "amatch: 1st param., the string, not found");
  if (!get_argument(1, AWK_STRING, &re))
    fatal(ext_id, "amatch: 2nd param., the regex, not found");

  // ( for wchar_t:
  //   wchar_t rew[] = L"";
  //   swprintf(rew, strlen(re.str_value.str), L"%ls", re.str_value.str); )
  
  // 4. Compile regex
  regex_t preg;
  tre_regcomp(&preg, re.str_value.str, REG_EXTENDED);

  // ( for wchar_t:
  //   tre_regwcomp(&preg, rew, REG_EXTENDED); )

  // 5. Do the match
  // set approx match params
  regaparams_t params = { 0 };
  params.cost_ins   = paramv[0]; 
  params.cost_del   = paramv[1];
  params.cost_subst = paramv[2];
  params.max_cost   = paramv[3];
  params.max_del    = paramv[4];
  params.max_ins    = paramv[5];
  params.max_subst  = paramv[6];
  params.max_err    = paramv[7];

  // create necessary structure for details of match
  regamatch_t match ;
  match.nmatch = MAXNSUBMATCH; 
  match.pmatch = (regmatch_t *) malloc(MAXNSUBMATCH * sizeof(regmatch_t));
  
  // do the approx regexp itself!
  int treret;
  treret = tre_regaexec(&preg, str.str_value.str, &match, params, 0);

  // ( for wchar_t:
  //   treret = tre_regawexec(&pregw, rew, &match, params, 0); )
  
  // set the amatch() return value depending on tre_regaexec() return
  //   1 if success, 0 if no match
  int rval = 1;
  if (treret == REG_NOMATCH) rval = 0;

  // Catch a "mem. not. allocated" return from tre_regaexec()
  if (treret == REG_ESPACE) {
    warning(ext_id,                                                     \
            "amatch: TRE err., mem. insufficient to complete the match.");
    free(match.pmatch);
    tre_regfree(&preg);
    return make_null_string(result);
  }

  // 6. If there is a cost array, set some return values (if a match)
  if ((hascostarr) && (rval)) {
    char matchcost[20]; // Single integers, max width ~= 10
    // cost
    del_array_element(costs.array_cookie,                           \
          make_const_string("cost", strlen("cost"), &costindex));
    sprintf(matchcost, "%d", match.cost);
    set_array_element(costs.array_cookie, \
          make_const_string("cost", strlen("cost"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
    // num_ins
    del_array_element(costs.array_cookie,                               \
          make_const_string("num_ins", strlen("num_ins"), &costindex));
    sprintf(matchcost, "%d", match.num_ins);
    set_array_element(costs.array_cookie, \
          make_const_string("num_ins", strlen("num_ins"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
    // num_del
    del_array_element(costs.array_cookie,                               \
          make_const_string("num_del", strlen("num_del"), &costindex));
    sprintf(matchcost, "%d", match.num_del);
    set_array_element(costs.array_cookie, \
          make_const_string("num_del", strlen("num_del"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
    // num_subst
    del_array_element(costs.array_cookie,                               \
          make_const_string("num_subst", strlen("num_subst"), &costindex));
    sprintf(matchcost, "%d", match.num_subst);
    set_array_element(costs.array_cookie, \
          make_const_string("num_subst", strlen("num_subst"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
  }
  
  // 7. Set 4th argument array, for matched substrings, if present
  //    and if a match found
  if ((nargs == 4) && (rval)) {
    awk_value_t substr; 
    // read 4th argument
    if (!get_argument(3, AWK_ARRAY, &substr)) {
      warning(ext_id, "amatch: Could not read 4th argument.");
    }
    else clear_array(substr.array_cookie);

    // hand the TRE substrings over to the gawk substring array
    char outindexc[20];
    int maxsubsize = 0;
    // first, find max size of substring
    for (i = 0; i < MAXNSUBMATCH ; i++)
      if (match.pmatch[i].rm_so != -1)
        if (maxsubsize < match.pmatch[i].rm_eo - match.pmatch[i].rm_so)
          maxsubsize = match.pmatch[i].rm_eo - match.pmatch[i].rm_so;
    // dimension the substring:
    char outvalc[maxsubsize+1];
    awk_value_t outindexp;
    awk_value_t outvalp;

    for (i = 0 ; i < (int) match.nmatch; i++) {
      if (match.pmatch[i].rm_so != -1) {
        sprintf(outindexc, "%d", i);
        // ( "%d %.*s", match.pmatch[i].rm_so+1, ... gives position
        //   by bytes, not by chars )
        sprintf(outvalc, "%.*s",                                 \
                match.pmatch[i].rm_eo - match.pmatch[i].rm_so,   \
                str.str_value.str + match.pmatch[i].rm_so);
        set_array_element(substr.array_cookie,                        
          make_const_string(outindexc, strlen(outindexc), &outindexp), \
          make_const_string(outvalc, strlen(outvalc), &outvalp));
      }
    }
  }
  
  free(match.pmatch);
  tre_regfree(&preg);
  return make_number(rval, result);
}

// Gawkextlib boilerplate:

static awk_ext_func_t func_table[] = \
  {
    { "amatch", do_amatch, 4, 2, awk_false, NULL  },
  };

static awk_bool_t (*init_func)(void) = NULL;

static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, amatch, "")
