// L3 Turchin Actor Runtime - JavaScript Implementation
// Pure JS implementation (no WASM required)
// Mirrors the C implementation from src/l3_turchin.c

class L3ActorState {
    constructor() {
        this.data = new Map();
    }

    get(key) {
        return this.data.get(key) || '0';
    }

    set(key, value) {
        this.data.set(key, String(value));
    }
}

class L3ExecutionContext {
    constructor(state, data, runtime, actorId) {
        this.state = state;
        this.data = data;
        this.runtime = runtime;
        this.actorId = actorId;
        this.locals = new Map();
    }

    setLocal(key, value) {
        this.locals.set(key, String(value));
    }

    getLocal(key) {
        return this.locals.get(key);
    }
}

class L3Actor {
    constructor(name, role, stateData, handlers) {
        this.name = name;
        this.role = role;
        this.state = new L3ActorState();
        this.handlers = handlers;

        // Initialize state
        for (const [key, value] of Object.entries(stateData)) {
            this.state.set(key, value);
        }
    }
}

class L3ActorRuntime {
    constructor() {
        this.actors = new Map();
        this.messageQueues = new Map();
        this.nextActorId = 1;
        this.running = true;
        this.logBuffer = [];
    }

    log(message) {
        this.logBuffer.push(message);
        console.log(message);
    }

    parseActor(code) {
        const lines = code.split('\n').map(line => line.trimEnd());
        let i = 0;

        // Parse actor name
        let actorName = null;
        let role = '';
        const stateData = {};
        const handlers = [];

        while (i < lines.length) {
            const line = lines[i];
            const trimmed = line.trim();

            if (trimmed.startsWith('actor ')) {
                actorName = trimmed.substring(6).trim();
            } else if (trimmed.startsWith('role is ')) {
                role = trimmed.substring(8).trim().replace(/^["']|["']$/g, '');
            } else if (trimmed === 'state has') {
                i++;
                while (i < lines.length) {
                    const stateLine = lines[i].trim();
                    if (stateLine === 'handlers' || stateLine.startsWith('on ')) {
                        i--;
                        break;
                    }
                    const match = stateLine.match(/^(\w+)\s+is\s+(.+)$/);
                    if (match) {
                        const key = match[1];
                        let value = match[2].trim();
                        value = value.replace(/^["']|["']$/g, '');
                        stateData[key] = value;
                    }
                    i++;
                }
            } else if (trimmed.startsWith('on ')) {
                const event = trimmed.substring(3).trim();
                const body = [];
                const baseIndent = this.getIndent(line);
                i++;

                while (i < lines.length) {
                    const handlerLine = lines[i];
                    const indent = this.getIndent(handlerLine);
                    if (indent <= baseIndent && handlerLine.trim()) {
                        i--;
                        break;
                    }
                    if (handlerLine.trim()) {
                        body.push(handlerLine);
                    }
                    i++;
                }

                handlers.push({ event, body });
            }
            i++;
        }

        return { name: actorName, role, stateData, handlers };
    }

    getIndent(line) {
        let indent = 0;
        while (line[indent] === ' ') indent++;
        return indent;
    }

    spawnActor(actorDef) {
        const actor = new L3Actor(actorDef.name, actorDef.role, actorDef.stateData, actorDef.handlers);
        const id = this.nextActorId++;
        this.actors.set(id, actor);
        this.messageQueues.set(id, []);
        this.log(`🎭 Spawned actor: ${actor.name} (id: ${id}, role: "${actor.role}")`);
        return id;
    }

    sendMessage(actorId, event, data = null) {
        this.log(`📨 Sent ${event} to actor ${actorId}`);
        this.messageQueues.get(actorId).push({ event, data });
    }

    tick() {
        for (const [actorId, queue] of this.messageQueues.entries()) {
            if (queue.length === 0) continue;

            const message = queue.shift();
            const actor = this.actors.get(actorId);

            this.log(`🎬 Actor ${actor.name} handling: ${message.event}`);

            const handler = actor.handlers.find(h => h.event === message.event);
            if (handler) {
                const ctx = new L3ExecutionContext(actor.state, message.data, this, actorId);
                this.executeBlock(handler.body, ctx);
            }
        }
    }

    executeBlock(lines, ctx, startIdx = 0) {
        let i = startIdx;

        while (i < lines.length) {
            const line = lines[i];
            const trimmed = line.trim();
            const indent = this.getIndent(line);

            if (trimmed.startsWith('if ')) {
                const condition = trimmed.substring(3).trim();
                const baseIndent = indent;
                const ifBody = [];
                i++;

                while (i < lines.length) {
                    const blockLine = lines[i];
                    const blockIndent = this.getIndent(blockLine);
                    if (blockIndent <= baseIndent && blockLine.trim()) {
                        i--;
                        break;
                    }
                    if (blockLine.trim()) {
                        ifBody.push(blockLine);
                    }
                    i++;
                }

                if (this.evaluateCondition(condition, ctx)) {
                    this.executeBlock(ifBody, ctx);
                }
            } else if (trimmed.startsWith('while ')) {
                const condition = trimmed.substring(6).trim();
                const baseIndent = indent;
                const whileBody = [];
                i++;

                while (i < lines.length) {
                    const blockLine = lines[i];
                    const blockIndent = this.getIndent(blockLine);
                    if (blockIndent <= baseIndent && blockLine.trim()) {
                        i--;
                        break;
                    }
                    if (blockLine.trim()) {
                        whileBody.push(blockLine);
                    }
                    i++;
                }

                let iterations = 0;
                const maxIterations = 10000;
                while (this.evaluateCondition(condition, ctx) && iterations < maxIterations) {
                    this.executeBlock(whileBody, ctx);
                    iterations++;
                }
            } else if (trimmed.startsWith('for ')) {
                const match = trimmed.match(/for\s+(\w+)\s+in\s+(\d+|\w+)\s+to\s+(\d+|\w+)/);
                if (match) {
                    const varName = match[1];
                    const start = parseInt(this.evaluateExpression(match[2], ctx));
                    const end = parseInt(this.evaluateExpression(match[3], ctx));
                    const baseIndent = indent;
                    const forBody = [];
                    i++;

                    while (i < lines.length) {
                        const blockLine = lines[i];
                        const blockIndent = this.getIndent(blockLine);
                        if (blockIndent <= baseIndent && blockLine.trim()) {
                            i--;
                            break;
                        }
                        if (blockLine.trim()) {
                            forBody.push(blockLine);
                        }
                        i++;
                    }

                    for (let val = start; val <= end; val++) {
                        ctx.setLocal(varName, val);
                        this.executeBlock(forBody, ctx);
                    }
                }
            } else if (trimmed) {
                this.executeLine(trimmed, ctx);
            }

            i++;
        }
    }

    executeLine(trimmed, ctx) {
        // send <Actor> <event>
        if (trimmed.startsWith('send ')) {
            const rest = trimmed.substring(5).trim();
            const parts = rest.split(/\s+/);
            const actorName = parts[0];
            const event = parts[1];

            for (const [id, actor] of this.actors.entries()) {
                if (actor.name === actorName) {
                    this.sendMessage(id, event, ctx.data);
                    this.log(`   → Sending ${event} to ${actorName}`);
                    return;
                }
            }
        }
        // let <var> = <value>
        else if (trimmed.startsWith('let ')) {
            const match = trimmed.match(/let\s+(\w+)\s*=\s*(.+)/);
            if (match) {
                const varName = match[1];
                const value = this.evaluateExpression(match[2], ctx);
                ctx.setLocal(varName, value);
                this.log(`   📦 Let ${varName} = ${value}`);
            }
        }
        // <var> = <value> or state.<key> = <value>
        else if (trimmed.includes('=')) {
            const match = trimmed.match(/^(.+?)\s*=\s*(.+)$/);
            if (match) {
                const target = match[1].trim();
                const value = this.evaluateExpression(match[2], ctx);

                if (target.startsWith('state.')) {
                    const key = target.substring(6);
                    ctx.state.set(key, value);
                    this.log(`   ✏️  Set state.${key} = ${value}`);
                } else {
                    ctx.setLocal(target, value);
                    this.log(`   📦 Set ${target} = ${value}`);
                }
            }
        }
        // log <message>
        else if (trimmed.startsWith('log ')) {
            const message = trimmed.substring(4).trim().replace(/^["']|["']$/g, '');
            this.log(`   📝 ${message}`);
        }
    }

    evaluateExpression(expr, ctx) {
        let trimmed = expr.trim();

        // String literal
        if (trimmed[0] === '"' || trimmed[0] === "'") {
            return trimmed.substring(1, trimmed.length - 1);
        }

        // Number literal
        if (/^-?\d+(\.\d+)?$/.test(trimmed)) {
            return trimmed;
        }

        // Boolean literal
        if (trimmed === 'true' || trimmed === 'false') {
            return trimmed;
        }

        // Arithmetic expression (check for operators FIRST)
        if (/[+\-*/()]/.test(trimmed)) {
            // Replace state references
            let replaced = trimmed.replace(/state\.(\w+)/g, (match, key) => {
                return ctx.state.get(key);
            });

            // Replace local variables
            for (const [key, value] of ctx.locals.entries()) {
                const regex = new RegExp(`\\b${key}\\b`, 'g');
                replaced = replaced.replace(regex, value);
            }

            try {
                return String(eval(replaced));
            } catch (e) {
                return '0';
            }
        }

        // state.key
        if (trimmed.startsWith('state.')) {
            const key = trimmed.substring(6);
            return ctx.state.get(key);
        }

        // Local variable
        const localVal = ctx.getLocal(trimmed);
        if (localVal !== undefined) {
            return localVal;
        }

        return expr;
    }

    evaluateCondition(condition, ctx) {
        const operators = ['<=', '>=', '==', '!=', '<', '>'];

        for (const op of operators) {
            const pos = condition.indexOf(op);
            if (pos !== -1) {
                const left = this.evaluateExpression(condition.substring(0, pos).trim(), ctx);
                const right = this.evaluateExpression(condition.substring(pos + op.length).trim(), ctx);

                const leftNum = parseFloat(left);
                const rightNum = parseFloat(right);

                switch (op) {
                    case '<': return leftNum < rightNum;
                    case '>': return leftNum > rightNum;
                    case '<=': return leftNum <= rightNum;
                    case '>=': return leftNum >= rightNum;
                    case '==': return leftNum === rightNum || left === right;
                    case '!=': return leftNum !== rightNum && left !== right;
                }
            }
        }

        return false;
    }

    getLogBuffer() {
        return this.logBuffer;
    }

    clearLogBuffer() {
        this.logBuffer = [];
    }
}

// Export for both Node.js and browser
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { L3ActorRuntime };
}
