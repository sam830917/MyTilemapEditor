#include "Brush/BrushCommon.h"
#include "Core/TileInfo.h"
#include <typeinfo>

template<typename T>
eVariableType getVariableType( const T& )
{
	const std::type_info& type = typeid(T);

	if ( type == typeid( QString ) )
	{
		return eVariableType::STRING;
	}
	if( type == typeid( TileInfo ) )
	{
		return eVariableType::TILE;
	}

	return eVariableType::UNKNOWN;
}
template eVariableType getVariableType( const QString& a );
template eVariableType getVariableType( const TileInfo& a );