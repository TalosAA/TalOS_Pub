SH := bash

all: iso

iso:
	$(SH) iso.sh DEBUG

release:
	$(SH) iso.sh RELEASE

debug:
	$(SH) iso.sh DEBUG

rebuild: clean
	$(SH) iso.sh RELEASE

rebuild-debug: clean
	$(SH) iso.sh DEBUG

clean-release:
	$(SH) clean.sh RELEASE

clean-debug:
	$(SH) clean.sh DEBUG

clean:
	$(SH) clean.sh DEBUG
	$(SH) clean.sh RELEASE