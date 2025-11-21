"use client"

import { useState, useEffect } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Label } from "@/components/ui/label"
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select"
import { Input } from "@/components/ui/input"
import { Play, Square } from "lucide-react"

interface ConfigurationPanelProps {
  onSolve: (config: {
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
  }) => void
  onPreview: (benchmark: string) => void
  isRunning: boolean
  statusLog: string[]
}

export function ConfigurationPanel({ onSolve, onPreview, isRunning, statusLog }: ConfigurationPanelProps) {
  const [problem, setProblem] = useState("berlin52.tsp")
  const [method, setMethod] = useState("traditional")
  const [solverType, setSolverType] = useState("single-thread")
  const [rho, setRho] = useState(0.5)
  const [alpha, setAlpha] = useState(1.0)
  const [beta, setBeta] = useState(2.0)
  const [iterations, setIterations] = useState(100)
  const [useDefaultAnts, setUseDefaultAnts] = useState(true)
  const [numAnts, setNumAnts] = useState(20)
  const [useConvergence, setUseConvergence] = useState(false)
  const [convergenceIterations, setConvergenceIterations] = useState(200)

  // Preview cities when problem changes
  const handleProblemChange = (newProblem: string) => {
    setProblem(newProblem)
    onPreview(newProblem)
  }

  // Load initial preview on mount
  useEffect(() => {
    onPreview(problem)
  }, []) // eslint-disable-line react-hooks/exhaustive-deps

  const handleSolve = () => {
    onSolve({
      problem,
      method,
      solverType,
      rho,
      alpha,
      beta,
      iterations,
      numAnts: useDefaultAnts ? null : numAnts,
      useConvergence,
      convergenceIterations
    })
  }

  return (
    <Card className="flex flex-col h-full overflow-hidden">
      <CardHeader className="py-3">
        <CardTitle className="text-lg">Configuration</CardTitle>
      </CardHeader>
      <CardContent className="flex-1 space-y-2 overflow-y-auto pt-0">
        {/* Dropdowns in a grid */}
        <div className="grid grid-cols-3 gap-2">
          <div className="space-y-1">
            <Label htmlFor="problem" className="text-xs">Problem</Label>
            <Select value={problem} onValueChange={handleProblemChange}>
              <SelectTrigger id="problem" className="h-8 text-xs">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="berlin52.tsp">Berlin52</SelectItem>
                <SelectItem value="eil76.tsp">Eil76</SelectItem>
                <SelectItem value="kroA100.tsp">KroA100</SelectItem>
                <SelectItem value="ch150.tsp">Ch150</SelectItem>
                <SelectItem value="tsp225.tsp">TSP225</SelectItem>
                <SelectItem value="a280.tsp">A280</SelectItem>
              </SelectContent>
            </Select>
          </div>

          <div className="space-y-1">
            <Label htmlFor="method" className="text-xs">Method</Label>
            <Select value={method} onValueChange={setMethod}>
              <SelectTrigger id="method" className="h-8 text-xs">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="traditional">Traditional</SelectItem>
                <SelectItem value="elitist">Elitist</SelectItem>
                <SelectItem value="rank-based">Rank-Based</SelectItem>
                <SelectItem value="max-min">Max-Min</SelectItem>
                <SelectItem value="acs">ACS</SelectItem>
              </SelectContent>
            </Select>
          </div>

          <div className="space-y-1">
            <Label htmlFor="solver" className="text-xs">Solver</Label>
            <Select value={solverType} onValueChange={setSolverType}>
              <SelectTrigger id="solver" className="h-8 text-xs">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="single-thread">Single</SelectItem>
                <SelectItem value="multi-thread">Multi</SelectItem>
                <SelectItem value="gpu">GPU</SelectItem>
              </SelectContent>
            </Select>
          </div>
        </div>

        {/* Parameters in a grid */}
        <div className="grid grid-cols-3 gap-2">
          <div className="space-y-1">
            <Label htmlFor="rho" className="text-xs">ρ (evap)</Label>
            <Input
              id="rho"
              type="number"
              step="0.1"
              min="0"
              max="1"
              value={rho}
              onChange={(e) => setRho(Number.parseFloat(e.target.value))}
              className="h-8 text-xs"
            />
          </div>

          <div className="space-y-1">
            <Label htmlFor="alpha" className="text-xs">α (pher)</Label>
            <Input
              id="alpha"
              type="number"
              step="0.1"
              min="0"
              max="5"
              value={alpha}
              onChange={(e) => setAlpha(Number.parseFloat(e.target.value))}
              className="h-8 text-xs"
            />
          </div>

          <div className="space-y-1">
            <Label htmlFor="beta" className="text-xs">β (heur)</Label>
            <Input
              id="beta"
              type="number"
              step="0.1"
              min="0"
              max="5"
              value={beta}
              onChange={(e) => setBeta(Number.parseFloat(e.target.value))}
              className="h-8 text-xs"
            />
          </div>
        </div>

        {/* Iterations and Ants */}
        <div className="grid grid-cols-2 gap-2">
          <div className="space-y-1">
            <Label htmlFor="iterations" className="text-xs">Max Iterations</Label>
            <Input
              id="iterations"
              type="number"
              step="10"
              min="10"
              max="10000"
              value={iterations}
              onChange={(e) => setIterations(Number.parseInt(e.target.value))}
              className="h-8 text-xs"
              disabled={useConvergence}
            />
          </div>

          <div className="space-y-1">
            <Label htmlFor="numAnts" className="text-xs">Ants {useDefaultAnts && "(Auto)"}</Label>
            <div className="flex gap-1">
              <Input
                id="numAnts"
                type="number"
                step="1"
                min="1"
                max="1000"
                value={numAnts}
                onChange={(e) => setNumAnts(Number.parseInt(e.target.value))}
                className="h-8 text-xs"
                disabled={useDefaultAnts}
              />
              <Button
                type="button"
                variant={useDefaultAnts ? "default" : "outline"}
                size="sm"
                onClick={() => setUseDefaultAnts(!useDefaultAnts)}
                className="h-8 text-xs px-2"
              >
                Auto
              </Button>
            </div>
          </div>
        </div>

        {/* Convergence Criterion */}
        <div className="space-y-1">
          <div className="flex items-center gap-2">
            <input
              type="checkbox"
              id="useConvergence"
              checked={useConvergence}
              onChange={(e) => setUseConvergence(e.target.checked)}
              className="h-4 w-4"
            />
            <Label htmlFor="useConvergence" className="text-xs cursor-pointer">
              Stop when no improvement for
            </Label>
            <Input
              id="convergenceIterations"
              type="number"
              step="10"
              min="10"
              max="1000"
              value={convergenceIterations}
              onChange={(e) => setConvergenceIterations(Number.parseInt(e.target.value))}
              className="h-8 text-xs w-20"
              disabled={!useConvergence}
            />
            <Label htmlFor="convergenceIterations" className="text-xs">
              iterations
            </Label>
          </div>
        </div>

        {/* Solve Button */}
        <Button onClick={handleSolve} disabled={isRunning} className="w-full" size="sm">
          {isRunning ? (
            <>
              <Square className="mr-2 h-3 w-3" />
              Running...
            </>
          ) : (
            <>
              <Play className="mr-2 h-3 w-3" />
              Solve
            </>
          )}
        </Button>

        {/* Console - takes remaining space */}
        <div className="flex-1 min-h-0 space-y-1">
          <Label className="text-xs">Status Console</Label>
          <div className="bg-secondary rounded-md p-2 h-full min-h-[60px] overflow-y-auto font-mono text-xs space-y-0.5">
            {statusLog.map((log, i) => (
              <div key={i} className="text-muted-foreground">
                <span className="text-accent">{">"}</span> {log}
              </div>
            ))}
          </div>
        </div>
      </CardContent>
    </Card>
  )
}
