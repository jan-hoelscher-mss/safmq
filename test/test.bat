@echo off

echo "Creating Environment"
md queues
xcopy /y ..\..\openssl-0.9.8e\out32dll\*.dll .


echo "Starting SAFMQ"
"..\Debug SSL\safmq" -c safmq-test.conf

echo "Cleaning Up"

rd /s /q queues

