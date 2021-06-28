# unsure_ownership_ptr

unsure_ownership_ptr is a c++ smarter pointer based on unique_ptr.

It's very similar to unique_ptr. But you can let it point to an object with no ownership. 

You can use has_ownership() to judge if the pointer has ownership.

This smart pointer will destructed the object only if it has ownership.