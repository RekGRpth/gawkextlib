#------------------------------------------------------------------
# csv --- additional functions to generate CSV data
#
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Created: May 2016
#
# Prefix for user seeable items:  csv
# Prefix for internal only items: _csv_
#------------------------------------------------------------------

# Load the CSV extension by default
@load "csv"

#------------------------------------------------------------------
# Functions for CSV format generation
#------------------------------------------------------------------

# Quote a CSV field value, if necessary
function csvquote(str, comma, quote) {
    if (length(comma)==0) comma = ","
    if (length(quote)==0) quote = "\""    #"
    if (index(str, comma) || index(str, quote) || index(str, "\n")) {
        gsub(quote, quote quote, str)
        str = quote str quote
    }
    return str
}

# Compose a CSV record from an array of fields
function csvcompose(af, comma, quote,        record, k, sep) {
    if (length(comma)==0) comma = ","
    if (length(quote)==0) quote = "\""    #"
    record = ""
    sep = ""
    for (k=1; k in af; k++) {
        record = record sep csvquote(af[k], comma, quote)
        sep = comma
    }
    return record
}

# Reformat a normal record as a CSV one
function csvformat(record, fs, comma, quote,     af) {
    split(record, af, fs)
    return csvcompose(af, comma, quote)
}

#------------------------------------------------------------------
# CSVMODE aware record printing
#------------------------------------------------------------------

function csvprint(csvrecord, fs, comma, quote) {
    if (csvrecord == "" && csvrecord == 0) {  # Null value
        if (_csv_mode) {
            print CSVRECORD
        } else {
            print csvformat($0, FS, CSVCOMMA, CSVQUOTE)
        }
    } else {
        fs = fs ? fs : (_csv_mode ? CSVFS : FS)
        comma = comma ? comma : CSVCOMMA
        quote = quote ? quote : CSVQUOTE
        print csvformat(csvrecord, fs, comma, quote)
    }
}

#------------------------------------------------------------------
# Access CSV fields by name
#------------------------------------------------------------------

function csvfield(name, missing) {
    if (name in _csv_column) {
        return $_csv_column[name]
    } else {
        return missing
    }
}

##################################################################
# Emulation of built-in features not yet implemented
##################################################################

# Test CSVMODE at beginfile and override FS for API v1
BEGINFILE {
    if (CSVMODE) {
        _csv_mode = 1
        _csv_save_fs = FS
        _csv_save_ofs = OFS
        FS = OFS = CSVFS
        delete _csv_column
    } else {
        _csv_mode = 0
    }
}

ENDFILE {
    if (_csv_mode) {
        FS = _csv_save_fs
        OFS = _csv_save_ofs
    }
}

# Record header labels
_csv_mode  && FNR==1 {
    for (k=1; k<=NF; k++) _csv_column[$k] = k
}
