Scriptable Brush
================

The implementation of the scriptable brush system which is based on the brush JavaScript files. 
A brush JavaScript file contains variables, painting logic, erasing logic and setup functions.

-  All brush files are loaded while editor is opening.
-  A brush file can create a brush instance by selecting the brush type in the drop-down list and clicking new brush 
   button on the brush window and assign variable through the editor. After a new brush instance is created, an XML 
   file will be generated for saving the assigned variable's value.
-  All of the brush instances will contain a name variable. This variable is for the XML file name and displaying name in the brush window.

.. figure:: ../images/scriptable_brush/brush_window.png
   :alt: MyTilemapEditor Preview

Default Types
-------------

Same Tile Brush
~~~~~~~~~~~~~~~

Same tile brush includes one variable which saves a tile. It can paint the specific tile into the map.

Large Item Brush
~~~~~~~~~~~~~~~~

Large item brush includes an array of tiles, a width value, and a height value. It can paint all the tiles in the array 
base on the width and height from left to right, top to bottom.

.. figure:: ../images/scriptable_brush/large_item_brush.png
   :alt: MyTilemapEditor Preview

Random Brush
~~~~~~~~~~~~

Random brush includes an array of tiles. When the paint function is called, it will pick a random tile in the array 
and paint the tile into the map.

.. figure:: ../images/scriptable_brush/random_brush.png
   :alt: MyTilemapEditor Preview

Road Brush
~~~~~~~~~~

Road brush includes an array of tiles with grids. The feature of this brush is to paint a continuing road. 
Based on the top, right, bottom, and left side of the tile, it will try to paint the best tile, 
in the array of tiles, in the giving coordinate. In order to paint a road, the user has to define 
the road information, helping editor to determine that every edge of tile should connect the way or not.

.. figure:: ../images/scriptable_brush/road_brush.png
   :alt: MyTilemapEditor Preview

For example, if the user tries to paint in the (1, 1) coordinate, the editor will check the tile on 
the top coordinate that is this tile in the array of defined tile. If so, get the value of the bottom 
edge in the top coordinate, and do the similar steps with the left, bottom, and right sides’ coordinate. 
As the result, the editor can pick a tile that fits within the (1, 1) coordinate, but if there is no best 
solution, it will stay empty tile.
   
.. figure:: ../images/scriptable_brush/road_brush2.png
   :alt: MyTilemapEditor Preview
   
Land Brush
~~~~~~~~~~

Land brush includes an array of tiles with grids. The feature of this brush is to paint a 9- slicing terrain 
into the map. Land brush is similar to the road brush, but instead of defining the edges of the tile, the user 
should define the corner of the tile.



Custom Type
-----------