#ifndef _SAFMQ_STATS_H_
#define _SAFMQ_STATS_H_

#include <time.h>
#include "Mutex.h"

#define STAT_COUNT_BUF_SIZE 512
#define STAT_COUNT_MASK		0x01FF

struct _1060300 {
	unsigned int _10;
	unsigned int _60;
	unsigned int _300;
};

class Stats {
private:
	time_t	lastSample;
	int curSample;

	unsigned int 	enqueued[STAT_COUNT_BUF_SIZE];
	unsigned int 	dequeued[STAT_COUNT_BUF_SIZE];
	unsigned int 	peeked[STAT_COUNT_BUF_SIZE];
protected:
	void rotateSamples();
public:
	Stats();
	virtual ~Stats();

	void messageEnqueued();
	void messageDequeued();
	void messagePeeked();

	_1060300 enqueueCounts();
	_1060300 dequeueCounts();
	_1060300 peekCounts();
};

class ServerStats {
private:
	Mutex mtx;
	Stats stats;
	static ServerStats* singleton;

protected:
	ServerStats() {}

public:
	static ServerStats* getServerStats();

	void messageEnqueued();
	void messageDequeued();
	void messagePeeked();

	_1060300 enqueueCounts();
	_1060300 dequeueCounts();
	_1060300 peekCounts();
};


#endif
