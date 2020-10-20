#pragma once

#include <QTreeWidgetItem>
#include <QPushButton>
#include <QWidget>

class TreeWidgetListContainer : public QTreeWidgetItem
{
public:
	TreeWidgetListContainer( bool needInitial = true );
	~TreeWidgetListContainer();

	virtual QWidget* createChildWidget();
 	virtual void deleteChild( int index );
	virtual void attachedTreeWidget() {}
	virtual QWidget* getTopItemWidget() { return m_plusButton; }

protected:
	QPushButton* m_plusButton;
	int m_childCount = 0;
};