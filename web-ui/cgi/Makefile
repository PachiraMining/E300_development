#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

CFLAGS = 	-Wundef \
			-Wstrict-prototypes \
			-Wno-trigraphs \
			-g -O0 \
			-fno-inline \
			-fno-short-enums \
			-fpic

CPPFLAGS = 	-D__packed__= \
			-DIMAGE_APPS_PROC \
			-DFEATURE_Q_NO_SELF_QPTR \
			-DAEEINTERFACE_CPLUSPLUS \
			-DFEATURE_SMD \
			-DFEATURE_DIAG_MP_MASTER_APPS \
			-DFEATURE_DS_LINUX_ANDROID \
			-DFEATURE_DSS_LINUX \
			-DFEATURE_LINUX \
			-DFEATURE_MULTIPROCESSOR \
			-DFEATURE_DSUTILS_OFFTARGET \
			-DFEATURE_NO_REX \
			-DFEATURE_XMLLIB \
			-I../inc \
			$(CFLAGS) \

LIBS = -lpthread -lcrypt -lrt -lbsd			

RM = rm -f
RM_CMD = $(RM) *.BAK *.bak *.d *.o *.so ,* *~ *.a *.orig *.rej *.out

all: qcmap_web_cgi qcmap_auth

%.o: %.c
	$(CC) $(CFLAGS) -c $<

qcmap_web_cgi: qcmap_web_cgi.o
	$(CC)  $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

qcmap_auth: qcmap_auth.o
	$(CC)  $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

install:: all

clean::
	$(RM_CMD) qcmap_web_cgi qcmap_auth

.PHONY: all install clean dependents
.SECONDARY:
