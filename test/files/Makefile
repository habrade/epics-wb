WBHEADERS = wbtest.h
WBINPUT = $(WBHEADERS:.h=wb)

all: header 

html: $(WBHEADERS:.h=.html)
db: $(WBHEADERS:.h=.db)
header: $(WBHEADERS)

clean:
	rm $(WBHEADERS)

%.h: %.wb
	wbgen2 --cstyle=extended  --co=$@ $<

%.db: %.wb
	wbgen2 --epicsdb=$@ $<

%.html: %.wb
	wbgen2 -f html --doco=$@ $<

%.vhdl: %.wb
	wbgen2  --vo=$@ $<
