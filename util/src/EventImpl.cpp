#include <EventImpl.h>

EventImpl::EventImpl(): m_pevbase(nullptr) {
}

EventImpl::~EventImpl(){
   if(m_pevbase)
      event_base_free(m_pevbase);
}

bool EventImpl::initEv(){
   m_pevbase = event_base_new();
   if(m_pevbase)
      return true;
   else
      return false;
}

void EventImpl::dispatchEvloop(){
   event_base_dispatch(m_pevbase);
}

void EventImpl::attachFdtoEv(struct event *ev, int fd, event_callback_fn cb, void *cookie){
   event_assign(ev, m_pevbase, fd, EV_READ|EV_PERSIST, cb, cookie);
}

void EventImpl::addToEv(struct event *ev){
   event_add(ev, nullptr);
}

void EventImpl::addToEv(struct event *ev, struct timeval *t_intvl){
   event_add(ev, t_intvl);
}

void EventImpl:: removeFromEv(struct event *ev){
    event_del(ev);
}

void EventImpl::exitLoop(){
    struct timeval delay = { 2, 0 };
    event_base_loopexit(m_pevbase, &delay);
}

bool EventImpl::addSignal(int signum, signal_callback_fn cbk, void *cookie){
    Context *ctx = new Context();
    ctx->cbk = cbk;
    ctx->cookie = cookie;
    ctx->event = evsignal_new(m_pevbase, signum, signalCbk, (void *)ctx);
    if(ctx->event){
        event_add(ctx->event, NULL);
        return true;
    }

    return false;
}

void EventImpl::signalCbk(evutil_socket_t sig, short events, void *user_data){
    Context *ctx = reinterpret_cast<Context *>(user_data);
    if(ctx && ctx->cbk){
        ctx->cbk(events, ctx->cookie);
        delete ctx;
    }
}
