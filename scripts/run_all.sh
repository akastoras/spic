#!/bin/bash

#---Run Course tests with dense methods---#

# Use integrated direct methods
python3 scripts/test_all.py tests/course                             --version 0
# Use custom direct methods
python3 scripts/test_all.py tests/course --custom                    --version 0
# Use integrated iterative methods
python3 scripts/test_all.py tests/course          --iter --itol=1e-6 --version 0
# Use custom iterative methods
python3 scripts/test_all.py tests/course --custom --iter --itol=1e-6 --version 0


#---Run Course tests with sparse methods---#

# Use integrated direct methods
python3 scripts/test_all.py tests/course --sparse                              --version 0
# Use integrated iterative methods
python3 scripts/test_all.py tests/course --sparse           --iter --itol=1e-6 --version 0
# Use custom iterative methods
python3 scripts/test_all.py tests/course --sparse  --custom --iter --itol=1e-6 --version 0


#---Run IMB tests with sparse methods---#

# Use integrated direct methods
python3 scripts/test_all.py tests/ibm --sparse                              --version 0
# Use integrated iterative methods
python3 scripts/test_all.py tests/ibm --sparse           --iter --itol=1e-6 --version 0
# Use custom iterative methods
python3 scripts/test_all.py tests/ibm --sparse  --custom --iter --itol=1e-6 --version 0
