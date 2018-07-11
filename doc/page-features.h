/**

\page Features


\section async Asynchronous methods

In addition to standard methods, which block the caller until the method execution is finished and
provide an immediate return value, Facelift supports asynchronous methods. Asynchronous methods have
been introduced to support operations which potentially last a long time, or which are not able to
produce a return value immediately.

\subsection async-sub1 Declaration

Methods can be marked as asynchronous using the "async" decorator, as in the following QFace definition:

\code
module facelift.example 1.0;

interface MyInterface {

    @async : true
    int myAsyncMethod(int inputArgument);

}
\endcode

\subsection async-sub2 Usage from QML UI code
In order for the method's return value to be provided asynchronously, the method needs to be called
with a JavaScript function object as argument, in addition to the "normal" input parameters. The
JavaScript function should accept an argument, unless the method is defined as returning "void".
\code
    myInterface.myAsyncMethod(integerInputArgument, function(integerResult) {
        print("myAsyncMethod finished execution with result " + integerResult);
    });
\endcode

\subsection async-sub3 C++ method implementation
In order for a C++ implementation to provide its return value asynchronously, an asynchronous method
takes an additional argument. That argument defines an "operator()" method, which means it can be called
as if it was a std::function. Although it can be called synchronously, that object would typically have
to be copied so that it can be called after the method returns. Here is an example of C++ implementation
of an asynchronous method, which involves a copy of the "answer" object, since it is captured by a
lambda function:
\code
    void myAsyncMethod(int inputArgument, facelift::AsyncAnswer<int> answer) override {
        // We simply provide our return value after 1000 ms
        QTimer::singleShot(1000, [this, answer]() mutable {
            int returnValue = 10;
            answer(returnValue);
        });
    }
\endcode

\section Property Ready Flag

\subsection sub1 QFace Syntax
In the QFace IDL file properties of an interface can be annotated with
\"<tt>\@hasReadyFlag: true</tt>\", like this:
\code
module facelift.example.ready 1.0;

interface IReady {
    @hasReadyFlag: true
    int someProperty;
}
\endcode
This will basically add a flag to the property, that tells whether the property's value has already
been set and is valid.

\subsection sub2 C++ Implementation

By default the ready flag will be true. It is however possible to set it to false explicitly in
your C++ implementation of the interface (derived from IReadyPropertyAdapter in the example
above). On the property you can call \c setReady(false) and set some reasonable default value
before. Note that whenever the poperty is assigned or bound its ready flag will automatically
switch to true. It can be set explicetely to true, as well, of course (\c setReady(true)). The
following code snippet, if added to the constructor simulates a property that becomes available
after 1s:
\code
    m_someProperty = 0;                // default value, as we don't know better yet
    m_someProperty.setReady(false);    // explicitely flag that the value is not properly set yet
    QTimer::singleShot(1000, this, [this] { m_someProperty = 42; });    // assignment will set ready flag to true automatically
\endcode

\subsection sub3 QML Implementation
It is not possible to implement this in QML (derived from IReadyQmlImplementation in the example
above).

\subsection sub4 UI Usage

The API that is exposed to your QML UI (derived from IReadyQmlFrontend) will include an additional
\c readyFlags property with elements of type bool that have the same name as the annotated property
itself. Each element indicates, whether the property has been set already and is valid. The
\c readyFlags property and its elements are read-only, of course. The following code snippet shows
how this could be used:
\code
IReadyFrontend {    // or whatever name the frontend is registered with
    onReadyFlagsChanged: console.log("ReadyFlags of someProperty is " + readyFlags.someProperty);
    onSomePropertyChanged: console.log("Value of someProperty changed to " + someProperty);
}
\endcode
Just to be clear: if you have several properties annotated with hasReadyFlag in your interface,
you will not get indivdual signals for the ready flag of each of the properties, but get a
\c readyFlagsChanged signal whenever the ready flag of a single property changes.

*/
