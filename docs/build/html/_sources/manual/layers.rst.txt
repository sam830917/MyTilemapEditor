Layers
======

A map supports various sorts of content, and this content is organized into two different layers.
There are two types of layers, Tile Layer and Marker Layer. 
The order of the layers determines the rendering order of your content.
Layers can be hidden, made only partially visible and can be locked. 

.. figure:: ../images/layers/layer_window.png
   :alt: Layer window

Layer Types
-----------

Tile Layers
~~~~~~~~~~~

Tile layers provide a way of storing a large area filled with tile data. 
The data is a simple array of the tileset references and the index of tile.

Marker Layers
~~~~~~~~~~~~~

Marker Layer allows the user to mark certain areas in the map.
You can treat the Marker Layer as true/false layer.
When the tiles are marked, they will have low opacity rectangle in the layer.
You can change the color for the Marker Layer by clicking the color button on the layer.

.. figure:: ../images/layers/select_color.png
   :alt: Change Marker Layer Color

Use Cases
---------

You can have custom data for the output XML file by using Marker Layer. For example, to mark an area as collider.

.. figure:: ../images/layers/marker_layer_eg1.png
   :alt: Layer window
   
.. figure:: ../images/layers/marker_layer_eg2.png
   :alt: Layer window

Then you can parse the XML file by the Marker Layer name in your game.
Here is snippet.

.. code:: xml

    <Layers>
        <Layer name="Collider" isLock="false" isVisible="true" type="MARKER" color="255,0,4" layerIndex="0"/>
    </Layers>
    <Tiles layerIndex="0">
        <Tile index="64"/>
        <Tile index="94"/>
        <Tile index="124"/>
        <Tile index="154"/>
        <Tile index="184"/>
        <Tile index="214"/>
    </Tiles>
