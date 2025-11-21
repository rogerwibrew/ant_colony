"use client"

import { ConfigurationPanel } from "@/components/configuration-panel"
import { CityVisualization } from "@/components/city-visualization"
import { PathDisplay } from "@/components/path-display"
import { PerformanceChart } from "@/components/performance-chart"
import { useSocket } from "@/hooks/useSocket"

export interface City {
  id: number
  x: number
  y: number
  name: string
}

export interface SimulationState {
  iteration: number
  bestPath: number[]
  bestPathLength: number
  cities: City[]
  isRunning: boolean
  statusLog: string[]
}

export function AcoTspDashboard() {
  const {
    isConnected,
    cities,
    currentIteration,
    bestDistance,
    bestTour,
    isRunning,
    error,
    convergenceData,
    statusLog,
    solve,
  } = useSocket()

  const handleSolve = async (config: {
    problem: string
    method: string
    solverType: string
    rho: number
    alpha: number
    beta: number
    iterations: number
    numAnts: number | null
    useConvergence: boolean
    convergenceIterations: number
  }) => {
    solve({
      benchmark: config.problem,
      alpha: config.alpha,
      beta: config.beta,
      rho: config.rho,
      iterations: config.iterations,
      num_ants: config.numAnts,
      use_convergence: config.useConvergence,
      convergence_iterations: config.convergenceIterations,
    })
  }

  // Build status log with connection status
  const fullStatusLog = [
    ...(error ? [`Error: ${error}`] : []),
    ...statusLog,
  ]

  // Map cities to expected format
  const mappedCities: City[] = cities.map((c) => ({
    id: c.id,
    x: c.x,
    y: c.y,
    name: c.name || `City ${c.id}`,
  }))

  return (
    <div className="min-h-screen bg-background p-4">
      <header className="mb-6">
        <h1 className="text-3xl font-bold text-foreground mb-2">ACO TSP Solver</h1>
        <p className="text-muted-foreground">
          Ant Colony Optimization for Traveling Salesman Problem
          <span className={`ml-2 text-sm ${isConnected ? "text-green-500" : "text-red-500"}`}>
            ({isConnected ? "Connected" : "Disconnected"})
          </span>
        </p>
      </header>

      <div className="grid grid-cols-1 lg:grid-cols-2 lg:grid-rows-2 gap-4 h-[calc(100vh-140px)]">
        {/* Top Left - Configuration */}
        <ConfigurationPanel
          onSolve={handleSolve}
          isRunning={isRunning}
          statusLog={fullStatusLog}
        />

        {/* Top Right - City Visualization */}
        <CityVisualization cities={mappedCities} bestPath={bestTour} />

        {/* Bottom Left - Path Display */}
        <PathDisplay
          cities={mappedCities}
          bestPath={bestTour}
          bestPathLength={bestDistance}
          iteration={currentIteration}
        />

        {/* Bottom Right - Performance Chart */}
        <PerformanceChart pathHistory={convergenceData} />
      </div>
    </div>
  )
}
