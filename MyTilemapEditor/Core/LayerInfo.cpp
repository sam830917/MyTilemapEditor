#include "Core/LayerInfo.h"

LayerInfo::LayerInfo( const QString& name, bool isLock /*= false*/, bool isVisible /*= true */ )
	:m_name(name),
	m_isLock(isLock),
	m_isVisible(isVisible)
{

}

LayerInfo::LayerInfo( const LayerInfo& copyLayerInfo )
{
	m_name = copyLayerInfo.m_name;
	m_isLock = copyLayerInfo.isLock();
	m_isVisible = copyLayerInfo.isVisible();
	m_type = copyLayerInfo.getLayerType();
	m_color = copyLayerInfo.getColor();
}

LayerInfo::~LayerInfo()
{

}
