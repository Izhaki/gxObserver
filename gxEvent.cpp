#include "gxEvent.h"


gxEvent::~gxEvent()
{
    mCallbacks.clear();
}

void gxEvent::Subscribe( gxCallback &aCallback )
{
    // Only subscribe to a callback if it isn't already subscribed.
    if ( !AlreadySubscribed( aCallback ) )
        mCallbacks.push_back( aCallback );
}

void gxEvent::Unsubscribe( void *aObj )
{
    gxCallbacks::iterator iCallback;
    
    // For each  callback
    for( iCallback = mCallbacks.begin(); iCallback != mCallbacks.end(); ++iCallback )
    {
        // If it's the same object
        if ( aObj == (*iCallback).GetObj() )
        {
            // Remove the callback from the list
            mCallbacks.erase( iCallback );
            break;
        }
    }
}

void gxEvent::Fire()
{
    gxCallbacks::iterator iCallback;
    
    // For each callback
    for( iCallback = mCallbacks.begin(); iCallback != mCallbacks.end(); ++iCallback )
    {
        Fire( *iCallback );
    }
}

bool gxEvent::AlreadySubscribed( gxCallback &aCallback )
{
    gxCallbacks::iterator iCallback;
    
    // For each callback
    for( iCallback = mCallbacks.begin(); iCallback != mCallbacks.end(); ++iCallback )
    {
        // We do not allow the same object to have two handlers for the same
        // event. So instead of comparing the Callbacks we compare their
        // objects.
        if ( aCallback.GetObj() == (*iCallback).GetObj() )
            return true;
    }
    
    return false;
}
