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

if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <input> <output>")
    exit(1)

finput = open(sys.argv[1], "r")
foutput = open(sys.argv[2], "w")

foutput.write("gchar *grl_core_keys[] = {\n")

output_keys = False
for line in finput:
    if re.search("BEGIN CORE KEYS", line):
        output_keys = True
    if re.search("END CORE KEYS", line):
        output_keys = False
    m = re.search("GRL_METADATA_KEY_[^ ]+", line)
    if output_keys and m:
            foutput.write("\"" +  m.group(0) + "\",\n")

foutput.write("};");

finput.close()
foutput.close()
