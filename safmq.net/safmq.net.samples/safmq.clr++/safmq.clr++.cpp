// safmq.clr++.cpp : main project file.

#include "stdafx.h"
#include <fstream>


using namespace System;
using namespace System::IO;
using namespace safmq::net;

using namespace std;



int main(array<System::String ^> ^args)
{
    QueueHandle^	q = gcnew QueueHandle;
	QueueMessage^	msg = gcnew QueueMessage;
	ErrorCode		err;
	MQConnection^	con = MQBuilder::BuildConnection(gcnew Uri("safmq://admin:@localhost"), "", "");

	remove("test.foo");

	err = con->OpenQueue("testQ", q);

	msg->Label = "C++ Test!";
	StreamWriter^ w = gcnew StreamWriter(msg->Stream);
	w->WriteLine("Hello from c++");
	w->Flush();

	err = con->Enqueue(q, msg);
	Console::WriteLine("Enqueue: " + err.ToString());

	err = con->CloseQueue(q);
	Console::WriteLine("CloseQueue: " + err.ToString());

	err = con->Close();
	Console::WriteLine("Close: " + err.ToString());

	fstream f("test.out", ios::out);
	f.close();


	return 0;
}
