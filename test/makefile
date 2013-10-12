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
# This software implements a platform independent Store and Forward Message Queue.
################################################################################

GPP_VER:=$(shell g++ -dumpversion)
GPP_MAG_VER:=$(shell g++ -dumpversion | cut -d "." -f 1,2)
DEST:=bin.$(GPP_VER)

TARGET=SafmqTest

CFLAGS= -I.. -I../shared -I/usr/local/include -g
LDFLAGS=-L../lib/$(DEST) -L/usr/local/lib -lsafmqclient -lcppunit -ldl

include ../makefile.in

$(TARGET): SafmqTest.cpp ../lib/$(DEST)/libsafmqclient.a
	g++ $(CFLAGS) SafmqTest.cpp $(LDFLAGS) -o SafmqTest

mptest: mptest.cpp
	g++ $(CFLAGS) mptest.cpp $(LDFLAGS) -o mptest
