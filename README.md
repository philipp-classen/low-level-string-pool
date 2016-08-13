# low-level-string-pool
Low-level string pool in C++. Strings are shared and their references will not be invalidated.

Was part of a small experiment where memory was crucial. It is very low level,
uses C strings and requires manual memory management, thus using it is error-prone.

On the plus side, the memory usage should be quite low, and you get an object
which you can safely reference as it will not be invalidated if the string pool grows.
As string sharing is guaranteed, it can use pointer comparison for equality.

The implementation takes advantage of Google's `sparse_hash_map` if available.
Otherwise, it falls back to `std::unordered_map`. Actually, the savings from
using `sparse_hash_map` are very significant. More significant than `char*`
vs `std::string`.
