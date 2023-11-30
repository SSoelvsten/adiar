# ---------------------------------------------------------------------------- #
#                          UPDATING FILES FOR RELEASES                         #
# ---------------------------------------------------------------------------- #
from datetime import datetime
import re

version = input("Version String: ").lower()
today   = datetime.today().strftime('%Y-%m-%d')

yes_choices = ['yes', 'y']
no_choices  = ['no',  'n']

# ---------------------------------------------------------------------------- #
#                                  CITATION.cff                                #
# ---------------------------------------------------------------------------- #
file_name = "CITATION.cff"

if input(f"Update {file_name}? (yes/No): ").lower() in yes_choices:
    content = ""

    with open(file_name, 'r') as in_file:
        content = in_file.read()

    with open(file_name, 'w') as out_file:
        content = re.sub(r"\ndate: .*\n",    f"\ndate: {today}\n",    content)
        content = re.sub(r"\nversion: .*\n", f"\nversion: {version}\n", content)

        out_file.write(content)

# ---------------------------------------------------------------------------- #
#                                 CMakeLists.txt                               #
# ---------------------------------------------------------------------------- #
file_name = "CMakeLists.txt"

if input(f"Update {file_name}? (yes/No): ").lower() in yes_choices:
    content = ""

    with open(file_name, 'r') as in_file:
        content = in_file.read()

    with open(file_name, 'w') as out_file:
        content = re.sub(r"  VERSION .+\n", f"  VERSION {version}\n", content)

        out_file.write(content)

