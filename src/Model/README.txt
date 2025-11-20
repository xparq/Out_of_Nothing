*
This dir (neither in `Engine`, nor in `app`) is suspected to become a kind
of "3rd-party space" for models; a standalone lib/repository that apps can
share/(re)use. (Mostly different flavors of physics, presumably.)

Currently it's a legacy pool of entangled, interoperating bits that both
the engine and the app use, in very tight integration, so cleanly separating
concerns here is not easy. It still must be done, nonetheless. It's even a
mix of engine-level *metamodel* (like entity/player abstractions), and also
random app-specific model details! :-/

*
[metamodel-ecs]
The Metamodel aspecs should migrate to the Szim engine, probably as an ECS
core. And the app-related specifics then should just go there (app/model).

*
[model vs. maths]

Even Math may need to be split to provide generic, ubiquitous services right
from the Engine directly, and optional specialized extra services needed (and
provided) by a particular model (or models — so, another sharing opportunity!).

But, because "math" in general is almost exclusively used only for modelling,
and partly because it's too early yet, it seems much safer to start out with
just having a local Model/Math component (thus considered "specialized",
despite math being inherently universal!), instead of risking to prematurely
lock in a half-baked global math API up in the Engine.

(The same is even more true for physics, of course.)

*
[model vs. physics]

The world model itself should be a lot more generic than the physics:

A particular physics impl. may only be applied to one model (being compatible
with the entity store, entity props, space (dimensionality, metrics), etc.)
— except maybe via different world adapters, in case the physics internals
are so flexible, but I don't think that would be practical.

OTOH, a model could use any number of different types of physics. Which is
kinda the point: the physics should be hot-pluggable!
