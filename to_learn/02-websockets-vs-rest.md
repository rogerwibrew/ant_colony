# WebSockets vs REST (axios)

## The Problem We're Solving

Our ACO solver runs for many iterations (e.g., 100). We want to show progress updates in real-time. How should the frontend know when each iteration completes?

## Option 1: REST with axios (Request-Response)

**How REST works:**
```
Browser                              Server
   │                                    │
   │  GET /api/solve?problem=berlin52   │
   │ ──────────────────────────────────>│
   │                                    │  (server works for 10 seconds)
   │                                    │
   │  { "result": 7630 }                │
   │ <──────────────────────────────────│
```

**The problem:** The browser sends ONE request and waits for ONE response. You can't get progress updates during the 10 seconds of computation.

**Workaround: Polling**
```
Browser                              Server
   │                                    │
   │  POST /api/start-solve             │
   │ ──────────────────────────────────>│
   │  { "jobId": "abc123" }             │
   │ <──────────────────────────────────│
   │                                    │
   │  GET /api/status/abc123            │  (every 500ms)
   │ ──────────────────────────────────>│
   │  { "progress": 10% }               │
   │ <──────────────────────────────────│
   │                                    │
   │  GET /api/status/abc123            │  (again)
   │ ──────────────────────────────────>│
   │  { "progress": 20% }               │
   │ <──────────────────────────────────│
   │                                    │
   │  ... (repeat 100 times) ...        │
```

**Problems with polling:**
- Wasteful: Many requests even when nothing changed
- Laggy: Updates only as fast as you poll
- Complex: Need to track job IDs, handle timeouts

## Option 2: WebSockets (What We Use)

**How WebSockets work:**
```
Browser                              Server
   │                                    │
   │  CONNECT (upgrade to WebSocket)    │
   │ <==================================>│  (persistent connection)
   │                                    │
   │  emit("solve", {...})              │
   │ ──────────────────────────────────>│
   │                                    │
   │  emit("progress", {iter: 10})      │  (server pushes when ready)
   │ <──────────────────────────────────│
   │                                    │
   │  emit("progress", {iter: 20})      │  (immediately, no polling)
   │ <──────────────────────────────────│
   │                                    │
   │  emit("complete", {...})           │
   │ <──────────────────────────────────│
```

**Advantages:**
- Real-time: Server pushes updates instantly
- Efficient: No repeated requests
- Simple: Just listen for events

## Comparison Table

| Feature | REST (axios) | WebSocket |
|---------|--------------|-----------|
| Connection | New connection per request | Persistent connection |
| Direction | Client → Server only | Bidirectional |
| Real-time updates | Need polling | Native support |
| Best for | CRUD operations, simple APIs | Real-time apps, live updates |
| Complexity | Simpler for basic use | More setup, but cleaner for real-time |

## When to Use What

**Use REST/axios when:**
- Fetching data once (load a page, get user info)
- Simple CRUD: Create, Read, Update, Delete
- Stateless operations
- Example: `GET /api/users/123`

**Use WebSockets when:**
- Real-time updates needed (chat, live scores, progress)
- Server needs to push data to client
- Long-running operations with progress
- Example: Our ACO solver!

## Code Comparison

### axios (REST) - Hypothetical

```typescript
// Would need polling for progress
const response = await axios.post('/api/solve', { benchmark: 'berlin52.tsp' })
const jobId = response.data.jobId

// Poll every 500ms (wasteful!)
const interval = setInterval(async () => {
  const status = await axios.get(`/api/status/${jobId}`)
  if (status.data.complete) {
    clearInterval(interval)
    setResult(status.data.result)
  } else {
    setProgress(status.data.progress)
  }
}, 500)
```

### socket.io (WebSocket) - What We Use

```typescript
// Clean event-based approach
socket.emit('solve', { benchmark: 'berlin52.tsp' })

socket.on('progress', (data) => {
  setProgress(data.progress)  // Called automatically when server sends
})

socket.on('complete', (data) => {
  setResult(data.result)  // Called automatically when done
})
```

## The socket.io Library

We use `socket.io` (server) and `socket.io-client` (browser) because:
- Handles connection management automatically
- Falls back to polling if WebSockets aren't available
- Nice event-based API (`emit`, `on`)
- Works with Flask via `Flask-SocketIO`

**Raw WebSocket API is more complex:**
```javascript
// Raw WebSocket (we don't use this directly)
const ws = new WebSocket('ws://localhost:5000')
ws.onmessage = (event) => {
  const data = JSON.parse(event.data)
  // Have to manually route to handlers
}
```

**socket.io makes it cleaner:**
```javascript
// socket.io (what we use)
const socket = io('http://localhost:5000')
socket.on('progress', (data) => { /* specific handler */ })
socket.on('complete', (data) => { /* specific handler */ })
```
