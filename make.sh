# Gets all files that end in .c
files="$(find . -type f | grep -vE '\./\.git|\.gch' | grep '\.c' | sort -r | sed -n -e 'H;${x;s/\n/ /g;s/^,//;p;}')"

warnings="-Wextra -Wall -pedantic -Wno-implicit-fallthrough -Wno-switch"
eval "gcc $files $warnings $* -std=c17 -lreadline -o jeru.out"
