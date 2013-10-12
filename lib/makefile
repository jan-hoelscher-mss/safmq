###############################################################################
# Copyright 2004-2008 Matthew J. Battey
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software distributed
#	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
#	CONDITIONS OF ANY KIND, either express or implied. See the License for the
#	specific language governing permissions and limitations under the License.
#
#
#
#
#This software implements a platform independent Store and Forward Message Queue.
################################################################################

GPP_VER:=$(shell g++ -dumpversion)
GPP_MAG_VER:=$(shell g++ -dumpversion | cut -d "." -f 1,2)
DEST=bin.$(GPP_VER)

LIB_OBJS=	$(DEST)/MessageQueue.o  $(DEST)/MQConnection.o  $(DEST)/MQFactory.o  \
			$(DEST)/tcpsocket.o

SAFMQLOC=..
SHAREDSRC=../shared
CFLAGS= -c -w \
	-fPIC \
	-I$(SAFMQLOC) \
	-I$(SHAREDSRC) \
	-D_THREAD_SAFE \
	-D_REENTRANT \
	-D_PTHREADS
 
CFLAGS+= $(shell if [ $(GPP_MAG_VER) = 3.2 ]; then echo -DGNU_STREAM_SET_M_MODE; fi )

LDFLAGS=

LIB=$(DEST)/libsafmqclient.a

include ../makefile.in

$(LIB): $(DEST) $(LIB_OBJS)
	@echo + Building $@
	@ar -rc $@ $(LIB_OBJS)

client: $(TARGET)

clean:
	@echo + Cleaning
	@rm -f $(OBJS) $(TARGET) $(LIB) $(LIB_OBJS)

docs: doc doc/index.html

sf-docs: sf-doc sf-doc/index.html

all: clean $(LIB)

$(DEST):
	@echo + Creating output directory
	@if [ \! -d "$(DEST)" ]; then mkdir $(DEST); fi

doc:
	@echo + Creating documentation directory
	@if [ \! -d "doc" ]; then mkdir doc; fi

doc/index.html: ../safmq.h ../safmq_defs.h ../quectrl.h *.h *.cpp ../shared/tcpsocket/*.h ../shared/tcpsocket/*.cpp footer.html
	@echo Building documentation
	@doc++ --title="SAFMQ Documentation" --sort --dir=doc --html \
			--before-class --before-group --footer footer.html \
			../safmq.h *.h *.cpp ../shared/tcpsocket/*.h ../shared/tcpsocket/*.cpp \
			../safmq_defs.h ../quectrl.h php_safmq.help

sf-doc:
	@echo + Creating documentation directory
	@if [ \! -d "doc" ]; then mkdir doc; fi

sf-doc/index.html: ../safmq.h ../safmq_defs.h ../quectrl.h *.h *.cpp ../shared/tcpsocket/*.h ../shared/tcpsocket/*.cpp footer.html
	@echo Building documentation
	@doc++ --title="SAFMQ Documentation" --sort --dir=sf-doc --html \
			--before-class --before-group --footer footer.sf.html \
			../safmq.h *.h *.cpp ../shared/tcpsocket/*.h ../shared/tcpsocket/*.cpp \
			../safmq_defs.h ../quectrl.h php_safmq.help

$(DEST)/tcpsocket.o: $(SHAREDSRC)/tcpsocket/tcpsocket.cpp \
				$(SHAREDSRC)/tcpsocket/tcpsocket.h
	@echo + $@
	@g++ $(CFLAGS) $(SHAREDSRC)/tcpsocket/tcpsocket.cpp -o $@

$(DEST)/sslsocket.o: $(SHAREDSRC)/tcpsocket/sslsocket.cpp \
				$(SHAREDSRC)/tcpsocket/sslsocket.h
	@echo + $@
	@g++ $(CFLAGS) $(SHAREDSRC)/tcpsocket/sslsocket.cpp -o $@

$(DEST)/SSLLockingPthread.o: $(SHAREDSRC)/tcpsocket/SSLLockingPthread.cpp \
				$(SHAREDSRC)/tcpsocket/SSLLocking.h
	@echo + $@
	@g++ $(CFLAGS) $(SHAREDSRC)/tcpsocket/SSLLockingPthread.cpp -o $@


$(DEST)/MessageQueue.o: MessageQueue.cpp MessageQueue.h MQConnection.h $(SAFMQLOC)/safmq.h $(SAFMQLOC)/safmq_defs.h
	@echo + $@
	@g++ $(CFLAGS) MessageQueue.cpp -o $@

$(DEST)/MQConnection.o: MQConnection.cpp MQConnection.h $(SAFMQLOC)/safmq.h $(SAFMQLOC)/safmq_defs.h $(SAFMQLOC)/comdefs.h
	@echo + $@
	@g++ $(CFLAGS) MQConnection.cpp -o $@

$(DEST)/MQFactory.o: MQFactory.cpp MQFactory.h MQConnection.h MessageQueue.h \
	$(SHAREDSRC)/url/urldecode.h $(SAFMQLOC)/safmq.h $(SAFMQLOC)/safmq_defs.h
	@echo + $@
	@g++ $(CFLAGS) MQFactory.cpp -o $@

