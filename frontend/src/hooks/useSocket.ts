"use client"

import { useEffect, useRef, useCallback, useState } from "react"
import { io, Socket } from "socket.io-client"

const BACKEND_URL = process.env.NEXT_PUBLIC_BACKEND_URL || "http://localhost:5000"

export interface SolveParams {
  benchmark: string
  num_ants?: number | null
  iterations?: number
  alpha?: number
  beta?: number
  rho?: number
  q?: number
  use_convergence?: boolean
  convergence_iterations?: number
  use_parallel?: boolean       // Enable multi-threading
  num_threads?: number          // Number of threads (0=auto, 1=serial, 2+=specific)
  use_local_search?: boolean    // Enable 2-opt/3-opt local search
  use_3opt?: boolean            // Use both 2-opt and 3-opt
  local_search_mode?: string    // When to apply local search (best, all, none)
}

export interface CityData {
  id: number
  x: number
  y: number
  name: string
}

export interface IterationProgress {
  iteration: number
  best_distance: number
  iteration_best_distance: number
  best_tour: number[]
  improvement: boolean
}

export interface SolveCompleted {
  best_distance: number
  best_tour: number[]
  total_iterations: number
  convergence_data: number[]
}

export interface UseSocketReturn {
  isConnected: boolean
  cities: CityData[]
  currentIteration: number
  bestDistance: number
  bestTour: number[]
  isRunning: boolean
  error: string | null
  convergenceData: { iteration: number; length: number }[]
  statusLog: string[]
  solve: (params: SolveParams) => void
  preview: (benchmark: string) => void
  stop: () => void
}

export function useSocket(): UseSocketReturn {
  const socketRef = useRef<Socket | null>(null)
  const [isConnected, setIsConnected] = useState(false)
  const [cities, setCities] = useState<CityData[]>([])
  const [currentIteration, setCurrentIteration] = useState(0)
  const [bestDistance, setBestDistance] = useState(0)
  const [bestTour, setBestTour] = useState<number[]>([])
  const [isRunning, setIsRunning] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const [convergenceData, setConvergenceData] = useState<{ iteration: number; length: number }[]>([])
  const [statusLog, setStatusLog] = useState<string[]>(["System initialized", "Connecting to backend..."])

  const addLog = useCallback((message: string) => {
    setStatusLog((prev) => [...prev.slice(-7), message])
  }, [])

  useEffect(() => {
    const socket = io(BACKEND_URL, {
      transports: ["websocket", "polling"],
      autoConnect: true,
    })

    socketRef.current = socket

    socket.on("connect", () => {
      setIsConnected(true)
      setError(null)
      addLog(`Connected to backend at ${BACKEND_URL}`)
    })

    socket.on("disconnect", () => {
      setIsConnected(false)
      addLog("Disconnected from backend")
    })

    socket.on("connect_error", (err) => {
      setError(`Connection error: ${err.message}`)
      addLog(`Connection error: ${err.message}`)
    })

    socket.on("loaded", (data: { benchmark: string; numCities: number; cities: number[][] }) => {
      // Transform [x, y] arrays to CityData objects
      const cityData: CityData[] = data.cities.map((coords, i) => ({
        id: i,
        x: coords[0],
        y: coords[1],
        name: `City ${i + 1}`,
      }))
      setCities(cityData)
      addLog(`Loaded ${data.benchmark}: ${data.numCities} cities`)
    })

    socket.on("preview_loaded", (data: { benchmark: string; numCities: number; cities: number[][] }) => {
      // Transform [x, y] arrays to CityData objects
      const cityData: CityData[] = data.cities.map((coords, i) => ({
        id: i,
        x: coords[0],
        y: coords[1],
        name: `City ${i + 1}`,
      }))
      setCities(cityData)
      // Clear previous tour when previewing new problem
      setBestTour([])
      setBestDistance(0)
      setCurrentIteration(0)
      setConvergenceData([])
      addLog(`Preview: ${data.benchmark} (${data.numCities} cities)`)
    })

    socket.on("progress", (data: { iteration: number; bestDistance: number; bestTour: number[]; progress: number; elapsedTime?: number }) => {
      setCurrentIteration(data.iteration)
      setBestDistance(data.bestDistance)
      setBestTour(data.bestTour)
      setConvergenceData((prev) => [...prev, { iteration: data.iteration, length: data.bestDistance }])

      // Show iteration status without repeating distance (we have the graph for that)
      const timeStr = data.elapsedTime ? ` [${data.elapsedTime.toFixed(1)}s]` : ''
      addLog(`Running iteration ${data.iteration}...${timeStr}`)
    })

    socket.on("complete", (data: { bestDistance: number; bestTour: number[]; totalIterations: number; elapsedTime?: number; benchmark?: string; optimalDistance?: number; optimalityGap?: number }) => {
      setIsRunning(false)
      setBestDistance(data.bestDistance)
      setBestTour(data.bestTour)

      // Build completion message with solution quality
      const timeStr = data.elapsedTime ? ` in ${data.elapsedTime.toFixed(2)}s` : ''
      addLog(`✓ Optimization complete${timeStr}`)
      addLog(`  Best distance: ${data.bestDistance.toFixed(2)}`)

      if (data.optimalDistance && data.optimalityGap !== undefined) {
        const gapStr = data.optimalityGap >= 0
          ? `+${data.optimalityGap.toFixed(2)}%`
          : `${data.optimalityGap.toFixed(2)}%`
        addLog(`  Optimal: ${data.optimalDistance} (${gapStr} above optimal)`)
      }

      addLog(`  Total iterations: ${data.totalIterations}`)
    })

    socket.on("error", (data: { message: string }) => {
      setError(data.message)
      setIsRunning(false)
      addLog(`Error: ${data.message}`)
    })

    return () => {
      socket.disconnect()
    }
  }, [addLog])

  const solve = useCallback((params: SolveParams) => {
    if (!socketRef.current?.connected) {
      setError("Not connected to backend")
      return
    }

    // Reset state
    setConvergenceData([])
    setCurrentIteration(0)
    setBestDistance(0)
    setBestTour([])
    setError(null)
    setIsRunning(true)

    addLog(`Starting solver on ${params.benchmark}`)
    if (params.use_convergence) {
      addLog(`Convergence mode: stop after ${params.convergence_iterations} iterations without improvement`)
    } else {
      addLog(`Fixed iterations: ${params.iterations || 100}`)
    }
    addLog(`Params: α=${params.alpha}, β=${params.beta}, ρ=${params.rho}`)

    // Log threading configuration
    if (params.use_parallel === false || params.num_threads === 1) {
      addLog(`Threading: Serial (single-threaded)`)
    } else if (params.num_threads && params.num_threads > 1) {
      addLog(`Threading: ${params.num_threads} threads`)
    } else {
      addLog(`Threading: Multi-threaded (auto-detect)`)
    }

    // Log local search configuration
    if (params.use_local_search) {
      const lsType = params.use_3opt ? "2-opt+3-opt" : "2-opt only"
      addLog(`Local Search: Enabled (${lsType}, mode=${params.local_search_mode})`)
    } else {
      addLog(`Local Search: Disabled`)
    }

    // Backend expects { benchmark, params: { alpha, beta, rho, ... } }
    const payload = {
      benchmark: params.benchmark,
      params: {
        alpha: params.alpha,
        beta: params.beta,
        rho: params.rho,
        numAnts: params.num_ants,
        iterations: params.iterations,
        Q: params.q,
        useConvergence: params.use_convergence,
        convergenceIterations: params.convergence_iterations,
        useParallel: params.use_parallel,
        numThreads: params.num_threads,
        useLocalSearch: params.use_local_search,
        use3Opt: params.use_3opt,
        localSearchMode: params.local_search_mode,
      },
    }
    console.log("Emitting solve event:", payload)
    socketRef.current.emit("solve", payload)
  }, [addLog])

  const preview = useCallback((benchmark: string) => {
    if (!socketRef.current?.connected) {
      setError("Not connected to backend")
      return
    }

    addLog(`Requesting preview for ${benchmark}`)
    socketRef.current.emit("preview", { benchmark })
  }, [addLog])

  const stop = useCallback(() => {
    if (socketRef.current?.connected) {
      socketRef.current.emit("stop")
      setIsRunning(false)
      addLog("Solver stopped")
    }
  }, [addLog])

  return {
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
    preview,
    stop,
  }
}
