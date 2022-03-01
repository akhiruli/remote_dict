
#ifndef _EVENT_H_
#define _EVENT_H_

#include <event2/event.h>
#include <event2/event_struct.h>

typedef void(* event_callback_fn) (int, short, void *);
typedef void(* signal_callback_fn) (short, void *);

class EventImpl {
    typedef struct _context{
        signal_callback_fn cbk;
        void               *cookie;
        struct event       *event;
        _context(){
            cbk = nullptr;
            cookie = nullptr;
            event = nullptr;
        }
        ~_context(){
            if(event){
                event_free(event);
            }
        }
    }Context;
   public:
      EventImpl();
      EventImpl(const EventImpl& ) = delete;
      virtual ~EventImpl();
      void attachFdtoEv(struct event *, int, event_callback_fn, void *);
      void dispatchEvloop();
      bool initEv();
      void addToEv(struct event *ev);
      void addToEv(struct event *ev, struct timeval *t_intvl);
      void removeFromEv(struct event *ev);
      void exitLoop();
      struct event* getNewPersistentEvent(event_callback_fn cb, void *cookie);
      bool addSignal(int signum, signal_callback_fn cbk, void *cookie);
      static void signalCbk(evutil_socket_t sig, short events, void *user_data);
      static void eventCbk(int, short, void *);
      void addEvent(int fd, event_callback_fn cb, void *cookie);
   private:
      struct event_base       *m_pevbase;
};
#endif
