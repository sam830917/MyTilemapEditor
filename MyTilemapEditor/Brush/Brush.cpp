#include "Brush/Brush.h"
#include "Utils/ProjectCommon.h"
#include <QLineEdit>
#include <QLabel>

static int constexpr MAX_BRUSHES( 128 );
static BrushType* g_registrarList[MAX_BRUSHES];
static int g_registrarBrushCount = 0;

BrushType::BrushType( const QString& displayName, BrushConstructorFunction constructorFunction )
	:m_displayName(displayName),
	m_constructorFunction(constructorFunction)
{
	g_registrarList[g_registrarBrushCount] = this;
	g_registrarBrushCount++;
}

Brush::Brush()
{
}

Brush::~Brush()
{
}

void Brush::paint( const QPoint& currentCoord, const MapInfo& mapInfo )
{
	setTile( currentCoord, getCurrentTile() );
}

void Brush::erase( const QPoint& currentCoord, const MapInfo& mapInfo )
{
	eraseTile( currentCoord );
}

QBoxLayout* Brush::createAddDialogUI()
{
	QBoxLayout* layout = new QBoxLayout( QBoxLayout::TopToBottom );
	layout->setContentsMargins( 0, 0, 0, 0 );

	QLineEdit* nameInput = new QLineEdit();
	nameInput->setText( m_name );
	QLabel* nameLabel = new QLabel;
	nameLabel->setText( "Name" );
	QBoxLayout* nameLayout = new QBoxLayout( QBoxLayout::LeftToRight );
	nameLayout->addWidget( nameLabel, 0, Qt::AlignTop );
	nameLayout->addWidget( nameInput, 0, Qt::AlignTop );

	layout->addLayout( nameLayout );
	QObject::connect( nameInput, &QLineEdit::textChanged, [=]( const QString &newValue ) { this->m_name = newValue; } );

	return layout;
}

void Brush::setTile( QPoint coordinate, TileInfo tileInfo )
{
	m_readyToPaintCoordList.push_back( TileModified( coordinate, tileInfo ) );
}

void Brush::eraseTile( QPoint coordinate )
{
	m_readyToPaintCoordList.push_back( TileModified( coordinate, TileInfo() ) );
}

QList<TileModified> Brush::popReadyToPaintCoordList()
{
	QList<TileModified> copy = m_readyToPaintCoordList;
	m_readyToPaintCoordList.clear();
	return copy;
}

STATIC QList<BrushType*> Brush::getAllBrushType()
{
	QList<BrushType*> typeList;
	for ( int i = 0; i < MAX_BRUSHES; ++i )
	{
		if ( g_registrarList[i] )
		{
			typeList.push_back( g_registrarList[i] );
		}
	}
	return typeList;
}
