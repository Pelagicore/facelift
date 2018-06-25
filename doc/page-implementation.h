/**

\page "Implementation and Usage"

The ordinary implementation an usage of a Facelift interface is described in the Example.
Here we focus on peculiarities of some selected types.

\section Models

QFace models need to be implemented differently compared to basic properties.

In the following we'll consider this QFace definition:
\code
module facelift.imodel 1.0;

interface IModel {
    readonly model<string> theModel;
}
\endcode

We use a model of string types here only to be concise, in real world applications the type is
usually a custom struct.

\note Only "readonly" models are supported by Facelift currently.


\subsection impl-sub1 Implementation

Models can only be implemented in C++, they cannot be implemented in QML. As usual, the
implementation should derive from IModelPropertyAdapter. Here is the outline of a sample
implementation:

\code
class IModelImplementation : public IModelPropertyAdapter
{
    Q_OBJECT
public:
    IModelImplementation(QObject *parent = nullptr) : IModelPropertyAdapter(parent)
    {
        m_theModel.setSize(m_items.size());
        m_theModel.setGetter(std::bind(&IModelImplementation::getItem, this, std::placeholders::_1));
    }

    QString getItem(int index)
    {
        return m_items.at(index);
    }

private:
    QStrigList m_items;
};
\endcode

The property adapter will include a public member variable that can be used to provide the model
data. In our case IModelPropertyAdapter includes \c m_theModel. Two methods of this member variable
should be called (usually in the constructor): \c setSize and \c setGetter. \c setSize needs to be
called with the model size (as int) and \c setGetter with a function that takes an index parameter
of type int and returns the corresponding model item. In the example above the getter is simply
called \c getItem.

\subsubsection impl-sub1-sub1 Item Changes
If model items change, this should be notified by emitting the \c dataChanged signal. In our case:
m_theModel.dataChanged(itemIndex) for a single item or m_theModel.dataChanged(first, last), if
several items changed.

\subsubsection impl-sub1-sub2 Item Insertion
If items are inserted into the model, this should be notified by emitting \c beginInsertElements,
followed by endInsertElements. In our case: m_theModel.beginInsertElements(first, last) and
m_theModel.endInsertElements(). The arguments first and last are the indices that the new items
will have after they have been inserted. If only one item is inserted first and last will have the
same index. The actual model update should be done in-between those two method calls, this must
include a call to \c setSize with the new model size.

\subsubsection impl-sub1-sub3 Item Removal
If items are removed from the model, this should be notified by emitting \c beginREmoveElements,
followed by endRemoveElements. In our case: m_theModel.beginRemoveElements(first, last) and
m_theModel.endRemoveElements(). The actual model update should be done in between those two method
calls, this must include a call to \c setSize with the new model size.

*/
