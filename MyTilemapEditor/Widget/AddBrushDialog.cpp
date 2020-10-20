#include "Widget/AddBrushDialog.h"
#include "Brush/Brush.h"
#include "Utils/ProjectCommon.h"
#include <QSpacerItem>
#include <QMessageBox>
#include <QFileInfo>

AddBrushDialog::AddBrushDialog( QWidget* parent /*= Q_NULLPTR */ )
{
	m_ui.setupUi( this );
	m_ui.m_treeWidget->setColumnCount( 2 );
	m_ui.m_treeWidget->setHeaderHidden( true );
	m_ui.m_treeWidget->setSelectionMode( QAbstractItemView::NoSelection );

	connect( m_ui.m_okBtn, SIGNAL( clicked() ), this, SLOT( saveBrush() ) );
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

void AddBrushDialog::saveBrush()
{
	if ( !m_brushFile.m_brush )
		return;

	QString& name = m_brushFile.m_brush->getName();
	if( name.isEmpty() )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "Name cannot be empty!" ) );
		return;
	}
	QString filePath = m_brushFile.m_filePath;
	if ( m_brushFile.m_filePath.isEmpty() )
	{
		filePath = getProjectRootPath() + "/" + name + ".brush";
		QFileInfo fileinfo = QFileInfo( filePath );
		if( fileinfo.exists() )
		{
			QMessageBox::warning( this, tr( "Warning" ), ("Brush name : \"" + name + "\" already exists!") );
			return;
		}
	}
	else
	{
		QString newFilePath = getProjectRootPath() + "/" + name + ".brush";
		if ( filePath != newFilePath )
		{
			QFileInfo fileinfo = QFileInfo( newFilePath );
			if( fileinfo.exists() )
			{
				QMessageBox::warning( this, tr( "Warning" ), ("Brush name : \"" + name + "\" already exists!") );
				return;
			}
			// Delete old file
			QFile file( filePath );
			file.remove();
			filePath = newFilePath;
		}
	}
	if( saveBrushAsFile( m_brushFile.m_brush, filePath ) )
	{
		m_brushFile.m_filePath = filePath;
		accept();
	}
	else
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "Save brush file failed!" ) );
		reject();
	}
}
