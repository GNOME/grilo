#
# generate_core_keys.py
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L.
#
# Generates a .c file containing an array with the core keys
#

import re
import sys


COMMENT_RE = re.compile(
    r'''
    //.*?$ | /\*.*?\*/ | \'(?:\\. | [^\\\'])*\' | "(?:\\. | [^\\"])*"
    ''',
    re.DOTALL | re.MULTILINE)


# From https://stackoverflow.com/a/241506
def comment_remover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " "  # note: a space and not an empty string
        else:
            return s
    return re.sub(COMMENT_RE, replacer, text)


if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <input> <output>")
    exit(1)

with open(sys.argv[1], "r") as finput:
    input_data = finput.read()
    input_data = comment_remover(input_data).split('\n')

with open(sys.argv[2], "w") as foutput:
    foutput.write("gchar *grl_core_keys[] = {\n")

    output_keys = False
    for line in input_data:
        if re.search("GRL_METADATA_KEY_ALBUM", line):
            output_keys = True
        if re.search("G_BEGIN_DECLS", line):
            output_keys = False
        m = re.search("GRL_METADATA_KEY_[^ ]+", line)
        if output_keys and m:
            foutput.write('"' + m.group(0) + '",\n')

    foutput.write("};")
