#include "AddTilesetDialog.h"
#include "../Utils/XmlUtils.h"
#include "../Utils/ProjectCommon.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>

AddTilesetDialog::AddTilesetDialog( QWidget* parent /*= Q_NULLPTR */ )
	:QDialog( parent )
{
	m_ui.setupUi( this );

	connect( m_ui.m_getImageButton, SIGNAL( clicked() ), this, SLOT( addImageFile() ) );
	connect( m_ui.m_okBtn, SIGNAL( clicked() ), this, SLOT( saveTileset() ) );
	connect( m_ui.m_heightValueBox, SIGNAL( valueChanged( int ) ), this, SLOT( checkTileSizeAndUpdate( int ) ) );
	connect( m_ui.m_widthValueBox, SIGNAL( valueChanged( int ) ), this, SLOT( checkTileSizeAndUpdate( int ) ) );
}

void AddTilesetDialog::updateTilePreview()
{
	QGraphicsScene* scene = new QGraphicsScene();
	m_ui.m_tilePreview->setScene( scene );
	//QGraphicsPixmapItem* item = new QGraphicsPixmapItem( img->copy( 32, 32, 32, 32 ) );
	QGraphicsPixmapItem* item = new QGraphicsPixmapItem( *m_tilesetImage );
	scene->addItem( item );
	
	QPen outlinePen( Qt::black );
	outlinePen.setWidth( 3 );
	outlinePen.setStyle( Qt::PenStyle::DotLine );
	
	// draw grid
	QSize tileSize = QSize( m_ui.m_widthValueBox->value(), m_ui.m_heightValueBox->value() );
	QSize size = m_tilesetImage->size();
	int vLineCount = size.width() / tileSize.width();
	int hLineCount = size.height() / tileSize.height();
	
	for( int v = 1; v < vLineCount; ++v )
	{
		scene->addLine( v * tileSize.width(), 0, v * tileSize.width(), size.height(), QPen( Qt::red ) );
	}
	for( int h = 1; h < hLineCount; ++h )
	{
		scene->addLine( 0, h * tileSize.height(), size.width(), h * tileSize.height(), QPen( Qt::red ) );
	}
}

Tileset* AddTilesetDialog::getResult()
{
	if ( m_tilesetImage == Q_NULLPTR )
	{
		return Q_NULLPTR;
	}
	QSize tileSize = QSize( m_ui.m_heightValueBox->value(), m_ui.m_widthValueBox->value() );
	Tileset* tileset = new Tileset( m_ui.m_nameBox->text(), m_imagePath, m_tilesetImage, tileSize );
	tileset->setFilePath( m_saveFilePath );

	return tileset;
}

void AddTilesetDialog::saveTileset()
{
	if ( m_tilesetImage == Q_NULLPTR )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "You did not import any image!" ) );
		return;
	}
	if( m_ui.m_nameBox->text().isEmpty() )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "Name cannot be empty!" ) );
		return;
	}

	// Save as XML
	QFileInfo imgFileInfo(m_imagePath);
	QString imgFileName = m_ui.m_nameBox->text() + "." + imgFileInfo.suffix();
	QString newImagePath = getProjectRootPath() + "/" + imgFileName;

	XmlDocument* xmlDocument = new XmlDocument();
	XmlElement* root = xmlDocument->NewElement( "Tileset" );
	XmlElement* tilesetEle = xmlDocument->NewElement( "Image" );
	tilesetEle->SetAttribute( "path", imgFileName.toStdString().c_str() );
	QSize tileSize = QSize( m_ui.m_widthValueBox->value(), m_ui.m_heightValueBox->value() );
	QString size = QString( "%1,%2" ).arg( tileSize.width() ).arg( tileSize.height() );
	tilesetEle->SetAttribute( "size", size.toStdString().c_str() );
	xmlDocument->LinkEndChild( root );
	root->LinkEndChild( tilesetEle );

	QString filePath = getProjectRootPath() + "/" + m_ui.m_nameBox->text() + ".tileset";
	saveXmlFile( *xmlDocument, filePath );
	m_saveFilePath = filePath;

	QFile::copy( m_imagePath, newImagePath );

	accept();
}

void AddTilesetDialog::addImageFile()
{
	QString dlgTitle = "Select Image";
	QString aFileName=QFileDialog::getOpenFileName( this, dlgTitle, QDir::currentPath(), tr( "pic file(* png * jpg * bmp)" ) );
	if( aFileName.isEmpty() ) return;

	QPixmap* img = new QPixmap();
	if( !(img->load( aFileName )) )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "Failed to import image!" ) );
		delete img;
		return;
	}
	m_imagePath = aFileName;
	m_tilesetImage = img;
	if ( m_ui.m_nameBox->text().isEmpty() )
	{
		QFileInfo imgFileInfo(aFileName);
		m_ui.m_nameBox->setText( imgFileInfo.completeBaseName() );
	}
	updateTilePreview();
}

void AddTilesetDialog::checkTileSizeAndUpdate( int val )
{
	if( val <= 0 )
	{
		return;
	}
	if( m_tilesetImage == Q_NULLPTR )
	{
		return;
	}
	updateTilePreview();
}
