all:	regulatory.bin

regulatory.bin:	db2bin.py key.priv.pem db.txt dbparse.py
	@./db2bin.py

clean:
	@rm -f regulatory.bin *~ *.pyc

generate_keys:
	openssl genrsa -out key.priv.pem 2048
