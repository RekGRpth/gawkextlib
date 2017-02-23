#ifndef CSV_SPLIT_H__
#define CSV_SPLIT_H__

/* Allow use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

int csv_split_record(const char* s, awk_array_t af, char comma, char quote, char options);

#ifdef __cplusplus
}
#endif	/* C++ */

#endif	/* CSV_SPLIT_H__ */
