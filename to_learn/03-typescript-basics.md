# TypeScript Basics

## What is TypeScript?

TypeScript = JavaScript + Types

It's JavaScript with added type annotations that help catch errors before you run the code.

```typescript
// JavaScript (no types)
function add(a, b) {
  return a + b
}
add("hello", 5)  // Returns "hello5" - probably not what you wanted!

// TypeScript (with types)
function add(a: number, b: number): number {
  return a + b
}
add("hello", 5)  // ERROR! TypeScript catches this before you run it
```

## Basic Type Annotations

```typescript
// Variables
let name: string = "Roger"
let age: number = 30
let isActive: boolean = true
let items: string[] = ["apple", "banana"]  // Array of strings
let coords: number[] = [10, 20]  // Array of numbers

// Functions
function greet(name: string): string {
  return `Hello, ${name}`
}

// Optional parameters (? means optional)
function greet(name: string, title?: string): string {
  if (title) {
    return `Hello, ${title} ${name}`
  }
  return `Hello, ${name}`
}
```

## Interfaces: Defining Object Shapes

An interface describes what properties an object must have:

```typescript
// Define the shape
interface City {
  id: number
  x: number
  y: number
  name: string
}

// Use it
const berlin: City = {
  id: 1,
  x: 565.0,
  y: 575.0,
  name: "Berlin"
}

// TypeScript will error if you miss a property or use wrong type
const invalid: City = {
  id: "1",  // ERROR: should be number
  x: 565.0
  // ERROR: missing y and name
}
```

## Reading Our Frontend Code

### Example 1: `useSocket.ts` Types

```typescript
// Define what data looks like
export interface SolveParams {
  benchmark: string
  num_ants?: number      // ? means optional
  iterations?: number
  alpha?: number
  beta?: number
  rho?: number
}

// Define what the hook returns
export interface UseSocketReturn {
  isConnected: boolean
  cities: CityData[]      // Array of CityData objects
  bestDistance: number
  solve: (params: SolveParams) => void  // A function that takes SolveParams
}
```

### Example 2: React Component Props

```typescript
// Define what props the component accepts
interface ConfigurationPanelProps {
  onSolve: (config: {
    problem: string
    alpha: number
    beta: number
    rho: number
  }) => void              // A callback function
  isRunning: boolean
  statusLog: string[]     // Array of strings
}

// Use the props in the component
export function ConfigurationPanel({ onSolve, isRunning, statusLog }: ConfigurationPanelProps) {
  // TypeScript knows:
  // - onSolve is a function
  // - isRunning is a boolean
  // - statusLog is string[]
}
```

### Example 3: useState with Types

```typescript
// TypeScript infers type from initial value
const [count, setCount] = useState(0)  // count is number

// Explicit type for complex objects
const [cities, setCities] = useState<CityData[]>([])  // Array of CityData

// Generic syntax: useState<TYPE>(initialValue)
const [error, setError] = useState<string | null>(null)  // string OR null
```

## Common Patterns in Our Code

### Destructuring with Types

```typescript
// Object destructuring in function parameters
function handleSolve({ benchmark, alpha, beta }: SolveParams) {
  // benchmark, alpha, beta are now variables
}

// Same thing in React components
function MyComponent({ onSolve, isRunning }: Props) {
  // ...
}
```

### Arrow Functions with Types

```typescript
// Regular function
function add(a: number, b: number): number {
  return a + b
}

// Arrow function (same thing)
const add = (a: number, b: number): number => {
  return a + b
}

// Short form for simple returns
const add = (a: number, b: number): number => a + b
```

### Callbacks with Types

```typescript
// Define a callback type
type OnSolveCallback = (config: { problem: string; alpha: number }) => void

// Use it
interface Props {
  onSolve: OnSolveCallback
}

// Or inline
interface Props {
  onSolve: (config: { problem: string; alpha: number }) => void
}
```

## File Extensions

| Extension | Meaning |
|-----------|---------|
| `.ts` | TypeScript file (no JSX) |
| `.tsx` | TypeScript file with JSX (React components) |
| `.js` | JavaScript file |
| `.jsx` | JavaScript file with JSX |

## Why Use TypeScript?

1. **Catch errors early** - Before running the code
2. **Better IDE support** - Autocomplete, hover docs
3. **Self-documenting** - Types show what functions expect
4. **Refactoring safety** - Change a type, see all places that break

## The tsconfig.json File

This configures the TypeScript compiler:

```json
{
  "compilerOptions": {
    "target": "ES2017",        // What JS version to output
    "strict": true,            // Enable all strict checks
    "jsx": "preserve",         // How to handle JSX
    "moduleResolution": "node" // How to find imports
  }
}
```

## Quick Reference: Our Types

```typescript
// City data from backend
interface CityData {
  id: number
  x: number
  y: number
  name: string
}

// Parameters for solving
interface SolveParams {
  benchmark: string
  alpha?: number
  beta?: number
  rho?: number
}

// Progress update from backend
interface ProgressData {
  iteration: number
  bestDistance: number
  bestTour: number[]
  progress: number
}
```
