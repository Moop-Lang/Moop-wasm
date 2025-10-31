# The Three Arrows of Moop

The Moop stack uses **three arrows** to represent all operations, aligned with the Universal Meta Equation (UME):

```
->   One-way (R-term, reversible)
<->  Bidirectional (R-term, reversible with return value)
<-   Inheritance (S-term, structural)
```

Plus optional D-term tagging: `@io`, `@irreversible`

---

## 1. One-Way Arrow `->` (R-term)

**Meaning:** Fire-and-forget message or assignment (reversible by default)

### Usage:

#### Assignment
```
state.count -> 10
let x -> 5
```

#### Message Passing
```
self -> log "Hello"
Actor -> process data
OtherActor -> notify
```

### Characteristics:
- **R-term** (reversible)
- **One-way** (no return value expected)
- **Asynchronous** (fire-and-forget)
- **Can be undone** (reversibility tracked)

### Examples:
```
actor Counter
    state has
        count -> 0

    on increment
        state.count -> state.count + 1
        self -> log "Incremented"
        Logger -> record state.count
```

---

## 2. Bidirectional Arrow `<->` (R-term)

**Meaning:** Request-response with explicit reversibility tracking

### Usage:

#### Synchronous Query
```
result <-> calculator add 5 3
value <-> database query key
response <-> service request data
```

#### Emphasis on Reversibility
```
// These operations can be undone
state <-> transform input
output <-> process input
```

### Characteristics:
- **R-term** (reversible)
- **Bidirectional** (returns a value)
- **Synchronous** (wait for response)
- **Explicitly reversible** (operation can be undone)
- **Traceable** (reversibility log maintained)

### Examples:
```
actor Calculator
    on compute
        result <-> math add 10 20
        state.result -> result
        response <-> formatter format result
        display -> show response
```

### Why Use `<->` Instead of `->`?

| Aspect | `->` | `<->` |
|--------|------|-------|
| Return value | No | Yes |
| Synchronous | No | Yes |
| Reversibility | Implicit | Explicit |
| Use case | Fire-and-forget | Request-response |

---

## 3. Inheritance Arrow `<-` (S-term)

**Meaning:** Structural inheritance relationship

### Usage:

#### Actor Inheritance
```
actor Counter <- BaseActor
actor AdvancedCounter <- Counter
```

#### Prototype Chain
```
MathProto <- ObjectProto
CalculatorProto <- MathProto
```

### Characteristics:
- **S-term** (structural)
- **Defines hierarchy** (parent-child relationship)
- **Canonical paths** (Proto.Actor.Func)
- **Homoiconic** (structure is data)

### Examples:
```
actor BaseActor
    state has
        id -> 0
        created -> timestamp

actor Counter <- BaseActor
    state has
        count -> 0

    on increment
        state.count -> state.count + 1
```

---

## R-D-S Summary

### R-term (Reversible)
- `->` one-way
- `<->` bidirectional
- Both are **reversible by default**
- Can be undone/rolled back

### D-term (Dissipative)
- Requires `@io` or `@irreversible` tag
- **Not reversible**
- Crosses coordination boundary

### S-term (Structural)
- `<-` inheritance
- Defines relationships
- Structure as data

---

## Complete Example

```
// S-term: Inheritance
actor Calculator <- BaseActor
    role is "Perform calculations"

    state has
        result -> 0
        history -> []

    on compute
        // <-> bidirectional (get result back)
        value <-> math add 10 20
        state.result -> value

        // -> one-way (fire-and-forget)
        self -> log "Computed result"
        Logger -> record value

        // D-term: Irreversible I/O
        @io display -> print value

// S-term: Inheritance
actor ScientificCalculator <- Calculator
    on advanced_compute
        // <-> request-response (reversible)
        result <-> math sqrt 144
        state.result -> result
```

---

## Implementation Status

### Implemented:
- ✅ `->` one-way arrow (assignment + messages)
- ✅ `<-` inheritance arrow (S-term)
- ✅ `@io` and `@irreversible` tags (D-term)

### To Implement:
- ⏳ `<->` bidirectional arrow (synchronous request-response)
- ⏳ Return value handling
- ⏳ Reversibility tracking for `<->`

---

## Why Three Arrows?

### LLM-Friendly
- **Minimal**: Only 3 operators for all operations
- **Visual**: Arrows show direction of data/control
- **Consistent**: Same pattern everywhere

### Semantically Rich
- `->` = "flows to" (one-way)
- `<->` = "exchanges with" (two-way)
- `<-` = "inherits from" (structure)

### Reversibility-First
- R-term by default (`->`, `<->`)
- D-term explicit (`@io`, `@irreversible`)
- S-term structural (`<-`)

---

## Grammar

```
Statement     := Assignment | OneWay | Bidirectional | Inheritance
Assignment    := Target "->" Expression
OneWay        := Target "->" Message [Args]
Bidirectional := Target "<->" Message [Args]
Inheritance   := Actor "<-" Parent
Tagged        := "@" Tag Target "->" Message
Tag           := "io" | "irreversible"
```

---

## Comparison with Traditional Syntax

| Traditional | Moop (Three Arrows) |
|------------|---------------------|
| `x = 5;` | `x -> 5` |
| `obj.method(arg)` | `obj -> method arg` |
| `result = obj.query(key)` | `result <-> obj query key` |
| `class Child extends Parent` | `actor Child <- Parent` |
| `file.write(data)` | `@io file -> write data` |

---

## Next Steps

1. Implement `<->` parser support
2. Add synchronous execution for `<->`
3. Track reversibility operations
4. Create examples using all three arrows
5. Update JavaScript runtime with `<->`

**The three arrows embody the Moop philosophy: minimal, visual, reversible-first! 🏹**
