"use client"

import { useState } from "react"
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
  }) => void
  isRunning: boolean
  statusLog: string[]
}

export function ConfigurationPanel({ onSolve, isRunning, statusLog }: ConfigurationPanelProps) {
  const [problem, setProblem] = useState("berlin52.tsp")
  const [method, setMethod] = useState("traditional")
  const [solverType, setSolverType] = useState("single-thread")
  const [rho, setRho] = useState(0.5)
  const [alpha, setAlpha] = useState(1.0)
  const [beta, setBeta] = useState(2.0)

  const handleSolve = () => {
    onSolve({ problem, method, solverType, rho, alpha, beta })
  }

  return (
    <Card className="flex flex-col">
      <CardHeader>
        <CardTitle className="text-xl">Configuration</CardTitle>
      </CardHeader>
      <CardContent className="flex-1 space-y-4 overflow-y-auto">
        {/* TSP Problem */}
        <div className="space-y-2">
          <Label htmlFor="problem">TSP Problem</Label>
          <Select value={problem} onValueChange={setProblem}>
            <SelectTrigger id="problem">
              <SelectValue />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="berlin52.tsp">Berlin52 (52 cities)</SelectItem>
              <SelectItem value="eil76.tsp">Eil76 (76 cities)</SelectItem>
              <SelectItem value="kroA100.tsp">KroA100 (100 cities)</SelectItem>
              <SelectItem value="ch150.tsp">Ch150 (150 cities)</SelectItem>
              <SelectItem value="tsp225.tsp">TSP225 (225 cities)</SelectItem>
              <SelectItem value="a280.tsp">A280 (280 cities)</SelectItem>
            </SelectContent>
          </Select>
        </div>

        {/* Solution Method */}
        <div className="space-y-2">
          <Label htmlFor="method">Solution Method</Label>
          <Select value={method} onValueChange={setMethod}>
            <SelectTrigger id="method">
              <SelectValue />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="traditional">Traditional ACO</SelectItem>
              <SelectItem value="elitist">Elitist Ant System</SelectItem>
              <SelectItem value="rank-based">Rank-Based AS</SelectItem>
              <SelectItem value="max-min">Max-Min AS</SelectItem>
              <SelectItem value="acs">Ant Colony System</SelectItem>
            </SelectContent>
          </Select>
        </div>

        {/* Solver Type */}
        <div className="space-y-2">
          <Label htmlFor="solver">Solver Type</Label>
          <Select value={solverType} onValueChange={setSolverType}>
            <SelectTrigger id="solver">
              <SelectValue />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="single-thread">Single Thread</SelectItem>
              <SelectItem value="multi-thread">Multi Thread</SelectItem>
              <SelectItem value="gpu">GPU Accelerated</SelectItem>
            </SelectContent>
          </Select>
        </div>

        {/* Simulation Constants */}
        <div className="space-y-4 pt-2">
          <h3 className="text-sm font-semibold text-foreground">Simulation Constants</h3>

          <div className="space-y-2">
            <Label htmlFor="rho" className="flex items-center justify-between">
              <span>Evaporation Rate (ρ)</span>
              <span className="text-xs text-muted-foreground font-mono">{rho}</span>
            </Label>
            <Input
              id="rho"
              type="number"
              step="0.1"
              min="0"
              max="1"
              value={rho}
              onChange={(e) => setRho(Number.parseFloat(e.target.value))}
            />
          </div>

          <div className="space-y-2">
            <Label htmlFor="alpha" className="flex items-center justify-between">
              <span>Pheromone Importance (α)</span>
              <span className="text-xs text-muted-foreground font-mono">{alpha}</span>
            </Label>
            <Input
              id="alpha"
              type="number"
              step="0.1"
              min="0"
              max="5"
              value={alpha}
              onChange={(e) => setAlpha(Number.parseFloat(e.target.value))}
            />
          </div>

          <div className="space-y-2">
            <Label htmlFor="beta" className="flex items-center justify-between">
              <span>Heuristic Importance (β)</span>
              <span className="text-xs text-muted-foreground font-mono">{beta}</span>
            </Label>
            <Input
              id="beta"
              type="number"
              step="0.1"
              min="0"
              max="5"
              value={beta}
              onChange={(e) => setBeta(Number.parseFloat(e.target.value))}
            />
          </div>
        </div>

        {/* Console */}
        <div className="space-y-2 pt-2">
          <Label>Status Console</Label>
          <div className="bg-secondary rounded-md p-3 h-32 overflow-y-auto font-mono text-xs space-y-1">
            {statusLog.map((log, i) => (
              <div key={i} className="text-muted-foreground">
                <span className="text-accent">{">"}</span> {log}
              </div>
            ))}
          </div>
        </div>

        {/* Solve Button */}
        <Button onClick={handleSolve} disabled={isRunning} className="w-full" size="lg">
          {isRunning ? (
            <>
              <Square className="mr-2 h-4 w-4" />
              Running...
            </>
          ) : (
            <>
              <Play className="mr-2 h-4 w-4" />
              Solve
            </>
          )}
        </Button>
      </CardContent>
    </Card>
  )
}
