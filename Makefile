all:	regulatory.bin verify


regulatory.bin:	regulatory.sqlite dbgen.py
	@./dbgen.py

clean:
	@rm -f regulatory.sqlite regulatory.bin *~

regulatory.sqlite: db/*.sql
	@rm -f requlatory.sqlite
	@for i in db/*.sql ; do \
		if [ $$i = "db/00-database.sql" ] ; then continue ; fi ;\
		sed 's/AUTO_INCREMENT/AUTOINCREMENT/; s/use regulatory;//' < $$i | sqlite3 regulatory.sqlite ;\
	done

verify:	regulatory.sqlite verify.sh
	@./verify.sh
