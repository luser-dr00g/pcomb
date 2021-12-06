SRC=struct2.ps pc11a.ps pc11are.ps pc11atoken.ps
GS=gsnd -q -dNOSAFER

list :
	a2ps -2 -f6 --delegate=no -o source.ps $(SRC)
	ps2pdf source.ps

pc12_test :
	$(GS) -ddebug-pc pc12.ps
	$(GS) -ddebug-re pc12re.ps
	$(GS) -ddebug-token pc12token.ps
	$(GS) -ddebug-json pc12json.ps

pc9_test :
	$(GS) pc9json.ps


