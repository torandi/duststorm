FROBNICATORS DEMO ENGINE
==========

Dependencies
------------
libglm-dev
libassim-dev
opencl-headers

Usage
===========

Scenes
-------
TODO

Particles
--------
TODO

Position tables and interpolation
------
TODO

Lights
------
Relevant structures:

* Shader::lights_data_t: The structure corresponding to the shader uniform light data struct, containing all scene lights and ambient light data
* Light : struct corresponding to a single light in the shader
* MovableLight : Wrapper around light, inheriting movable object. To update the underlying light, call update()
* LightsData: A wrapper around lights_data_t, this is the class you probably want to use (see below)

LightsData contains all you need. To handle individual lights you have access to the lights-array, containing MovableLights. 
To change ambient light or number of lights you call the corresponding method (returns referenses to the variables).

Finally, to upload to the shader call Shader::upload_lights(lights_data). This calls the LightsData::shader_data() function, which will call update on the movable lights.




