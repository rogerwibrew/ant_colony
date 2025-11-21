"use client"

import { useEffect, useRef, useCallback, useState } from "react"
import { io, Socket } from "socket.io-client"

const BACKEND_URL = process.env.NEXT_PUBLIC_BACKEND_URL || "http://localhost:5000"

export interface SolveParams {
  benchmark: string
  num_ants?: number
  iterations?: number
  alpha?: number
  beta?: number
  rho?: number
  q?: number
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

    socket.on("progress", (data: { iteration: number; bestDistance: number; bestTour: number[]; progress: number }) => {
      setCurrentIteration(data.iteration)
      setBestDistance(data.bestDistance)
      setBestTour(data.bestTour)
      setConvergenceData((prev) => [...prev, { iteration: data.iteration, length: data.bestDistance }])
      addLog(`Iteration ${data.iteration}: Best = ${data.bestDistance.toFixed(2)} (${data.progress}%)`)
    })

    socket.on("complete", (data: { bestDistance: number; bestTour: number[]; totalIterations: number }) => {
      setIsRunning(false)
      setBestDistance(data.bestDistance)
      setBestTour(data.bestTour)
      addLog(`Completed: Best distance = ${data.bestDistance.toFixed(2)}`)
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
    addLog(`Params: α=${params.alpha}, β=${params.beta}, ρ=${params.rho}`)

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
      },
    }
    console.log("Emitting solve event:", payload)
    socketRef.current.emit("solve", payload)
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
    stop,
  }
}
