#!/bin/sh

. "$SOURCE"/tap/libtap.sh
cd "$BUILD"

TMPDIR=$(test_tmpdir)
TESTS_DIR="$SOURCE"/data
ZSCANNER_TOOL="$BUILD"/zscanner-tool

plan 68

mkdir -p "$TMPDIR"/includes/
for a in 1 2 3 4 5 6; do
    cat "$TESTS_DIR"/includes/include"$a" > "$TMPDIR"/includes/include"$a";
done

for case in $(cat "$SOURCE"/zscanner-TESTS); do
    casein=$(test_file_path data/"$case".in)
    caseout=$(test_file_path data/"$case".out)
    filein="$TMPDIR"/"$case".in
    fileout="$TMPDIR"/"$case".out

    sed -e "s|@TMPDIR@|$TMPDIR|;s|@SOURCE@|$SOURCE|;" < "$casein" > "$filein"
    diag $(ls "$filein")

    "$ZSCANNER_TOOL" -m 2 . "$filein" > "$fileout"

    if cmp -s "$fileout" "$caseout"; then
	ok "$case: output matches" true
	rm "$filein"
	rm "$fileout"
    else
	ok "$case: output differs" false
	diff -urNap "$caseout" "$fileout" | while read line; do diag "$line"; done
    fi
done

rm -rf "$TMPDIR"/includes/
