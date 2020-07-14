#include "FutureJob.hpp"

FutureJob::FutureJob(){};

FutureJob::FutureJob(String &relayId, Job job)
{
    _relayId = relayId;
    _job = job;
}

int FutureJob::getRelayPin() { return _job._relayPin; };
int FutureJob::getStatusPin() { return _job._statusPin; };
int FutureJob::getRelayState() { return _job._relayState; };
bool FutureJob::isSetState() { return _job._setState; };

String FutureJob::getRelayId() { return _relayId; };

