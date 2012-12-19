#include <iostream>
#include "gxEvent.h"
#include "gxSubject.h"
#include "gxObserver.h"

using namespace std;

// -------------------------------------------------------------------------- //
// Simple demo classes
// -------------------------------------------------------------------------- //

class SimpleSubject : virtual public gxSubject
{
public:
    // Defining an event with a single int parameter
    gxDefineEvent( evAge, int )
    
    void FireEvents()
    {
        // Fire the event
        Fire( evAge, 69 );
    }
};



class SimpleObserver
{
    gxDeclareObserver( SimpleObserver )
public:
    SimpleObserver( SimpleSubject* aSubject )
    {
        mSubject = aSubject;
        
        // Subscribe to the subject events
        mSubject->gxSubscribe( evAge, OnAgeChanged );
    }
    
    virtual ~SimpleObserver()
    {
        // Unsubscribe from the subject events
        mSubject->gxUnsubscribe( evAge );
    }
    
    // The event handler callback
    void OnAgeChanged( int aAge )
    {
        cout << "SimpleObserver::OnAgeChanged with " << aAge << "\n";
    }
private:
    SimpleSubject* mSubject;
};

// -------------------------------------------------------------------------- //
// Complex demo classes
// -------------------------------------------------------------------------- //

struct Size
{
    int Width;
    int Height;
};

class ComplexSubject : virtual public gxSubject
{
public:
    ComplexSubject()
    {
        mSize.Width  = 100;
        mSize.Height = 10;
        mCity        = "London";
    }
    
    // An event with no parameters
    gxDefineEvent( evNoParameters )
    
    // Defining an event with a single int parameter
    gxDefineEvent( evAge, int )
    
    // An event with 4 parameters, including the sender as the first parameter.
    gxDefineEvent( evQuad, ComplexSubject*, int, int, bool )
    
    // An event with struct parameter.
    gxDefineEvent( evSize, Size* )

    // Defining a bound event - one that fires upon subscription. This bounds
    // the string value to a getter.
    gxDefineBoundEvent( evName, string, GetName() )
    
    // A bound event with sender, we bind to a member variable rather than a
    // getter.
    gxDefineBoundEvent( evCity, ComplexSubject*, string, mCity )
    
    void FireEvents()
    {
        // Fire the events
        Fire( evNoParameters );
        Fire( evAge, 76 );
        // If events are queued this is the only evAge to be fired; the previous
        // one will be ignored.
        Fire( evAge, 12 );
        Fire( evQuad, this, 11, 22, true );
        Fire( evSize, &mSize );
    }
    
    // Manually fire bound events.
    void FireBoundEvents()
    {
        // Bound events can be shot without parameters
        Fire( evName );
        // But also with
        Fire( evName, "Daisy!" );        
    }
    
    string GetName()
    {
        return "Crazy!";
    }
private:
    Size   mSize;
    string mCity;
};



class ComplexObserver
{
    gxDeclareObserver( ComplexObserver )
public:
    ComplexObserver( ComplexSubject* aSubject )
    : mSubject( aSubject )
    {
        // Subscribe to the subject events
        mSubject->gxSubscribe( evNoParameters, OnNoParametersEvent );
        mSubject->gxSubscribe( evAge, OnAgeChanged );
        mSubject->gxSubscribe( evQuad, OnQuad );
        
        mSubject->gxSubscribe( evSize, OnSize );
        mSubject->gxSubscribe( evSize, OnSize ); // This will be ignored as we've just subscribed to the same event.
        
        mSubject->gxSubscribe( evName, OnName );
        mSubject->gxSubscribe( evCity, OnCity );
    }
        
    virtual ~ComplexObserver()
    {
        // Unsubscribe from the subject events
        mSubject->gxUnsubscribe( evNoParameters );        
        mSubject->gxUnsubscribe( evAge );
        mSubject->gxUnsubscribe( evQuad );
        mSubject->gxUnsubscribe( evSize );
        mSubject->gxUnsubscribe( evName );
        mSubject->gxUnsubscribe( evCity );
    }
    
    // Event handlers
    
    void OnNoParametersEvent()
    {
        cout << "ComplexObserver::OnNoParametersEvent\n";
    }
    
    void OnAgeChanged( int aAge )
    {
        cout << "ComplexObserver::OnAgeChanged with " << aAge << "\n";
    }
    
    void OnQuad( ComplexSubject* aSubject, int X, int Y, bool aBool )
    {
        cout << "ComplexObserver::onQuad (" << X << ", " << Y << ", " << aBool << ")\n";
    }
    
    void OnSize( Size* aSize )
    {
        cout << "ComplexObserver::OnSize (" << aSize->Width << ", " << aSize->Height << ")\n";
    }

    void OnName( string aName )
    {
        cout << "ComplexObserver::OnName with: " << aName << "\n";
    }

    void OnCity( ComplexSubject* aSubject, string aCity )
    {
        cout << "ComplexObserver::OnCity with: " << aCity << "\n";
    }
    
protected:
    ComplexSubject* mSubject;
};


// -------------------------------------------------------------------------- //
// Base-derived demo classes
// -------------------------------------------------------------------------- //

class BaseObserver
{
    gxDeclareObserver( BaseObserver )
public:
    BaseObserver( ComplexSubject* aSubject )
      : mSubject( aSubject )
    {
        // We cannot subscribe to events here as when this base class is
        // constructed it only see its own handler and not the virtual one
        // of derived class.
        //
        // This means that any bound events fired due to a subscription here
        // we be routed to the base class handler, which is not what we want.
        //
        // So subscription happens in a dedicated method below, which will
        // be called by either the client code, or the derived class.
    }
    
    void DoSubscribe()
    {
        // Subscribe to the subject events
        mSubject->gxSubscribe( evName, OnName );
    }
    
    virtual ~BaseObserver()
    {
        // Unsubscribe from the subject events
        mSubject->gxUnsubscribe( evName );
    }
    
    // A virtual event handler !!!
    virtual void OnName( string aName )
    {
        cout << "BaseObserver::OnName with: " << aName << "\n";
    }
protected:
    ComplexSubject* mSubject;
};

class DerivedObserver : public BaseObserver
{
    gxDeclareObserver( DerivedObserver )
public:
    DerivedObserver( ComplexSubject* aSubject )
    : BaseObserver( aSubject )
    {
        // It is save to subscribe here as we're only within the scope of the
        // derived class, so the virtual handler will be picked alright.
        DoSubscribe();
    }
    
    // Event handlers
    virtual void OnName( string aName )
    {
        // We can call the base class handler if we want by uncommenting this
        // line:
        // BaseObserver::OnName( aName );
        
        cout << "DerivedObserver::OnName with: " << aName << "\n";

    }
};

int main(int argc, const char * argv[])
{
    cout << "Simple Demo:\n";
    // Simple Demo
    SimpleSubject *iSimpleSubject = new SimpleSubject();
    SimpleObserver *iSimpleObserver = new SimpleObserver( iSimpleSubject );
    
    iSimpleSubject->FireEvents();
    
    delete iSimpleObserver;
    delete iSimpleSubject;
    
    cout << "\nComplex Demo:\n";
    
    // Complex Demo
    ComplexSubject *iComplexSubject = new ComplexSubject();
    
    cout << "Bound events that fired upon subscribe:\n";
    
    // New complex observer
    ComplexObserver *iComplexObserver = new ComplexObserver( iComplexSubject );
    
    // New derived observer ( using BaseObserver variable to show polymorphism
    // works.
    BaseObserver *iObserver = new DerivedObserver( iComplexSubject );

    // Suspending events, but queuing them
    iComplexSubject->SuspendEvents( true );
    iComplexSubject->FireEvents();
    
    // Now resume event (will fire all queued events
    cout << "\nNormal Events (these where queued):\n";
    iComplexSubject->ResumeEvents();

    cout << "\nBound events fired manually:\n";
    // Now fire some bound events
    iComplexSubject->FireBoundEvents();
    
    delete iObserver;
    delete iComplexObserver;
    delete iComplexSubject;

    return 0;
}

