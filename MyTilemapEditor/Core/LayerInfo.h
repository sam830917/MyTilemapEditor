#pragma once

#include <QString>
#include <QColor>

enum class eLayerType
{
	TILE_LAYER,
	MARKER_LAYER
};

class LayerInfo
{
public:
	LayerInfo( const QString& name = "Layer 1", bool isLock = false, bool isVisible = true );
	LayerInfo( const LayerInfo& copyLayerInfo );
	~LayerInfo();

	QString		getNmae() const { return m_name; }
	bool		isLock() const { return m_isLock; }
	bool		isVisible() const { return m_isVisible; }
	eLayerType	getLayerType() const { return m_type; }
	QColor		getColor() const { return m_color; }

	void		setName( const QString& name ) { m_name = name; }
	void		setIsLock( bool isLock ) { m_isLock = isLock; }
	void		setIsVisible( bool isVisible ) { m_isVisible = isVisible; }
	void		setLayerType( eLayerType type ) { m_type = type; }
	void		setColor( QColor color ) { m_color = color; }

private:
	QString m_name;
	bool m_isLock = false;
	bool m_isVisible = true;
	eLayerType m_type = eLayerType::TILE_LAYER;
	QColor m_color = QColorConstants::Yellow;
};