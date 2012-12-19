#include "gxSubject.h"

gxSubject::~gxSubject()
{
    mEventsQueue.clear();
}

void gxSubject::Subscribe( gxEvent &aEvent, gxCallback aCallback )
{
    // We delegate the subscription to the event itself, as each event keeps
    // a its own callbacks references.
    aEvent.Subscribe( aCallback );
}

void gxSubject::Unsubscribe( gxEvent &aEvent, void *aObj )
{
    // We delegate the unsubscription to the event itself, as each event keeps
    // a its own callbacks references.
    aEvent.Unsubscribe( aObj );
}

void gxSubject::Fire( gxEvent &aEvent, gxCallback aCallback )
{    
    if ( mFiringMode == on )
    {
        // If a specific callback was requested then only fire that one,
        // otherwise Fire() will fire all callbacks.
        aCallback.empty() ? aEvent.Fire() : aEvent.Fire( aCallback );
    } else if ( mFiringMode == queue ) {
        QueueEvent( aEvent );
    } // Otherwise FiringMode is off
}

void gxSubject::SuspendEvents( bool aQueueSuspended )
{
    mFiringMode = aQueueSuspended ? queue : off;
}

void gxSubject::ResumeEvents()
{
    gxEventsQueue::iterator iEvent;
    
    // For each event
    for( iEvent = mEventsQueue.begin(); iEvent != mEventsQueue.end(); ++iEvent )
    {
        (**iEvent).Fire();
    }
    
    mFiringMode = on;
}

bool gxSubject::EventAlreadyInQueue( gxEvent &aEvent )
{
    gxEventsQueue::iterator iEvent;
    
    // For each event
    for( iEvent = mEventsQueue.begin(); iEvent != mEventsQueue.end(); ++iEvent )
    {
        // Check if the event is already in the queue
        if ( (*iEvent) == &aEvent )
        {
            return true;
        }
    }
    return false;
}

void gxSubject::QueueEvent( gxEvent &aEvent )
{
    if ( !EventAlreadyInQueue( aEvent ) )
        mEventsQueue.push_back( &aEvent );
}