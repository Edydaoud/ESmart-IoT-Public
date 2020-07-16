#include "Arduino.h"
#pragma once

struct Job
{
    int _relayPin;
    int _statusPin;
    int _relayState;
    bool _setState;
    int _jobId;
};

class FutureJob : public Printable
{

    String _relayId;
    Job _job;

public:
    FutureJob();
    FutureJob(String &relayId, Job job);

    int getRelayPin();
    int getStatusPin();
    int getRelayState();
    int getJobId();

    String getRelayId();
    void doWork();
    bool isSetState();


public:
  size_t printTo(Print &p) const
  {
    size_t t = 0;
    t += p.print("_relayId: ");
    t += p.println(_relayId);
    t += p.print("_relayPin: ");
    t += p.println(_job._relayPin);
    t += p.print("_statusPin: ");
    t += p.println(_job._statusPin);
    t += p.print("_relayState: ");
    t += p.println(_job._relayState);
    t += p.print("_setState: ");
    t += p.println(_job._setState);
    t += p.print("_jobId: ");
    t += p.println(_job._jobId);
    return t;
  }
};
