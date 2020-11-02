#pragma once

#include <QList>
#include <QMap>
#include "Utils/ProjectCommon.h"
#include "Brush/BrushCommon.h"
#include "Brush/BrushHelper.h"

QT_FORWARD_DECLARE_CLASS( QJSEngine );

struct BrushItemInfo
{
	eItemType m_itemType = eItemType::UNKNOWN;
	QString m_labelName;
	QString m_id;
};

struct BrushFile
{
	QString m_filePath;
	QList<BrushItemInfo> m_itemList;
};

class BrushParser
{
public:
	BrushParser();
	~BrushParser();

	QList<AddBrushItem*> createBrushUI( const QString& brushName );
	QList<AddBrushItem*> createBrushUIByCurrentBrush( int index );
	bool loadBrushFile( const QString& filePath );
	bool saveBrushAsFile( QList<AddBrushItem*> items, const QString& saveFilePath, const QString& brushFilePath, QJSEngine* jsEngine = nullptr );
	bool modifyBrushAsFile( QList<AddBrushItem*> items, const QString& saveFilePath, int brushIndex );
	void deleteBrush( int index );
	void deleteAllBrush();

	QList<TileModified> getPaintMapResult( int brushIndex, const QPoint& coord, eDrawTool tool );
	QString getFilePathByIndex( int index );
	QString getFileName( int index );
	QString getBrushFilePathByIndex( int index );
	QString getBrushFileName( int index );

private:
	void initialBrushFile( const QString& filePath );

	void createStringUI( const QString& labelName, QList<AddBrushItem*>& itemList, const QString& value = QString() );
	void createTileUI( const QString& labelName, QList<AddBrushItem*>& itemList, const TileInfo& value = TileInfo() );
	void createTileListUI( const QString& labelName, QList<AddBrushItem*>& itemList, QList<TileInfo> tileList );
	void createTileGridUI( const QString& labelName, QList<AddBrushItem*>& itemList );
	void createIntUI( const QString& labelName, QList<AddBrushItem*>& itemList, int value = 0 );

	QJSEngine* createJSEngine( const QString& filePath );

private:
	QList<QJSEngine*> m_brushes;
	// Key = Brush Name
	QMap<QString, BrushFile> m_brushItemMap;
};