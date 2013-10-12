#include "Stats.h"
#include <memory.h>
#include <string.h>
#include "Log.h"
#include <sstream>

Stats::Stats()
{
	// setting last sample to zero ensures the circular buffer
	// will be cleared
	lastSample = 0;
	curSample = 0;
}

Stats::~Stats()
{
}

void Stats::rotateSamples()
{
	time_t now = time(NULL);
	long timeDiff = (long)(now - lastSample);
	if (timeDiff > STAT_COUNT_BUF_SIZE)
		timeDiff = STAT_COUNT_BUF_SIZE;

	if (timeDiff > 0) {
		// Initialize to zero

		if ( timeDiff == 1) {
			// Skip the loop building
			enqueued[(curSample + 1) & STAT_COUNT_MASK] = 0;
			dequeued[(curSample + 1) & STAT_COUNT_MASK] = 0;
			peeked[(curSample + 1) & STAT_COUNT_MASK] = 0;
		} else {
			for(long x = 1; x <= timeDiff; ++x) {
				enqueued[(curSample + x) & STAT_COUNT_MASK] = 0;
				dequeued[(curSample + x) & STAT_COUNT_MASK] = 0;
				peeked[(curSample + x) & STAT_COUNT_MASK] = 0;
			}
		}

		curSample = (curSample + timeDiff) & STAT_COUNT_MASK;
		lastSample = now;
	}
}


void Stats::messageEnqueued()
{
	rotateSamples();
	++enqueued[curSample];
}
void Stats::messageDequeued()
{
	rotateSamples();
	++dequeued[curSample];
}

void Stats::messagePeeked()
{
	rotateSamples();
	++peeked[curSample];
}

_1060300 Stats::enqueueCounts()
{
	_1060300 result;

	rotateSamples();
	result._10 = enqueued[curSample] 
				+ enqueued[(curSample-1) & STAT_COUNT_MASK] + enqueued[(curSample-2) & STAT_COUNT_MASK] + enqueued[(curSample-3) & STAT_COUNT_MASK]
				+ enqueued[(curSample-4) & STAT_COUNT_MASK] + enqueued[(curSample-5) & STAT_COUNT_MASK] + enqueued[(curSample-6) & STAT_COUNT_MASK]
				+ enqueued[(curSample-7) & STAT_COUNT_MASK] + enqueued[(curSample-8) & STAT_COUNT_MASK] + enqueued[(curSample-9) & STAT_COUNT_MASK];
	result._60 = result._10
				+ enqueued[(curSample-10) & STAT_COUNT_MASK]
				+ enqueued[(curSample-11) & STAT_COUNT_MASK] + enqueued[(curSample-12) & STAT_COUNT_MASK] + enqueued[(curSample-13) & STAT_COUNT_MASK]
				+ enqueued[(curSample-14) & STAT_COUNT_MASK] + enqueued[(curSample-15) & STAT_COUNT_MASK] + enqueued[(curSample-16) & STAT_COUNT_MASK]
				+ enqueued[(curSample-17) & STAT_COUNT_MASK] + enqueued[(curSample-18) & STAT_COUNT_MASK] + enqueued[(curSample-19) & STAT_COUNT_MASK]
				+ enqueued[(curSample-20) & STAT_COUNT_MASK]
				+ enqueued[(curSample-21) & STAT_COUNT_MASK] + enqueued[(curSample-22) & STAT_COUNT_MASK] + enqueued[(curSample-23) & STAT_COUNT_MASK]
				+ enqueued[(curSample-24) & STAT_COUNT_MASK] + enqueued[(curSample-25) & STAT_COUNT_MASK] + enqueued[(curSample-26) & STAT_COUNT_MASK]
				+ enqueued[(curSample-27) & STAT_COUNT_MASK] + enqueued[(curSample-28) & STAT_COUNT_MASK] + enqueued[(curSample-29) & STAT_COUNT_MASK]
				+ enqueued[(curSample-30) & STAT_COUNT_MASK]
				+ enqueued[(curSample-31) & STAT_COUNT_MASK] + enqueued[(curSample-32) & STAT_COUNT_MASK] + enqueued[(curSample-33) & STAT_COUNT_MASK]
				+ enqueued[(curSample-34) & STAT_COUNT_MASK] + enqueued[(curSample-35) & STAT_COUNT_MASK] + enqueued[(curSample-36) & STAT_COUNT_MASK]
				+ enqueued[(curSample-37) & STAT_COUNT_MASK] + enqueued[(curSample-38) & STAT_COUNT_MASK] + enqueued[(curSample-39) & STAT_COUNT_MASK]
				+ enqueued[(curSample-40) & STAT_COUNT_MASK]
				+ enqueued[(curSample-41) & STAT_COUNT_MASK] + enqueued[(curSample-42) & STAT_COUNT_MASK] + enqueued[(curSample-43) & STAT_COUNT_MASK]
				+ enqueued[(curSample-44) & STAT_COUNT_MASK] + enqueued[(curSample-45) & STAT_COUNT_MASK] + enqueued[(curSample-46) & STAT_COUNT_MASK]
				+ enqueued[(curSample-47) & STAT_COUNT_MASK] + enqueued[(curSample-48) & STAT_COUNT_MASK] + enqueued[(curSample-49) & STAT_COUNT_MASK]
				+ enqueued[(curSample-50) & STAT_COUNT_MASK]
				+ enqueued[(curSample-51) & STAT_COUNT_MASK] + enqueued[(curSample-52) & STAT_COUNT_MASK] + enqueued[(curSample-53) & STAT_COUNT_MASK]
				+ enqueued[(curSample-54) & STAT_COUNT_MASK] + enqueued[(curSample-55) & STAT_COUNT_MASK] + enqueued[(curSample-56) & STAT_COUNT_MASK]
				+ enqueued[(curSample-57) & STAT_COUNT_MASK] + enqueued[(curSample-58) & STAT_COUNT_MASK] + enqueued[(curSample-59) & STAT_COUNT_MASK];

	result._300 = result._60;
	for(int x = 60; x < 300; ++x) {
		result._300 += enqueued[(curSample-x) & STAT_COUNT_MASK];
	}
	return result;
}

_1060300 Stats::dequeueCounts()
{
	_1060300 result;

	rotateSamples();
	result._10 = dequeued[curSample] 
				+ dequeued[(curSample-1) & STAT_COUNT_MASK] + dequeued[(curSample-2) & STAT_COUNT_MASK] + dequeued[(curSample-3) & STAT_COUNT_MASK]
				+ dequeued[(curSample-4) & STAT_COUNT_MASK] + dequeued[(curSample-5) & STAT_COUNT_MASK] + dequeued[(curSample-6) & STAT_COUNT_MASK]
				+ dequeued[(curSample-7) & STAT_COUNT_MASK] + dequeued[(curSample-8) & STAT_COUNT_MASK] + dequeued[(curSample-9) & STAT_COUNT_MASK];
	result._60 = result._10
				+ dequeued[(curSample-10) & STAT_COUNT_MASK]
				+ dequeued[(curSample-11) & STAT_COUNT_MASK] + dequeued[(curSample-12) & STAT_COUNT_MASK] + dequeued[(curSample-13) & STAT_COUNT_MASK]
				+ dequeued[(curSample-14) & STAT_COUNT_MASK] + dequeued[(curSample-15) & STAT_COUNT_MASK] + dequeued[(curSample-16) & STAT_COUNT_MASK]
				+ dequeued[(curSample-17) & STAT_COUNT_MASK] + dequeued[(curSample-18) & STAT_COUNT_MASK] + dequeued[(curSample-19) & STAT_COUNT_MASK]
				+ dequeued[(curSample-20) & STAT_COUNT_MASK]
				+ dequeued[(curSample-21) & STAT_COUNT_MASK] + dequeued[(curSample-22) & STAT_COUNT_MASK] + dequeued[(curSample-23) & STAT_COUNT_MASK]
				+ dequeued[(curSample-24) & STAT_COUNT_MASK] + dequeued[(curSample-25) & STAT_COUNT_MASK] + dequeued[(curSample-26) & STAT_COUNT_MASK]
				+ dequeued[(curSample-27) & STAT_COUNT_MASK] + dequeued[(curSample-28) & STAT_COUNT_MASK] + dequeued[(curSample-29) & STAT_COUNT_MASK]
				+ dequeued[(curSample-30) & STAT_COUNT_MASK]
				+ dequeued[(curSample-31) & STAT_COUNT_MASK] + dequeued[(curSample-32) & STAT_COUNT_MASK] + dequeued[(curSample-33) & STAT_COUNT_MASK]
				+ dequeued[(curSample-34) & STAT_COUNT_MASK] + dequeued[(curSample-35) & STAT_COUNT_MASK] + dequeued[(curSample-36) & STAT_COUNT_MASK]
				+ dequeued[(curSample-37) & STAT_COUNT_MASK] + dequeued[(curSample-38) & STAT_COUNT_MASK] + dequeued[(curSample-39) & STAT_COUNT_MASK]
				+ dequeued[(curSample-40) & STAT_COUNT_MASK]
				+ dequeued[(curSample-41) & STAT_COUNT_MASK] + dequeued[(curSample-42) & STAT_COUNT_MASK] + dequeued[(curSample-43) & STAT_COUNT_MASK]
				+ dequeued[(curSample-44) & STAT_COUNT_MASK] + dequeued[(curSample-45) & STAT_COUNT_MASK] + dequeued[(curSample-46) & STAT_COUNT_MASK]
				+ dequeued[(curSample-47) & STAT_COUNT_MASK] + dequeued[(curSample-48) & STAT_COUNT_MASK] + dequeued[(curSample-49) & STAT_COUNT_MASK]
				+ dequeued[(curSample-50) & STAT_COUNT_MASK]
				+ dequeued[(curSample-51) & STAT_COUNT_MASK] + dequeued[(curSample-52) & STAT_COUNT_MASK] + dequeued[(curSample-53) & STAT_COUNT_MASK]
				+ dequeued[(curSample-54) & STAT_COUNT_MASK] + dequeued[(curSample-55) & STAT_COUNT_MASK] + dequeued[(curSample-56) & STAT_COUNT_MASK]
				+ dequeued[(curSample-57) & STAT_COUNT_MASK] + dequeued[(curSample-58) & STAT_COUNT_MASK] + dequeued[(curSample-59) & STAT_COUNT_MASK];

	result._300 = result._60;
	for(int x = 60; x < 300; ++x) {
		result._300 += dequeued[(curSample-x) & STAT_COUNT_MASK];
	}

	std::stringstream counts;
	for(int x = 0; x < 300; ++x) {
		counts << dequeued[(curSample-x)&STAT_COUNT_MASK] << " ";
	}
	safmq::Log::getLog()->Info("curSample: %ld", curSample);
	safmq::Log::getLog()->Info("Dequeued: %s", counts.str().c_str());

	return result;
}

_1060300 Stats::peekCounts()
{
	_1060300 result;

	rotateSamples();
	result._10 = peeked[curSample] 
				+ peeked[(curSample-1) & STAT_COUNT_MASK] + peeked[(curSample-2) & STAT_COUNT_MASK] + peeked[(curSample-3) & STAT_COUNT_MASK]
				+ peeked[(curSample-4) & STAT_COUNT_MASK] + peeked[(curSample-5) & STAT_COUNT_MASK] + peeked[(curSample-6) & STAT_COUNT_MASK]
				+ peeked[(curSample-7) & STAT_COUNT_MASK] + peeked[(curSample-8) & STAT_COUNT_MASK] + peeked[(curSample-9) & STAT_COUNT_MASK];
	result._60 = result._10
				+ peeked[(curSample-10) & STAT_COUNT_MASK]
				+ peeked[(curSample-11) & STAT_COUNT_MASK] + peeked[(curSample-12) & STAT_COUNT_MASK] + peeked[(curSample-13) & STAT_COUNT_MASK]
				+ peeked[(curSample-14) & STAT_COUNT_MASK] + peeked[(curSample-15) & STAT_COUNT_MASK] + peeked[(curSample-16) & STAT_COUNT_MASK]
				+ peeked[(curSample-17) & STAT_COUNT_MASK] + peeked[(curSample-18) & STAT_COUNT_MASK] + peeked[(curSample-19) & STAT_COUNT_MASK]
				+ peeked[(curSample-20) & STAT_COUNT_MASK]
				+ peeked[(curSample-21) & STAT_COUNT_MASK] + peeked[(curSample-22) & STAT_COUNT_MASK] + peeked[(curSample-23) & STAT_COUNT_MASK]
				+ peeked[(curSample-24) & STAT_COUNT_MASK] + peeked[(curSample-25) & STAT_COUNT_MASK] + peeked[(curSample-26) & STAT_COUNT_MASK]
				+ peeked[(curSample-27) & STAT_COUNT_MASK] + peeked[(curSample-28) & STAT_COUNT_MASK] + peeked[(curSample-29) & STAT_COUNT_MASK]
				+ peeked[(curSample-30) & STAT_COUNT_MASK]
				+ peeked[(curSample-31) & STAT_COUNT_MASK] + peeked[(curSample-32) & STAT_COUNT_MASK] + peeked[(curSample-33) & STAT_COUNT_MASK]
				+ peeked[(curSample-34) & STAT_COUNT_MASK] + peeked[(curSample-35) & STAT_COUNT_MASK] + peeked[(curSample-36) & STAT_COUNT_MASK]
				+ peeked[(curSample-37) & STAT_COUNT_MASK] + peeked[(curSample-38) & STAT_COUNT_MASK] + peeked[(curSample-39) & STAT_COUNT_MASK]
				+ peeked[(curSample-40) & STAT_COUNT_MASK]
				+ peeked[(curSample-41) & STAT_COUNT_MASK] + peeked[(curSample-42) & STAT_COUNT_MASK] + peeked[(curSample-43) & STAT_COUNT_MASK]
				+ peeked[(curSample-44) & STAT_COUNT_MASK] + peeked[(curSample-45) & STAT_COUNT_MASK] + peeked[(curSample-46) & STAT_COUNT_MASK]
				+ peeked[(curSample-47) & STAT_COUNT_MASK] + peeked[(curSample-48) & STAT_COUNT_MASK] + peeked[(curSample-49) & STAT_COUNT_MASK]
				+ peeked[(curSample-50) & STAT_COUNT_MASK]
				+ peeked[(curSample-51) & STAT_COUNT_MASK] + peeked[(curSample-52) & STAT_COUNT_MASK] + peeked[(curSample-53) & STAT_COUNT_MASK]
				+ peeked[(curSample-54) & STAT_COUNT_MASK] + peeked[(curSample-55) & STAT_COUNT_MASK] + peeked[(curSample-56) & STAT_COUNT_MASK]
				+ peeked[(curSample-57) & STAT_COUNT_MASK] + peeked[(curSample-58) & STAT_COUNT_MASK] + peeked[(curSample-59) & STAT_COUNT_MASK];

	result._300 = result._60;
	for(int x = 60; x < 300; ++x) {
		result._300 += peeked[(curSample-x) & STAT_COUNT_MASK];
	}
	return result;
}


ServerStats* ServerStats::singleton = NULL;

ServerStats* ServerStats::getServerStats()
{
	static Mutex smtx;
	MutexLock lock(&smtx);
	if (singleton == NULL)
		singleton = new ServerStats;
	return singleton;
}

void ServerStats::messageEnqueued()
{
	MutexLock lock(&mtx);
	stats.messageEnqueued();
}

void ServerStats::messageDequeued()
{
	MutexLock lock(&mtx);
	stats.messageDequeued();
}

void ServerStats::messagePeeked()
{
	MutexLock lock(&mtx);
	stats.messagePeeked();
}

_1060300 ServerStats::enqueueCounts()
{
	MutexLock lock(&mtx);
	return stats.enqueueCounts();
}

_1060300 ServerStats::dequeueCounts()
{
	MutexLock lock(&mtx);
	return stats.dequeueCounts();
}

_1060300 ServerStats:: peekCounts()
{
	MutexLock lock(&mtx);
	return stats.peekCounts();
}
