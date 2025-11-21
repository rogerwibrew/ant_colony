"use client"

import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import type { City } from "@/components/aco-tsp-dashboard"
import { ArrowRight } from "lucide-react"

interface PathDisplayProps {
  cities: City[]
  bestPath: number[]
  bestPathLength: number
  iteration: number
}

export function PathDisplay({ cities, bestPath, bestPathLength, iteration }: PathDisplayProps) {
  return (
    <Card className="flex flex-col">
      <CardHeader>
        <CardTitle className="text-xl flex items-center justify-between">
          <span>Current Best Path</span>
          {bestPathLength > 0 && (
            <span className="text-sm font-mono text-muted-foreground">
              Length: <span className="text-primary font-bold">{bestPathLength.toFixed(2)}</span>
            </span>
          )}
        </CardTitle>
      </CardHeader>
      <CardContent className="flex-1 overflow-y-auto">
        {bestPath.length === 0 ? (
          <div className="flex items-center justify-center h-full text-muted-foreground">
            <p>No path calculated yet. Click Solve to begin.</p>
          </div>
        ) : (
          <div className="space-y-4">
            <div className="flex items-center gap-2 text-sm text-muted-foreground mb-4">
              <span className="font-mono">Iteration: {iteration}</span>
              <span>•</span>
              <span className="font-mono">{cities.length} cities</span>
            </div>

            <div className="flex flex-wrap gap-2">
              {bestPath.map((cityIndex, i) => {
                const city = cities[cityIndex]
                return (
                  <div key={i} className="flex items-center">
                    <div className="bg-secondary px-3 py-2 rounded-md">
                      <span className="font-mono text-sm font-semibold text-primary">
                        {city?.id !== undefined ? city.id + 1 : "?"}
                      </span>
                    </div>
                    {i < bestPath.length - 1 && <ArrowRight className="mx-1 h-4 w-4 text-muted-foreground" />}
                  </div>
                )
              })}
              <div className="flex items-center">
                <ArrowRight className="mx-1 h-4 w-4 text-muted-foreground" />
                <div className="bg-accent/20 px-3 py-2 rounded-md border border-accent">
                  <span className="font-mono text-sm font-semibold text-accent">
                    {bestPath[0] !== undefined && cities[bestPath[0]]?.id !== undefined
                      ? cities[bestPath[0]].id + 1
                      : "?"}
                  </span>
                </div>
              </div>
            </div>

            <div className="pt-4 border-t border-border">
              <h4 className="text-sm font-semibold mb-2">Path Sequence</h4>
              <p className="text-xs font-mono text-muted-foreground leading-relaxed">
                {bestPath
                  .map((cityIndex) => (cities[cityIndex]?.id !== undefined ? cities[cityIndex].id + 1 : "?"))
                  .join(" → ")}{" "}
                →{" "}
                {bestPath[0] !== undefined && cities[bestPath[0]]?.id !== undefined ? cities[bestPath[0]].id + 1 : "?"}
              </p>
            </div>
          </div>
        )}
      </CardContent>
    </Card>
  )
}
