# Developer Notes

Information useful to developers of this library. In other words, things that
even I have trouble remembering.

## Coroutine Linked List

The `COROUTINE()` macro creates an instance of `Coroutine` whose constructor
inserts the coroutine instance into a singly linked list. This allows the
`CoroutineScheduler` to iterate over the linked list and allow the coroutines to
execute in a round-robin fashion.

It always takes me a few minutes to understand how I implemented the singly
linked list, so here are some notes to myself.

Each instance of `Coroutine` contains an `mNext` variable that points to the
next instance of `Coroutine`. The last instance has an `mNext` that contains a
`nullptr`.

```
            getRoot()         Coroutine         Coroutine
            +--------+        +--------+       +--------+
            |        |    --->|        |    -->|        |     --> nullptr
            |        |   /    |        |   /   |        |    /
            | root-------     | mNext------    | mNext-------
            +-^------+        +-^------+       +-^------+
Coroutine     |                /                /
Scheduler     |               /                /
+---------+   |              /                /
|         |   |             /                /
|mCurrent------------------------------------
+---------+
```

The first element of the linked list is defined by the pointer stored in a
static variable called `root` inside the `Coroutine::getRoot()` static function.
Static variables inside functions are guaranteed to be initialized only once
upon the first call to `getRoot()` and will be initialized to the default value
of the type (`nullptr` in this case).

The `CoroutineScheduler` uses a variable called `mCurrent` to keep track of the
`Coroutine` which is currently being handled. The important part is to realize
that `mCurrent` does *not* hold a pointer to `Coroutine` (i.e. `Coroutine*`) but
it holds a pointer to a pointer to `Coroutine` (i.e. `Coroutine**`). Similarly,
`Coroutine::getRoot()` returns a *pointer* to the `root` (i.e. `Coroutine**`),
not the value of `root` (i.e. `Coroutine*`).

Using the pointer to a pointer (i.e. `Coroutine**`) to represent the "current"
coroutine has a number of advantages and simplifies the code that traverses the
linked list.

* The root node can be treated the same as a regular `Coroutine` node.
    * See `CoroutineScheduler::listCoroutines()` to see this in action.
* It allows new nodes to be inserted just after the "current" node.
    * The `mCurrent` points to the `mNext` of the previous node, so with just a
      single pointer, both the previous node and the current node can be
      modified. If the "current" node was represented by a `Coroutine*`, it
      would not be possible to update the previous node because this is a singly
      linked list.
    * See `Coroutine::insertAtRoot()` and `Coroutine::insertSorted()` to see how
      this work.
* It also allows the "current" node to be deleted from the linked list, although
  this capability is not used in the library.
