all:	regulatory.bin


regulatory.bin:	regulatory.sqlite dbgen.py key.priv.pem
	@./dbgen.py

clean:
	@rm -f regulatory.sqlite regulatory.bin *~

regulatory.sqlite: db/*.sql
	@rm -f requlatory.sqlite
	@(for i in db/*.sql ; do \
		if [ $$i = "db/00-database.sql" ] ; then continue ; fi ;\
		sed 's/AUTO_INCREMENT/AUTOINCREMENT/; s/use regulatory;//' < $$i ;\
	done) | sqlite3 regulatory.sqlite

generate_keys:
	openssl genrsa -out key.priv.pem 2048
