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
DEST:=bin.$(GPP_VER)


OBJS=	$(DEST)/main.o \
		$(DEST)/Log.o \
		$(DEST)/logger.o \
		$(DEST)/SysLogger.o \
		$(DEST)/DirectoryList.o \
		$(DEST)/Mutex.o \
		$(DEST)/QStorage.o \
		$(DEST)/ServiceThread.o \
		$(DEST)/QManager.o \
		$(DEST)/ServerThread.o \
		$(DEST)/Signal.o \
		$(DEST)/thdlib.o \
		$(DEST)/uuidgen.o \
		$(DEST)/QAccessControl.o \
		$(DEST)/SystemDelivery.o \
		$(DEST)/param_config.o \
		$(DEST)/ForwardThread.o \
		$(DEST)/TransactionManager.o \
		$(DEST)/SystemConnection.o \
		$(DEST)/Command.o \
		$(DEST)/AddressParser.o \
		$(DEST)/Stats.o

INCFILES=	\
			safmq.h \
			safmq_defs.h \
			lib/MessageQueue.h \
			lib/MQConnection.h \
			lib/MQFactory.h \
			shared/bufstream.h

TCPSOCKET_INCFILES = \
			shared/tcpsocket/sslsocket.h \
			shared/tcpsocket/tcpsocket.h \
			shared/tcpsocket/socstream.h 

URL_INCFILES = shared/url/urldecode.h 
UUIDGEN_INCFILES = shared/uuidgen/uuidgen.h 


CFLAGS= -c \
	-Ishared \
	-I. \
	-DMULTI_THREAD_PROCESSING \
	-D_THREAD_SAFE \
	-D__USE_GNU \
	-DCONFIG_ENV=\"SAFMQ_CFG\" \
	-Wno-multichar

CFLAGS+= $(shell if [ $(GPP_MAG_VER) = 3.2 ]; then echo -DGNU_STREAM_SET_M_MODE; fi )

LDFLAGS=-lpthread -Llib/$(DEST) -lsafmqclient
BIN_NAME=safmq
TARGET=$(DEST)/$(BIN_NAME)

include makefile.in

$(TARGET): $(DEST) $(OBJS) lib/$(DEST)/libsafmqclient.a utilities/$(DEST)/safmqc $(PHP_SAFMQ_TARGET)
	@echo + Linking $@
	@g++ $(OBJS) $(LDFLAGS) -o $@
	@echo Build Complete =============================

makefile.in:
	@echo "Configuation not found, using default configuration settings"
	./configure

totally-clean: clean
	rm makefile.in

clean: clean-lib clean-util $(PHP_SAFMQ_CLEAN_TARGET)
	@echo + Cleaning
	@rm -f $(OBJS) $(TARGET)

clean-lib:
	@(cd lib; $(MAKE) clean)

clean-util:
	@(cd utilities; $(MAKE) clean)

all: clean $(TARGET)

install: $(TARGET)
	@if [ ! -d "$(INS_BIN_DIR)" ]; then mkdir -p "$(INS_BIN_DIR)"; fi
	@cp -f $(TARGET) "$(INS_BIN_DIR)"
	@cp -f utilities/$(DEST)/safmqc "$(INS_BIN_DIR)"
	@if [ ! -d "$(INS_LIB_DIR)" ]; then mkdir -p "$(INS_LIB_DIR)"; fi
	@cp -f lib/$(DEST)/libsafmqclient.a "$(INS_LIB_DIR)"
	@if [ ! -d "$(INS_INC_DIR)" ]; then mkdir -p "$(INS_INC_DIR)"; fi
	@cp -f $(INCFILES) "$(INS_INC_DIR)"
	@if [ ! -d "$(INS_INC_DIR)/tcpsocket" ]; then mkdir -p "$(INS_INC_DIR)/tcpsocket"; fi
	@cp -f $(TCPSOCKET_INCFILES) "$(INS_INC_DIR)/tcpsocket"
	@if [ ! -d "$(INS_INC_DIR)/url" ]; then mkdir -p "$(INS_INC_DIR)/url"; fi
	@cp -f $(URL_INCFILES) "$(INS_INC_DIR)/url"
	@if [ ! -d "$(INS_INC_DIR)/uuidgen" ]; then mkdir -p "$(INS_INC_DIR)/uuidgen"; fi
	@cp -f $(UUIDGEN_INCFILES) "$(INS_INC_DIR)/uuidgen"
	@echo "+ Install Complete"
	
uninstall:
	@rm -f "$(INS_BIN_DIR)/$(BIN_NAME)"
	@rm -f "$(INS_BIN_DIR)/safmqc"
	@rm -f "$(INS_LIB_DIR)/libsafmqclient.a"
	@for i in $(INCFILES); do \
		rm -f $(INS_INC_DIR)/`echo $$i |sed -e 's/.*\///g'`; \
		done
	@for i in $(TCPSOCKET_INCFILES); do \
		rm -f $(INS_INC_DIR)/tcpsocket/`echo $$i |sed -e 's/.*\///g'`; \
		done
	@for i in $(URL_INCFILES); do \
		rm -f $(INS_INC_DIR)/url/`echo $$i |sed -e 's/.*\///g'`; \
		done
	@for i in $(UUIDGEN_INCFILES); do \
		rm -f $(INS_INC_DIR)/uuidgen/`echo $$i |sed -e 's/.*\///g'`; \
		done
	@rmdir "$(INS_INC_DIR)/url"
	@rmdir "$(INS_INC_DIR)/tcpsocket"
	@rmdir "$(INS_INC_DIR)/uuidgen"
	@if [ "$(INS_BIN_DIR)" != /usr/local/bin -a "$(INS_BIN_DIR)" != /usr/bin ] ; then \
		rmdir "$(INS_BIN_DIR)"; \
		fi
	@if [ "$(INS_LIB_DIR)" != /usr/local/bin -a "$(INS_LIB_DIR)" != /usr/bin ] ; then \
		rmdir "$(INS_LIB_DIR)"; \
		fi
	@if [ "$(INS_INC_DIR)" != /usr/local/bin -a "$(INS_INC_DIR)" != /usr/bin ] ; then \
		rmdir "$(INS_INC_DIR)"; \
		fi
	@if [ "$(INS_PREFIX)" != /usr/local -a "$(INS_PREFIX)" != /usr ] ; then \
		rmdir "$(INS_PREFIX)"; \
		fi
	@echo "+ Uninstall Complete"



$(DEST):
	@echo + Creating output directory
	@if [ \! -d "$(DEST)" ]; then mkdir $(DEST); fi

lib/$(DEST)/libsafmqclient.a: safmq.h safmq_defs.h lib/*.cpp lib/*.h \
		shared/url/urldecode.h \
		shared/tcpsocket/*.h shared/tcpsocket/*.cpp 
	+ (cd lib; $(MAKE) )

utilities/$(DEST)/safmqc: safmq.h safmq_defs.h lib/*.cpp lib/*.h \
		shared/url/urldecode.h \
		shared/tcpsocket/*.h shared/tcpsocket/*.cpp \
		lib/$(DEST)/libsafmqclient.a
	+ (cd utilities; $(MAKE) )

.php_safmq_lib: lib/$(DEST)/libsafmqclient.a *.h lib/*.h php_safmq/Makefile
	+ (cd php_safmq; $(MAKE)  clean; $(MAKE) )
	@touch .php_safmq_lib

php_safmq/Makefile: php_safmq/config.m4
	+(cd php_safmq; phpize; $(PHP_VARS) ./configure)

php_safmq_lib_clean: 
	+ (cd php_safmq; $(MAKE)  clean; phpize --clean)

$(DEST)/DirectoryList.o: DirectoryList.cpp DirectoryList.h
	@echo + $@
	@g++ $(CFLAGS) DirectoryList.cpp -o $@

$(DEST)/Mutex.o: Mutex.cpp Mutex.h
	@echo + $@
	@g++ $(CFLAGS) Mutex.cpp -o $@

$(DEST)/param_config.o: param_config.cpp param_config.h 
	@echo + $@
	@g++ $(CFLAGS) param_config.cpp -o $@

$(DEST)/Signal.o: Signal.cpp safmq-Signal.h
	@echo + $@
	@g++ $(CFLAGS) Signal.cpp -o $@

$(DEST)/uuidgen.o: shared/uuidgen/uuidgen.c shared/uuidgen/uuidgen.h
	@echo + $@
	@g++ $(CFLAGS) shared/uuidgen/uuidgen.c -o $@

$(DEST)/thdlib.o: shared/thdlib.cpp shared/thdlib.h
	@echo + $@
	@g++ $(CFLAGS) shared/thdlib.cpp -o $@


##########################################################
# Core application modules

$(DEST)/main.o: main.cpp main.h *.h
	@echo + $@
	@g++ $(CFLAGS) main.cpp -o $@

$(DEST)/Log.o: Log.cpp Log.h *.h
	@echo + $@
	@g++ $(CFLAGS) Log.cpp -o $@

$(DEST)/logger.o: logger.cpp logger.h *.h
	@echo + $@
	@g++ $(CFLAGS) logger.cpp -o $@

$(DEST)/SysLogger.o: SysLogger.cpp SysLogger.h *.h
	@echo + $@
	@g++ $(CFLAGS) SysLogger.cpp -o $@

$(DEST)/QStorage.o: QStorage.cpp QStorage.h QManager.h StreamCache.h *.h
	@echo + $@
	@g++ $(CFLAGS) QStorage.cpp -o $@

$(DEST)/QManager.o: QManager.cpp QManager.h StreamCache.h *.h
	@echo + $@
	@g++ $(CFLAGS) QManager.cpp -o $@

$(DEST)/SystemConnection.o: SystemConnection.cpp SystemConnection.h lib/*.h *.h 
	@echo + $@
	@g++ $(CFLAGS) SystemConnection.cpp -o $@

$(DEST)/QAccessControl.o: QAccessControl.cpp QAccessControl.h *.h \
		shared/utilities.h main.h
	@echo + $@
	@g++ $(CFLAGS) QAccessControl.cpp -o $@

$(DEST)/ServiceThread.o: ServiceThread.cpp ServiceThread.h *.h \
		shared/tcpsocket/socstream.h shared/thdlib.h 
	@echo + $@
	@g++ $(CFLAGS) ServiceThread.cpp -o $@

$(DEST)/TransactionManager.o: TransactionManager.cpp TransactionManager.h *.h
	@echo + $@
	@g++ $(CFLAGS) TransactionManager.cpp -o $@

$(DEST)/ServerThread.o: ServerThread.cpp ServerThread.h *.h \
		shared/tcpsocket/socstream.h shared/thdlib.h 
	@echo + $@
	@g++ $(CFLAGS) ServerThread.cpp -o $@

$(DEST)/SSLServerThread.o: SSLServerThread.cpp SSLServerThread.h *.h \
		shared/tcpsocket/socstream.h shared/thdlib.h 
	@echo + $@
	@g++ $(CFLAGS) SSLServerThread.cpp -o $@

$(DEST)/SystemDelivery.o: SystemDelivery.cpp SystemDelivery.h lib/*.h *.h \
		shared/thdlib.h shared/url/urldecode.h
	@echo + $@
	@g++ $(CFLAGS) SystemDelivery.cpp -o $@

$(DEST)/ForwardThread.o: ForwardThread.cpp ForwardThread.h lib/*.h *.h \
		shared/thdlib.h shared/url/urldecode.h 
	@echo + $@
	@g++ $(CFLAGS) ForwardThread.cpp -o $@

$(DEST)/AddressParser.o: AddressParser.cpp AddressParser.h
	@echo + $@
	@g++ $(CFLAGS) AddressParser.cpp -o $@

$(DEST)/Command.o: Command.cpp Command.h *Cmds.h
	@echo + $@
	@g++ $(CFLAGS) Command.cpp -o $@

$(DEST)/Stats.o: Stats.cpp Stats.h
	@echo + $@
	@g++ $(CFLAGS) Stats.cpp -o $@


