DIRS       = lib \
	     aos \
	     NeXT \
	     src \
	     convers \
	     util \
	     bbs

all:;   @-chmod 755 cc
	@-for dir in $(DIRS); do ( cd $$dir; $(MAKE) -i all install ); done
	@-$(MAKE) -i /tcp/hostaddr.pag
	@-if [ -d tools ]; then ( cd tools; $(MAKE) -i all install ); fi

/tcp/hosts:
	[ -f /tcp/hosts ] || touch /tcp/hosts

/tcp/domain.txt:
	[ -f /tcp/domain.txt ] || touch /tcp/domain.txt

/tcp/hostaddr.pag: /tcp/hosts /tcp/domain.txt util/mkhostdb
	rm -f /tcp/hostaddr.* /tcp/hostname.*
	util/mkhostdb >/dev/null 2>&1
	if [ -f /tcp/hostaddr.db ]; then ln /tcp/hostaddr.db $@; fi

clean:; @-for dir in $(DIRS); do ( cd $$dir; $(MAKE) -i clean ); done
	@-if [ -d tools ]; then  ( cd tools; $(MAKE) -i clean ); fi
	@-find . -name .trimtime -exec rm {} \;
###
