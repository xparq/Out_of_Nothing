NOTE:

  Still a prototype, with missing and unclear scope/interfaces/services (e.g.
  this is mostly just a system/backend/foundations + UI + app logic/controller
  layer, with no support for rendering or physics/modeling (TBD!)), but slowly
  graduating to be useful/usable on its own, nonetheless.

  As things mature, bits that generalize well should percolate here,
  separated (much better) from app-specifics.

*
- Upper-cased dirs are more-or-less structural elements (subsystems)
- Lower-cased dirs tend to contain "orthogonal, cross-cutting" internal
  and/or housekeeping tools, which are also available for applications
