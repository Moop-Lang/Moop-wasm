# L3 Turchin Actor Syntax - Quorum-like with Arrows

**Design Principles:**
- Quorum-like: English-like, natural language
- Arrows: `->` (message), `<-` (inheritance)
- Whitespace-delimited: NO parentheses, NO semicolons, NO 'end'
- Indentation-based blocks
- R-D-S tagging: Reversible by default, explicit D-term tags

---

## Core Syntax

### Actor Definition
```
actor <Name>
    role is "<description>"

    state has
        <key> -> <value>
        <key> -> <value>

    on <event>
        <statements>

    on <event>
        <statements>
```

### Inheritance (S-term)
```
actor Counter <- BaseActor
    role is "Count things"
    ...
```

### Message Passing

#### One-way Message (R-term, fire-and-forget)
```
target -> message arg1 arg2
self -> log "Hello"
OtherActor -> process data
```

#### Bidirectional/Query (R-term, request-response, reversible)
```
result <-> calculator add 5 3
value <-> database query key
response <-> service request data
```

**Note:** `<->` emphasizes structural reversibility - the operation can be undone and returns a value.

### D-term Tagging (explicit irreversibility)
```
@io display -> output result
@irreversible logger -> writeToDisk "log.txt"
```

---

## Control Flow (Whitespace-based)

### If Statement
```
if condition
    statement1
    statement2
```

### While Loop
```
while condition
    statement1
    statement2
```

### For Loop
```
for i in 1 to 10
    statement1
    statement2
```

**NO 'end', 'endif', 'endwhile'** - blocks end with dedentation!

---

## Statements

### State Assignment (uses arrow)
```
state.count -> state.count + 1
state.name -> "New Value"
```

### Local Variables (uses arrow)
```
let x -> 10
let result -> state.count + x
```

### Message Send
```
self -> log "Message"
OtherActor -> process data
@io display -> print result
```

### Logging
```
self -> log "Debug message"
```

---

## Expressions

### Numbers
```
42
3.14
```

### Strings
```
"hello world"
'single quotes work too'
```

### State References
```
state.count
state.name
```

### Local Variables
```
x
result
```

### Arithmetic
```
state.count + 1
x * 2
(a + b) / c
```

### Comparisons
```
state.count > 5
x <= 10
name == "Alice"
```

---

## Complete Example

```
actor Counter <- BaseActor
    role is "Count and log messages"

    state has
        count -> 0
        max -> 10

    on start
        self -> log "Starting counter"
        state.count -> 0

    on increment
        if state.count < state.max
            state.count -> state.count + 1
            self -> log "Count incremented"
        else
            @io display -> print "Max reached!"

    on reset
        state.count -> 0
        self -> log "Counter reset"

    on notify_other
        OtherActor -> process state.count
```

---

## Multi-Actor Communication

```
actor Sender <- BaseActor
    state has
        sent is 0

    on start
        Receiver -> ping
        state.sent is 1

actor Receiver <- BaseActor
    state has
        received is 0

    on ping
        self -> log "Received ping"
        state.received is 1
        Sender -> pong

actor Sender
    on pong
        self -> log "Received pong"
```

---

## R-D-S Classification

### R-term (Reversible - default)
```
// One-way (fire-and-forget)
state.count -> state.count + 1
let x -> 10
self -> log "message"
Actor -> event

// Bidirectional (request-response, emphasizes reversibility)
result <-> calculator add 5 3
value <-> service query
```

**All R-term operations are reversible by default and can be undone.**

### D-term (Dissipative - requires tag)
```
@io display -> print result
@irreversible file -> write "data.txt"
@io network -> sendHTTP url
```

**D-term operations are irreversible and require explicit tagging.**

### S-term (Structural)
```
actor Counter <- BaseActor     // Inheritance
Proto <- ParentProto           // Prototype chain
```

**S-term defines structure and relationships.**

---

## Key Differences from Traditional Syntax

| Traditional | L3 Turchin (Arrow-based) |
|------------|--------------------------|
| `function add(x, y)` | `on add` with `let x` |
| `if (x > 5) { ... }` | `if x > 5` with indent |
| `while (count < 10) { ... }` | `while count < 10` with indent |
| `target.send("msg", arg)` | `target -> msg arg` |
| `class Child extends Parent` | `actor Child <- Parent` |
| `x = 10;` | `let x -> 10` |
| `this.count = 5;` | `state.count -> 5` |
| `}` or `end` | dedent |

---

## Strict Mode

When `strict_mode` is enabled, all D-term operations **must** be explicitly tagged:

```
// ❌ Fails in strict mode (D-term without tag)
display -> print result

// ✅ Passes in strict mode (explicit tag)
@io display -> print result
```

---

## Implementation Notes

1. **Whitespace matters** - indentation defines blocks
2. **No punctuation** - arrows and 'is' are the only operators
3. **Natural language** - reads like English
4. **R-term by default** - reversible unless tagged
5. **Explicit D-terms** - irreversibility must be marked

---

## Grammar Summary

```
Program      := Actor+
Actor        := "actor" Name ["<-" Parent] Block
Block        := IndentedLines
Statement    := Assignment | MessageSend | ControlFlow | Log
Assignment   := Target "is" Expression
MessageSend  := [Tag] Target "->" Message [Args]
ControlFlow  := If | While | For
If           := "if" Condition Block
While        := "while" Condition Block
For          := "for" Var "in" Start "to" End Block
Tag          := "@io" | "@irreversible"
Target       := "state." Name | "self" | ActorName
Expression   := Number | String | StateRef | Arithmetic
```

---

## Comparison with Quorum

### Similar to Quorum:
✅ English-like keywords (`is`, `has`)
✅ Indentation-based blocks
✅ Natural language feel
✅ No punctuation noise

### Different from Quorum:
- Uses arrows (`->`, `<-`) instead of dot notation
- `is` for assignment instead of `=`
- No parentheses for function calls
- Explicit R-D-S tagging for reversibility

---

This is the **true L3 Turchin syntax** - Quorum-inspired with arrow operators and whitespace structure!
