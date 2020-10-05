#include "PropertiesWidget.h"
#include <QBoxLayout>

PropertiesWidget::PropertiesWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_treeWidget = new QTreeWidget();
	m_treeWidget->setColumnCount( 2 );
	QStringList strList;
	strList << "Name" << "Value";
	m_treeWidget->setHeaderLabels(strList);

	QWidget* placeholder = new QWidget( this );
	QBoxLayout* layout = new QBoxLayout( QBoxLayout::TopToBottom, placeholder );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( m_treeWidget );
	setWidget( placeholder );
}

void PropertiesWidget::showProperties( const QMap<QString, QString>& informationMap )
{
	if ( informationMap.isEmpty() )
		return;

	m_treeWidget->clear();
	QMap<QString, QString>::const_iterator mapIterator = informationMap.constBegin();
	while( mapIterator != informationMap.constEnd() )
	{
		QString name = mapIterator.key();
		QString value = mapIterator.value();
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText( 0, name );
		item->setText( 1, value );
		m_treeWidget->addTopLevelItem( item );

		++mapIterator;
	}
}
