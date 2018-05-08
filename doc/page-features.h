/**

\page Features

\section Readiness

\subsection sub1 QFace Syntax
In the QFace IDL file properties of an interface can be annotated with
\"<tt>\@readiness: true</tt>\", like this:
\code
module facelift.example.ready 1.0;

interface IReady {
    @readiness: true
    int someProperty;
}
\endcode
This will basically add a flag to the property, that tells whether the property's value has already
been set and is valid.

\subsection sub2 C++ Implementation

By default the \c readiness value will be true. It is however possible to set it to false
explicitly in your C++ implementation of the interface (derived from IReadyPropertyAdapter in the
example above). On the property you can call \c setReadiness(false) and set some reasonable
default value before. Note that whenever the poperty is assigned or bound its readiness will
automatically switch to true. It can be set explicetely to true, as well, of course
(\c setReadiness(true)). The following code snippet, if added to the constructor simulates a
property that becomes available after 1s:
\code
    m_someProperty = 0;                    // default value, as we don't know better yet
    m_someProperty.setReadiness(false);    // explicitely flag that the value is not properly set yet
    QTimer::singleShot(1000, this, [this] { m_someProperty = 42; });    // assignment will set readiness to true automatically
\endcode

\subsection sub3 QML Implementation
It is not possible to implement this in QML (derived from IReadyQmlImplementation in the example
above).

\subsection sub4 UI Usage

The API that is exposed to your QML UI (derived from IReadyQmlFrontend) will include an additional
\c readiness property with elements of type bool that have the same name as the annotated property
itself. Each element indicates, whether the property has been set already and is valid. The
\c readiness property and its elements are ready-only, of course. The following code snippet shows
how this could be used:
\code
IReadyFrontend {    // or whatever name the frontend is registered with
    onReadinessChanged: console.log("Readiness of someProperty is " + readiness.someProperty);
    onReadyPropChanged: console.log("Value of someProperty changed to " + someProperty);
}
\endcode
Just to be clear: if you have several properties annotated with the readiness flag in your
interface, you will not get indivdual signals for the readiness of each of the properties, but get
a \c readinessChanged signal whenever the readiness of a single property changes.

*/
