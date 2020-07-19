#pragma once
#include <EsmartFirebase.hpp>

class FutureJob {
   public:
    EsmartFirebase esmart;

    FutureJob();
    FutureJob(EsmartFirebase &esmart);

    void clear() {
        esmart = nullptr;
    }
};
