#include "BrushUI/ListContainerBase.h"
#include <QString>

ListContainerBase::ListContainerBase( bool needInitial )
	:QTreeWidgetItem()
{
	if ( needInitial )
	{
		m_plusButton = new QPushButton();
		m_plusButton->setIcon( QIcon( ":/MainWindow/Icon/plus.png" ) );
		m_plusButton->setMaximumSize( QSize( 30, 30 ) );
		m_plusButton->setMinimumSize( QSize( 30, 30 ) );

		QObject::connect( m_plusButton, &QPushButton::clicked, [=]()
			{
				QTreeWidgetItem* childItem = new QTreeWidgetItem();
				QPushButton* removeBtn = new QPushButton;
				removeBtn->setIcon( QIcon( ":/MainWindow/Icon/minus.png" ) );
				removeBtn->setMaximumSize( QSize( 30, 30 ) );
				removeBtn->setMinimumSize( QSize( 30, 30 ) );

				childItem->setText( 0, QString::number( m_childCount++ ) );
				this->setExpanded( true );
				this->addChild( childItem );
				this->treeWidget()->setItemWidget( childItem, 0, removeBtn );
				this->treeWidget()->setItemWidget( childItem, 1, createChildWidget() );

				QObject::connect( removeBtn, &QPushButton::clicked, [=]()
					{
						int index = indexOfChild( childItem );
						this->removeChild( childItem );
						deleteChild( index );
					} );
			} );
	}
}

ListContainerBase::~ListContainerBase()
{
}

QWidget* ListContainerBase::createChildWidget()
{
	return new QPushButton();
}

void ListContainerBase::deleteChild( int index )
{
}
