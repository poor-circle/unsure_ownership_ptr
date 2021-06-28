# unsure_ownership_ptr

unsure_ownership_ptr is a c++ smarter pointer. 

It's very similar to unique_ptr and based on it. But you can let it point to an object with no ownership. 

You can use has_ownership() to judge if the pointer has ownership.

This smart pointer will destruct the object only if it has ownership.