#include "DummyEventLoop.h"

bool DummyEventLoop::RunNext() {
    bool ran = false;
    if ( tasks.size() > 0 ) {
        tasks.back()();
        tasks.pop_back();
        ran = true;
    }

    return ran;
}

void DummyEventLoop::PostTask(const Task& t) {
    tasks.push_front(t);
}
