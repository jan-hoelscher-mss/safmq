#!/bin/sh 

EXTERNAL=no

while [ "$1" != "" ]; do
	case "$1" in 
		--external)
			EXTERNAL=yes;
			;;
	esac
	shift
done

echo "Creating Enviornment"
rm -rf queues
mkdir queues

echo "Starting SAFMQ"
GPPVER=`g++ -dumpversion`
../bin.$GPPVER/safmq -c safmq-test.conf </dev/null > server.out 2>&1 &
PID=$!

echo "Waiting for server to start"
sleep 2

if [ $EXTERNAL = no ]; then
	echo "Running Tests"
	./SafmqTest localhost 40000 40001 2>&1 | tee tests.out
else
	echo "Press enter to end test"
	read line
fi

echo "Stopping Server"
kill -INT $PID

echo "Cleaning up"
rm -rf queues

echo See results in server.out and tests.out
