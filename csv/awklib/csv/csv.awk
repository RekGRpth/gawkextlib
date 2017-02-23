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

function csvprint() {
    if (CSVMODE) {
        print CSVRECORD
    } else {
        print
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

# Test CSVMODE at beginfile and record parse options
BEGINFILE {
    if (CSVMODE) {
        _csv_mode = 1
        if (CSVCOMMA "" != "") {
            _csv_comma = substr(CSVCOMMA, 1, 1)
###         _csv_comma = CSVCOMMA    #####################################################################
        } else {
            _csv_comma = ","
        }
        if (CSVQUOTE "" != "") {
            _csv_quote = substr(CSVQUOTE, 1, 1)
        } else {
            _csv_quote = "\""                      #"
        }
        if (CSVFS "" != "") {
            _csv_fs = substr(CSVFS, 1, 1)
        } else {
            _csv_fs = "\0"
        }
        _csv_save_fs = FS
        FS = _csv_fs
    } else {
        _csv_mode = 0
    }
}

ENDFILE {
    if (_csv_mode) FS = _csv_save_fs
	delete _csv_column
}

# Transform CSV records
_csv_mode {
    # Collect multi-line data, if it is the case
    while (gsub("\"", "\"", $0) % 2 == 1 && (getline _csv_) > 0) {
        $0 = $0 "\n" _csv_
        NR--
        FNR--
    }
    # Convert the CSV record
    CSVRECORD = $0
    $0 = csvconvert($0, _csv_fs, _csv_comma, _csv_quote)
    if (FNR==1) {
        for (k=1; k<=NF; k++) _csv_column[$k] = k
    }
}
