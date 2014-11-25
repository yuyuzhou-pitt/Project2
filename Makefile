all:
	(cd port-mapper; make all)
	(cd server; make all)
	(cd client; make all)
	(cd tool; make all)
clean:
	(cd port-mapper; make clean)
	(cd server; make clean)
	(cd client; make clean)
	(cd tool; make clean)
