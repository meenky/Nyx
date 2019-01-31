# Nyx

  The goal of Nyx is to provide a relatively simple way to define an
arbitrarily complex data structure that can be 'rendered' into a target
programming language complete with serialization and deserialization. This goal
assumes that every target programming language provides user definable
aggregate types with named fields and binary I/O capabilities. This is a
relatively safe assumtion as long as the target programming language isn't
[esoteric](https://en.wikipedia.org/wiki/Esoteric_programming_language).

