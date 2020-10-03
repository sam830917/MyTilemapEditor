#include "Core/LayerInfo.h"

LayerInfo::LayerInfo( const QString& name, bool isLock /*= false*/, bool isVisible /*= true */ )
	:m_name(name),
	m_isLock(isLock),
	m_isVisible(isVisible)
{

}

LayerInfo::~LayerInfo()
{

}
