#------------------------------------------------------------------
# csv --- companion library for the gawk-csv extension
#
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Created: May 2016
# Updated: Mar 2018
#
# Prefix for user seeable items:  csv
# Prefix for internal only items: _csv_
#------------------------------------------------------------------

# Load the CSV extension by default
@load "csv"

#------------------------------------------------------------------
# Extract clean values from CSV formatted data
#------------------------------------------------------------------

# Extract the clean value of a CSV field
function csvunquote(csvfield, quote) {
    quote = quote ? quote : (CSVQUOTE ? CSVQUOTE : "\"")    #"
    if (csvfield ~ ("^" quote ".*" quote "$")) {
        csvfield = substr(csvfield, 2, length(csvfield)-2)
        gsub(quote quote, quote, csvfield)
    }
    return csvfield
}

#------------------------------------------------------------------
# Functions for CSV format generation
#------------------------------------------------------------------

# Quote a CSV field value, if necessary
function csvquote(str, comma, quote) {
    comma = comma ? comma : (CSVCOMMA ? CSVCOMMA : ",")
    quote = quote ? quote : (CSVQUOTE ? CSVQUOTE : "\"")    #"
    if (index(str, comma) || index(str, quote) || index(str, "\n")) {
        gsub(quote, quote quote, str)
        str = quote str quote
    }
    return str
}

# Compose a CSV record from an array of fields
function csvcompose(af, comma, quote,        record, k, sep) {
    comma = comma ? comma : (CSVCOMMA ? CSVCOMMA : ",")
    quote = quote ? quote : (CSVQUOTE ? CSVQUOTE : "\"")    #"
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
    fs = fs ? fs : FS
    split(record, af, fs)
    return csvcompose(af, comma, quote)
}

#------------------------------------------------------------------
# CSVMODE aware record printing
#------------------------------------------------------------------

# Print a CSV formatted record
function csvprint(csvrecord, fs, comma, quote) {
    if (csvrecord == "" && csvrecord == 0) {  # Null value
        if (_csv_mode) {
            print CSVRECORD
        } else {
            print csvformat($0, FS, CSVCOMMA, CSVQUOTE)
        }
    } else {
        fs = fs ? fs : (_csv_mode ? CSVFS : FS)
        print csvformat(csvrecord, fs, comma, quote)
    }
}

# Print the current record
function csvprint0() {
    if (_csv_mode) {
        print CSVRECORD
    } else {
        print
    }
}

#------------------------------------------------------------------
# Access CSV fields by name
#------------------------------------------------------------------

function csvfield(name, missing) {
    if (name in _csv_label) {
        return $_csv_label[name]
    } else {
        return missing
    }
}

##################################################################
# Emulation of built-in features not implemented by the API
##################################################################

# Test CSVMODE at BEGINFILE and override FS and OFS
BEGINFILE {
    if (CSVMODE) {
        _csv_mode = 1
        _csv_save_fs = FS
        _csv_save_ofs = OFS
        FS = OFS = CSVFS
        delete _csv_label
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
    for (k=1; k<=NF; k++) _csv_label[$k] = k
}
