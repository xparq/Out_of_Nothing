This "dangling" dir (neither in `Engine`, nor in `app`) is a legacy pool of
entangled, interoperating bits that both the engine and the app use in very
tight integration, so cleanly separating concerns here is not easy.

It still must be done, nonetheless, so this directory should eventually
evaporate — or, in fact, move to a standalone repository of models that
apps can share and (re)use.

*
Even Math may need to be split to provide generic, ubiquitous services right
from the Engine directly, and optional specialized extra services needed (and
provided) by a particular model (or models — so, another sharing opportunity!).

But, because "math" in general is almost exclusively used only for modelling,
and partly because it's too early yet, it seems much safer to start out with
just having a local Model/Math component (thus considered "specialized",
despite math being inherently universal!), instead of risking to prematurely
lock in a half-baked global math API up in the Engine.

(The same is even more true for physics, of course.)
