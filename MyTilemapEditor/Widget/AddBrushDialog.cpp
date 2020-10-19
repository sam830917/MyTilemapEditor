#include "Widget/AddBrushDialog.h"
#include <QSpacerItem>

AddBrushDialog::AddBrushDialog( QWidget* parent /*= Q_NULLPTR */ )
{
	m_ui.setupUi( this );
	m_ui.m_treeWidget->setColumnCount( 2 );
	m_ui.m_treeWidget->setHeaderHidden( true );
	m_ui.m_treeWidget->setSelectionMode( QAbstractItemView::NoSelection );
}

void AddBrushDialog::addItem( QList<AddBrushItem*> items )
{
	for ( AddBrushItem* item : items )
	{
		QTreeWidgetItem* treeWidget = new QTreeWidgetItem(m_ui.m_treeWidget);
		treeWidget->setText( 0, item->m_name );
		m_ui.m_treeWidget->setItemWidget( treeWidget, 1, item->m_widgetItem );
	}
}
