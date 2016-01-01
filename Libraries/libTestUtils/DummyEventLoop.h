#ifndef __STOCK_FIGHTER_LIBS_TESTING_MOCK_EVENT_LOOP_H__
#define __STOCK_FIGHTER_LIBS_TESTING_MOCK_EVENT_LOOP_H__

#include <IPostable.h>
#include <list>

class DummyEventLoop: public IPostable {
public:
    /**
     * Run the next task, if there is one.
     *
     * @returns true if there was a task to run, false otherwise
     */
    virtual bool RunNext();

    virtual void PostTask(const Task& t);
private:
    std::list<Task> tasks;
};


#endif
