#!/bin/sh

rm -f requlatory.sqlite
for i in *.sql ; do
	sed 's/AUTO_INCREMENT/AUTOINCREMENT/' < $i | sqlite3 regulatory.sqlite
done
