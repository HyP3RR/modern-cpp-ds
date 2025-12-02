/*

alignas(alignof(T)) std::byte store[sizeof(T)];
type aligned storage! best for std::optional, so it never default constructs and
we manage this

for std::any, know the basic place holder with runtime polymorphism to simulate
std::any, we can use type erased approach by templated constructors for storing
functions of fixed return type.

std::any, can implement small size optimisation! avoid that dynamic allocation
overhead!
*/