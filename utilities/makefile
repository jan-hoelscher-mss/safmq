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

CFLAGS= -I.. -I../shared
LDFLAGS=-L../lib/$(DEST) -lsafmqclient

TARGET=$(DEST)/safmqc

include ../makefile.in

$(DEST)/safmqc: $(DEST) safmqc.cpp ../*.h ../*.cpp ../lib/$(DEST)/libsafmqclient.a 
	@echo + compiling and linking safmqc
	@g++ $(CFLAGS) safmqc.cpp $(LDFLAGS) -o $(DEST)/safmqc

$(DEST)/_getopt_long.o: _getopt_long.c _getopt.h
	@echo + $@
	@gcc $(CFLAGS) _getopt_long.c -o $@

$(DEST):
	@echo + Making output directory
	@if [ ! -d $(DEST) ]; then mkdir $(DEST); fi

all: clean $(TARGET)

clean:
	@echo + Cleaning
	@rm -f $(TARGET)
