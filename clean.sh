#!/bin/bash

find . -name Release\* \
		-or -name Debug\*\
		-or -name \*.opt\
		-or -name \*.ncb\
		-or -name \*.suo | ( read name; while [ $? = 0 ]; do echo $name; rm -rf "$name"; read name; done )

