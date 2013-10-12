
VERSION=0.5
OUT=com.safmq.$(VERSION).jar \
	com.safmq.manager.$(VERSION).jar \
	com.safmq.src.$(VERSION).jar

all: $(OUT)

com.safmq.$(VERSION).jar:
	(cd Safmq; jar cvf ../$@ com/safmq/*.class)

com.safmq.manager.$(VERSION).jar:
	(cd Safmq; jar cvf ../$@ com/safmq/manager/*.class com/safmq/manager/images/*.gif)

com.safmq.src.$(VERSION).jar:
	(cd Safmq; jar cvf ../$@ com/safmq/*.java com/safmq/manager/*.java com/safmq/manager/images/*.gif)

