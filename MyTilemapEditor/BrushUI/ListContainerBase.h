#pragma once

#include <QTreeWidgetItem>
#include <QPushButton>
#include <QWidget>

class ListContainerBase : public QTreeWidgetItem
{
public:
	ListContainerBase( bool needInitial = true );
	virtual ~ListContainerBase();

	virtual QWidget* createChildWidget();
 	virtual void deleteChild( int index );
	virtual void attachedTreeWidget() {}
	virtual QWidget* getTopItemWidget() { return m_plusButton; }

protected:
	QPushButton* m_plusButton = Q_NULLPTR;
	int m_childCount = 0;
};