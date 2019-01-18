# Gets all files that end in .c
files="$(find . -type f | grep -vE '\./\.git|\.gch' | grep '\.c' | sort -r | sed -n -e 'H;${x;s/\n/ /g;s/^,//;p;}')"

eval "gcc $files -Wextra -Wall -pedantic -o jeru.out"
