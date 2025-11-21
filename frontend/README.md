# ACO TSP Solver - Next.js Frontend

Interactive web interface for Ant Colony Optimization TSP solver with real-time visualization and advanced controls.

## Features

### Problem Preview
- **Automatic City Loading**: Select a TSP problem from dropdown to instantly preview city positions
- **Visual Inspection**: View problem layout before running optimization
- **Seamless Updates**: Cities displayed in real-time as problem selection changes

### Solver Controls
- **Iterations**: Configure max iterations (10-10,000)
- **Convergence Mode**: Auto-stop when no improvement for N iterations (default: 200)
- **Number of Ants**:
  - Auto mode: Calculates 1-2 ants per city based on problem size
  - Manual mode: Specify custom ant count (1-1000)
- **ACO Parameters**: Adjustable α (pheromone), β (heuristic), ρ (evaporation)

### Real-Time Visualizations
- **City Positions** (top-right): Dots showing city locations with uniform viewport-based sizing
- **Current Path** (bottom-left): Live tour updates during optimization
- **Convergence Graph** (bottom-right): Monotonically decreasing plot of best solution over time
- **Status Console**: Real-time solver status, iteration count, and progress messages

### Performance
- WebSocket-based updates every 10 iterations
- Smooth real-time visualization without blocking UI
- Elapsed time and progress tracking

## Technology Stack

- **Next.js 16** - App Router
- **TypeScript** - Type-safe development
- **Tailwind CSS** - Utility-first styling
- **shadcn/ui** - High-quality React components
- **Recharts** - Convergence visualization charts
- **socket.io-client** - Real-time WebSocket communication

## Getting Started

### Prerequisites

Ensure the backend is running first:

```bash
cd ../backend
pip install -r requirements.txt
python app.py  # Starts on http://localhost:5000
```

### Installation

```bash
cd frontend
npm install
```

### Development

```bash
npm run dev
```

Open [http://localhost:3000](http://localhost:3000) with your browser.

### Build for Production

```bash
npm run build
npm start
```

## Project Structure

```
frontend/
├── src/
│   ├── app/
│   │   ├── page.tsx                    # Main page (imports dashboard)
│   │   └── layout.tsx                  # Root layout
│   ├── components/
│   │   ├── aco-tsp-dashboard.tsx       # Main dashboard layout
│   │   ├── configuration-panel.tsx     # Problem selection & controls
│   │   ├── city-visualization.tsx      # City positions & preview
│   │   ├── path-display.tsx            # Current iteration path
│   │   ├── performance-chart.tsx       # Convergence graph
│   │   └── ui/                         # shadcn/ui components
│   └── hooks/
│       └── useSocket.ts                # WebSocket hook for backend
├── public/                              # Static assets
└── package.json
```

## Components

### AcoTspDashboard

Main dashboard component that orchestrates the entire UI.

**Features:**
- 2x2 grid layout (configuration, city viz, path, chart)
- Manages WebSocket connection state
- Coordinates data flow between components

### ConfigurationPanel

Problem selection and solver controls.

**Features:**
- TSP problem dropdown (triggers preview on change)
- Iteration count input (disabled in convergence mode)
- Ant count control with Auto/Manual toggle
- ACO parameter sliders (α, β, ρ)
- Convergence criterion checkbox
- Solve button (disabled when running)
- Status console with real-time logs

### CityVisualization

Displays city positions with optional tour overlay.

**Features:**
- Automatic preview when problem changes
- Uniform viewport-based dot/line sizing (0.8% of viewport)
- Dynamic viewBox calculation with 10% padding
- Grid background for reference
- Real-time tour overlay during solving

### PathDisplay

Shows current iteration's best path.

**Features:**
- Live updates during optimization
- Same visualization style as CityVisualization
- Iteration number and path length display

### PerformanceChart

Convergence graph using Recharts.

**Features:**
- Monotonically decreasing line (running global best)
- X-axis: Iteration number
- Y-axis: Best distance found
- Responsive sizing
- Smooth animations

### useSocket Hook

Custom React hook for WebSocket communication.

**Events Handled:**
- `connect` / `disconnect` - Connection management
- `preview_loaded` - Cities loaded for preview
- `loaded` - Benchmark loaded for solving
- `progress` - Real-time solver updates (every 10 iter)
- `complete` - Optimization finished
- `error` - Error handling

**Exported Functions:**
- `solve(params)` - Start optimization
- `preview(benchmark)` - Load cities without solving
- `stop()` - Cancel running solver

## Configuration

### Backend URL

Edit `src/hooks/useSocket.ts`:

```typescript
const BACKEND_URL = process.env.NEXT_PUBLIC_BACKEND_URL || "http://localhost:5000"
```

Or set environment variable:

```bash
NEXT_PUBLIC_BACKEND_URL=http://your-backend:5000 npm run dev
```

## Development Notes

### Hot Reload

The development server automatically reloads when you edit:
- `src/**/*.tsx` - React components
- `src/**/*.ts` - TypeScript files
- `tailwind.config.ts` - Tailwind configuration

### Type Safety

All WebSocket events and data structures are typed:
- `SolveParams` - Solver configuration
- `CityData` - City coordinates
- `UseSocketReturn` - Hook return values

### State Management

- Uses React hooks (useState, useCallback, useMemo)
- WebSocket state managed in useSocket hook
- No global state library needed (simple app)

## Troubleshooting

### WebSocket connection fails

- **Check backend is running** on http://localhost:5000
- **Verify CORS settings** in backend's `config.py`
- **Check browser console** for connection errors

### Cities not displaying

- **Ensure backend has access** to `data/` directory
- **Check file exists**: `ls ../data/berlin52.tsp`
- **View browser console** for preview errors

### Slow visualization updates

- **Reduce callback interval** in backend (currently 10 iterations)
- **Use fewer iterations** for testing
- **Try smaller problems** (e.g., berlin52 instead of a280)

## Future Enhancements

- Additional benchmark problems in dropdown
- Path animation playback controls
- Side-by-side comparison mode (multiple solvers)
- Export results to CSV/JSON
- Parameter presets (Quick, Balanced, Thorough)
- Tour length comparison vs optimal
- Heat map of pheromone trails

## Learn More

- [Next.js Documentation](https://nextjs.org/docs)
- [Tailwind CSS](https://tailwindcss.com/docs)
- [shadcn/ui](https://ui.shadcn.com/)
- [Socket.IO Client](https://socket.io/docs/v4/client-api/)
- [Recharts](https://recharts.org/)
