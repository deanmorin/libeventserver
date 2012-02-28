#ifndef DM_EVENTBASE_HPP
#define DM_EVENTBASE_HPP
#include <event2/event.h>
namespace dm {

/**
 * Wrapper class for setting up an event base.
 * @author Dean Morin
 */
class EventBase
{
private:
    struct event_base* base_;

public:
    /**
     * @param method The event method to use.
     * @throws BadBaseException method is not available on this system.
     * @throws exception pthreads are not available on this system. 
     * @author Dean Morin
     */
    EventBase(const char* method);
    ~EventBase();

    /**
     * @return The name of the event handling method being used.
     * @author Dean Morin
     */
    const char* getMethod();
    /**
     * @return The event base structure.
     * @author Dean Morin
     */
    struct event_base* getBase();
    /**
     * @return All available event handling methods for this system.
     * @author Dean Morin
     */
    static const char** getAvailableMethods();
};

} // namespace dm
#endif
