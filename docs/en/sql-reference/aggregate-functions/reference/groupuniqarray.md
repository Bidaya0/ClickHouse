---
slug: /en/sql-reference/aggregate-functions/reference/groupuniqarrayarray
sidebar_position: 154
---

# groupUniqArray

Syntax: `groupUniqArrayArray(x)` or `groupUniqArrayArray(max_size)(x)`

Creates an array from different argument values. Memory consumption is the same as for the [uniqExact](../../../sql-reference/aggregate-functions/reference/uniqexact.md) function.

The second version (with the `max_size` parameter) limits the size of the resulting array to `max_size` elements.
For example, `groupUniqArrayArray(1)(x)` is equivalent to `[any(x)]`.
