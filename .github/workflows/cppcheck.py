#!/usr/bin/env python

import sys

ignored_lines = [
    "nofile:0:0: information: Cppcheck cannot find all the include files (use --check-config for details) [missingIncludeSystem]\n",
    "\n"
]

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("Missing filename argument")
        exit(1)

    filename = str(sys.argv[1])
    unignored_lines = []

    with open(filename, "r") as f:
        unignored_lines = [l for l in f.readlines() if not l in ignored_lines]

    if len(unignored_lines) > 0:
        with open(filename, "w") as f:
            f.writelines(["Cppcheck\n","--------------\n","\n"] + unignored_lines)

        print("Cppcheck found problems!")
        print("------------------------------------------------------")
        for l in unignored_lines:
            print(l)
        print("------------------------------------------------------")
        exit(1)

    print("All good!")
    exit(0)
