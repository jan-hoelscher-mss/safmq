#pragma warning(disable: 4786)

#include <time.h>
#include <sys/time.h>

#include <iostream>
#include <string>
#include <exception>

#include "safmq.h"
#include "lib/MQFactory.h"
#include "lib/MessageQueue.h"
#include "lib/MQConnection.h"
#include "../shared/thdlib.cpp"

#if _WIN32
	#define STRICMP _stricmp
	#include "_getopt.h"
#elif unix || (__APPLE__ && __MACH__) || __FreeBSD__

	#define STRICMP strcasecmp
	#include <termios.h>
	#include <unistd.h>
	#include <pwd.h>

	#if !defined(SAFMQ_GETOPT)
		#include <getopt.h>
	#else
		#include "utilities/_getopt_long.c"
	#endif
#else
	#error Unknown OS	
#endif

using namespace safmq;

const char* EC_Decode(ErrorCode ec) {
	static char buffer[1024];
#define ECOUT(v) case v: return #v;
	switch(ec) {
		ECOUT(EC_NOERROR)
		ECOUT(EC_TIMEDOUT)
		ECOUT(EC_NOTAUTHORIZED)
		ECOUT(EC_ALREADYCLOSED)
		ECOUT(EC_DOESNOTEXIST)
		ECOUT(EC_NOTOPEN)
		ECOUT(EC_NETWORKERROR)
		ECOUT(EC_TTLEXPIRED)
		ECOUT(EC_CURSORINVALIDATED)
		ECOUT(EC_DUPLICATEMSGID)
		ECOUT(EC_SERVERUNAVAIL)
		ECOUT(EC_NOMOREMESSAGES)
		ECOUT(EC_FILESYSTEMERROR)
		ECOUT(EC_CANNOTCHANGEGROUP)
		ECOUT(EC_CANNOTCHANGEUSER)
		ECOUT(EC_ALREADYDEFINED)
		ECOUT(EC_NOTIMPLEMENTED)
		ECOUT(EC_QUEUEOPEN)
		ECOUT(EC_NOTLOGGEDIN)
		ECOUT(EC_INVALIDNAME)
		ECOUT(EC_FORWARDNOTALLOWED)
		ECOUT(EC_WRONGMESSAGETYPE)
		ECOUT(EC_UNSUPPORTED_PROTOCOL)
		ECOUT(EC_ALREADYOPEN)
		ECOUT(EC_ERROR)
		default: 
			sprintf(buffer, "UNKNOWN:%ld", (int)ec);
			return buffer;
	}
#undef ECOUT
}

class xmiter : public Thread {
public:
	xmiter() : count(0) {}
	double count;
	string QUEUE_NAME;
};

class Receiver : public xmiter {
public:
	Receiver(const string& qn) { QUEUE_NAME = qn; }

	void* run() {
		while (true) {
			count = 0;
			try {
				MessageQueue* q = MQFactory::BuildQueueConnection(QUEUE_NAME,"","");
				ErrorCode	ec;
				while (true) { {
					QueueMessage	msg;
					if ((ec=q->Retrieve(true, -1, msg)) == EC_NOERROR)
						count++;
					else {
						cout << "Retrieve: " << EC_Decode(ec) << endl;
						break;
					}
				} }
			} catch (ErrorCode e) {
				cout << "Receiver: " << EC_Decode(e) << endl;
			} catch (MQFactoryException e) {
				cout << "Receiver: " << e.what() << endl;
			} catch (tcpsocket::SocketException e) {
				cout << "Receiver: " << e.what() << endl;
			}
		}
		return NULL;
	}
};


class Sender : public xmiter {
public:
	Sender(const string& qn) { QUEUE_NAME=qn; }

	void* run() {
		while (true) {
			count = 0;
			try {
				MessageQueue* q = MQFactory::BuildQueueConnection(QUEUE_NAME, "","");
				while (true) { {
					QueueMessage	msg;
					ErrorCode		ec;
					msg.setLabel("Hello World");
					*msg.getBufferStream() << "Hello World";
		
					if ((ec=q->Enqueue(msg)) == EC_NOERROR) {
						//cout << "." << flush;
						count++;
					} else {
						cout << "Send: " << EC_Decode(ec) << endl;
						break;
					}
				} }
			} catch (ErrorCode e) {
				cout << "Sender: " << EC_Decode(e) << endl;
			} catch (MQFactoryException e) {
				cout << "Sender: " << e.what() << endl;
			} catch (tcpsocket::SocketException e) {
				cout << "Sender: " << e.what() << endl;
			}
		}
		return NULL;
	}
};

enum OPID {
	OPID_RECEIVE=10000,
	OPID_RATE,
	OPID_HELP,
};

struct option	options[] = {
	{"receive",0,0,OPID_RECEIVE},
	{"rate",1,0, OPID_RATE},
	{"help",0,0,OPID_HELP},
};

void usage() {
	cout << "mptest: Safmq Multithread throughput test" << endl;
	cout << endl;
	cout << "Usage:" << endl;
	cout << "\tmptest [--receive|-r] [--rate=N] [--help|-h] <num threads> <URL>" << endl;
	cout << endl;
}

int main(int argc, char* argv[])
{
	int			count = 0;
	int			x;
	int			samplerate = 3; // default 3 second sample rate
	bool		receive = false;

	int theopt;		
	while ( (theopt=getopt_long(argc,argv,"hr",options,NULL)) != -1) {
		switch (theopt) {
			case '?': {
				cerr << "Unknown option: " << argv[optind-1] << endl << endl;
				usage();
				return -1;
			}

			case OPID_RATE:
				samplerate = atoi(optarg);
				break;
			
			case 'h':
			case OPID_HELP:
				usage();
				return 0;

			case 'r':
			case OPID_RECEIVE:
				receive = true;
				break;
		}
	}

	cout << "argc-optind:" << (argc-optind) << endl;
	if (argc-optind < 2) {
		usage();
		return -1;
	}

	count = atoi(argv[optind]);
	string queueName = argv[optind+1];

	xmiter		*xmitters[count];

	if (receive) {
		cout << "Starting " << count << " receivers" << endl;
		for (x=0; x< count; x++)
			(xmitters[x] = new Receiver(queueName))->start();
	} else {
		cout << "Starting " << count << " senders" << endl;
		for (x=0; x< count; x++)
			(xmitters[x] = new Sender(queueName))->start();
	}

	struct timeval	st, now;
	double			sv, cursamp;

	sv = 0;
	for(int x=0; x<count;x++)
		sv += xmitters[x]->count;
	gettimeofday(&st, NULL);

	while (true) {
		struct timespec ts;
		ts.tv_sec = samplerate;
		ts.tv_nsec = 0;
		nanosleep(&ts, NULL);
		

		cursamp = 0;
		for(int x=0; x<count;x++)
			cursamp += xmitters[x]->count;
		gettimeofday(&now, NULL);

		double inc = cursamp - sv;
		double span = now.tv_sec - st.tv_sec;
		span += (now.tv_usec - st.tv_usec)/1000000.0;

		printf("C S R: %0.0lf %0.03lf %0.03lf/sec\n", inc, span, (inc/span));
		st = now;
		sv = cursamp;
	}
	return 0;
}

