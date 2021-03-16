#include "Widget/ConsoleWidget.h"
#include <QTextBrowser>
#include <QBoxLayout>
#include <QColor>
#include <QMenu>

ConsoleWidget* g_consoleWidget = nullptr;

ConsoleWidget::ConsoleWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_textBrowser = new QTextBrowser(this);

	QWidget* placeholder = new QWidget( this );
	QBoxLayout* layout = new QBoxLayout( QBoxLayout::TopToBottom, placeholder );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( m_textBrowser );
	setWidget( placeholder );
	g_consoleWidget = this;

	m_textBrowser->setContextMenuPolicy( Qt::CustomContextMenu );
	connect( m_textBrowser, &QTextBrowser::customContextMenuRequested, this, &ConsoleWidget::popupRightClickMenu );
}

void ConsoleWidget::print( const QString& text )
{
	m_textBrowser->setTextColor( QColor( 255, 255, 255 ) );
	m_textBrowser->append( text );
}

void ConsoleWidget::printError( const QString& text )
{
	m_textBrowser->setTextColor( QColor( 255, 0, 0 ) );
	m_textBrowser->append( text );
}

void ConsoleWidget::popupRightClickMenu( const QPoint& pos )
{
	QMenu menu;
	menu.addAction( "Clear", [&](){ m_textBrowser->clear(); } );
	menu.exec( QCursor::pos() );
}
