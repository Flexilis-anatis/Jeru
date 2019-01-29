# Gets all files that end in .c
files="$(find . -type f | grep -vE '\./\.git|\.gch' | grep '\.c' | sort -r | sed -n -e 'H;${x;s/\n/ /g;s/^,//;p;}')"

#                                 for murmur implementation
warnings="-Wextra -Wall -pedantic -Wno-implicit-fallthrough"
# Will pass args to gcc
eval "gcc $files $warnings $* -std=c17 -lreadline -lm -O2 -o jeru.out"
