#include "AddMapDialog.h"
#include "../Utils/ProjectCommon.h"
#include "../Utils/XmlUtils.h"
#include <QMessageBox>

AddMapDialog::AddMapDialog( QWidget* parent /*= Q_NULLPTR */ )
	:QDialog( parent )
{
	m_ui.setupUi( this );

	connect( m_ui.m_okBtn, SIGNAL( clicked() ), this, SLOT( saveMap() ) );
}

MapInfo* AddMapDialog::getResult() const
{
	MapInfo* mapInfo = new MapInfo( m_ui.m_mapWidthValueBox->value(), m_ui.m_mapHeightValueBox->value(),
		m_ui.m_tileWidthValueBox->value(), m_ui.m_tileHeightValueBox->value() );
	mapInfo->setFilePath(m_filePath);
	mapInfo->setName( m_ui.m_nameBox->text() );

	return mapInfo;
}

void AddMapDialog::saveMap()
{
	if( m_ui.m_nameBox->text().isEmpty() )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "Name cannot be empty!" ) );
		return;
	}

	// Save as XML
	XmlDocument* xmlDocument = new XmlDocument();
	XmlElement* root = xmlDocument->NewElement( "Map" );
	QString mapSize = QString( "%1,%2" ).arg( m_ui.m_mapWidthValueBox->value() ).arg( m_ui.m_mapHeightValueBox->value() );
	root->SetAttribute( "mapSize", mapSize.toStdString().c_str() );
	QString tileSize = QString( "%1,%2" ).arg( m_ui.m_tileWidthValueBox->value() ).arg( m_ui.m_tileHeightValueBox->value() );
	root->SetAttribute( "tileSize", tileSize.toStdString().c_str() );
	xmlDocument->LinkEndChild( root );

	QString filePath = getProjectRootPath() + "/" + m_ui.m_nameBox->text() + ".map";
	saveXmlFile( *xmlDocument, filePath );
	m_filePath = filePath;

	accept();
}
