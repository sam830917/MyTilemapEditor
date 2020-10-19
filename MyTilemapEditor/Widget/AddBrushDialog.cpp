#include "Widget/AddBrushDialog.h"
#include <QSpacerItem>

AddBrushDialog::AddBrushDialog( QWidget* parent /*= Q_NULLPTR */ )
{
	m_ui.setupUi( this );
}

void AddBrushDialog::setAddLayout( QBoxLayout* widget )
{
	QSpacerItem* spacerItem = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
	widget->addItem( spacerItem );
	m_ui.m_addWidget->setLayout( widget );
}
