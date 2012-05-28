Scene definitions
=================

To create a new scene type:

1. Edit src/scene/myscene.cpp
1. Implement your scene-class which inheirits from Scene.
1. Use REGISTER_SCENE_TYPE(MyScene, "MySceneTypeName") to register it with the type system.
1. If needed, specialize SceneTraits::create or SceneTraits::metadata.
   * Metadata is used to instruct the editor about what kind of resources/data the scene deals with.
1. Allocate a new instance using SceneFactory::create("MySceneTypeName", size)
