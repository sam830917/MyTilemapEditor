#include "Widget/AddBrushDialog.h"
#include "Brush/BrushParser.h"
#include "Utils/ProjectCommon.h"
#include "BrushUI/ListContainerBase.h"
#include <QSpacerItem>
#include <QMessageBox>
#include <QFileInfo>
#include <QLineEdit>

AddBrushDialog::AddBrushDialog( QWidget* parent /*= Q_NULLPTR */ )
{
	m_ui.setupUi( this );
	m_ui.m_treeWidget->setColumnCount( 2 );
	m_ui.m_treeWidget->headerItem()->setText( 0, "Name" );
	m_ui.m_treeWidget->headerItem()->setText( 1, "Value" );
	m_ui.m_treeWidget->setSelectionMode( QAbstractItemView::NoSelection );

	connect( m_ui.m_okBtn, SIGNAL( clicked() ), this, SLOT( saveBrush() ) );
}

void AddBrushDialog::addItem( QList<AddBrushItem*> items )
{
	m_brushUI = items;
	QLineEdit* lineInput = new QLineEdit(this);
	QObject::connect( lineInput, &QLineEdit::textChanged, [=]( const QString& newValue ) { m_name = newValue; } );
	lineInput->setText( m_name );
	QTreeWidgetItem* wItem = new QTreeWidgetItem();

	wItem->setText( 0, "Name" );
	m_ui.m_treeWidget->addTopLevelItem( wItem );
	m_ui.m_treeWidget->setItemWidget( wItem, 1, lineInput );

	for ( AddBrushItem* item : items )
	{
		m_ui.m_treeWidget->addTopLevelItem( item->m_treeItem );
		m_ui.m_treeWidget->setItemWidget( item->m_treeItem, 1, item->m_widgetItem );
		if ( isListType( item->m_type ) )
		{
			ListContainerBase* c = dynamic_cast<ListContainerBase*>( item->m_treeItem );
			c->setExpanded(true);
			c->attachedTreeWidget();
		}
	}
}

void AddBrushDialog::saveBrush()
{
	if( m_name.isEmpty() )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "Name cannot be empty!" ) );
		return;
	}
	QString filePath = getProjectRootPath() + "/" + m_name + ".brush";
	if ( !m_isModify )
	{
		QFileInfo fileinfo = QFileInfo( filePath );
		if( fileinfo.exists() )
		{
			QMessageBox::warning( this, tr( "Warning" ), ("Brush name : \"" + m_name + "\" already exists!") );
			return;
		}
		if( m_brushParser->saveBrushAsFile( m_brushUI, filePath, m_brushFilePath ) )
		{
			m_brushFilePath = filePath;
			accept();
		}
		else
		{
			QMessageBox::warning( this, tr( "Warning" ), tr( "Save brush file failed!" ) );
			reject();
		}
	}
	else
	{
		if( m_brushParser->modifyBrushAsFile( m_brushUI, filePath, m_brushIndex ) )
		{
			m_brushFilePath = filePath;
			accept();
		}
		else
		{
			QMessageBox::warning( this, tr( "Warning" ), tr( "Save brush file failed!" ) );
			reject();
		}
	}
}
