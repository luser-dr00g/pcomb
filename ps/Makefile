#SRC=struct2.ps pc11a.ps pc11are.ps pc11atoken.ps
SRC=Makefile struct2.ps pc12.ps pc12re.ps pc12token.ps pc12json.ps pc12ctok.ps pc12csv.ps
GS=gsnd -q -dNOSAFER

list :
	a2ps -2 -f6 --delegate=no -o source.ps $(SRC)
	ps2pdf source.ps

pc12_test :
	$(GS) -ddebug-pc pc12.ps
	$(GS) -ddebug-re pc12re.ps
	$(GS) -ddebug-token pc12token.ps
	$(GS) -ddebug-json pc12json.ps
	$(GS) -ddebug-ctok pc12ctok.ps

pc9_test :
	$(GS) pc9json.ps


