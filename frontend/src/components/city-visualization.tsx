"use client"

import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import type { City } from "@/components/aco-tsp-dashboard"
import { useMemo } from "react"

interface CityVisualizationProps {
  cities: City[]
  bestPath: number[]
}

export function CityVisualization({ cities, bestPath }: CityVisualizationProps) {
  // Calculate dynamic viewBox based on city coordinates
  const viewBoxData = useMemo(() => {
    if (cities.length === 0) {
      return { minX: 0, minY: 0, width: 500, height: 400 }
    }

    const xs = cities.map(c => c.x)
    const ys = cities.map(c => c.y)
    const minX = Math.min(...xs)
    const maxX = Math.max(...xs)
    const minY = Math.min(...ys)
    const maxY = Math.max(...ys)

    const width = maxX - minX
    const height = maxY - minY

    // Add 10% padding on each side
    const padding = Math.max(width, height) * 0.1

    return {
      minX: minX - padding,
      minY: minY - padding,
      width: width + padding * 2,
      height: height + padding * 2
    }
  }, [cities])

  const viewBox = `${viewBoxData.minX} ${viewBoxData.minY} ${viewBoxData.width} ${viewBoxData.height}`

  // Calculate grid size based on viewBox dimensions
  const gridSize = useMemo(() => {
    const maxDim = Math.max(viewBoxData.width, viewBoxData.height)
    // Aim for ~10 grid lines across the larger dimension
    return Math.ceil(maxDim / 10)
  }, [viewBoxData])

  // Calculate circle radius based on coordinate scale
  const circleRadius = useMemo(() => {
    const maxDim = Math.max(viewBoxData.width, viewBoxData.height)
    // Scale circle size to ~0.5% of the larger dimension
    return Math.max(3, maxDim * 0.005)
  }, [viewBoxData])

  return (
    <Card className="flex flex-col h-full overflow-hidden">
      <CardHeader className="py-3">
        <CardTitle className="text-lg flex items-baseline gap-2">
          City Positions
          <span className="text-xs text-muted-foreground font-mono">
            ({cities.length} cities | viewBox: {viewBoxData.width.toFixed(0)}×{viewBoxData.height.toFixed(0)})
          </span>
        </CardTitle>
      </CardHeader>
      <CardContent className="flex-1 p-2 min-h-0">
        <div className="w-full h-full">
          <svg className="w-full h-full" viewBox={viewBox} preserveAspectRatio="xMidYMid meet">
          {/* Background and grid */}
          <defs>
            <pattern id="grid" width={gridSize} height={gridSize} patternUnits="userSpaceOnUse">
              <rect width={gridSize} height={gridSize} fill="rgb(239, 236, 227)" />
              <path
                d={`M ${gridSize} 0 L 0 0 0 ${gridSize}`}
                fill="none"
                stroke="hsl(var(--border))"
                strokeWidth={gridSize * 0.01}
                opacity="0.3"
              />
            </pattern>
          </defs>
          <rect
            x={viewBoxData.minX}
            y={viewBoxData.minY}
            width={viewBoxData.width}
            height={viewBoxData.height}
            fill="url(#grid)"
          />

          {/* Debug: ViewBox border */}
          <rect
            x={viewBoxData.minX}
            y={viewBoxData.minY}
            width={viewBoxData.width}
            height={viewBoxData.height}
            fill="none"
            stroke="red"
            strokeWidth={circleRadius * 0.2}
            strokeDasharray={`${circleRadius * 2} ${circleRadius}`}
          />

          {/* Draw paths if available */}
          {bestPath.length > 0 && cities.length > 0 && (
            <g>
              {bestPath.map((cityIndex, i) => {
                const nextIndex = bestPath[(i + 1) % bestPath.length]
                const city1 = cities[cityIndex]
                const city2 = cities[nextIndex]
                if (city1 && city2) {
                  return (
                    <line
                      key={`path-${i}`}
                      x1={city1.x}
                      y1={city1.y}
                      x2={city2.x}
                      y2={city2.y}
                      stroke="hsl(var(--primary))"
                      strokeWidth={circleRadius * 0.5}
                      opacity="0.6"
                    />
                  )
                }
                return null
              })}
            </g>
          )}

          {/* Draw cities */}
          {cities.map((city) => (
            <circle
              key={city.id}
              cx={city.x}
              cy={city.y}
              r={circleRadius}
              fill="hsl(var(--accent))"
              stroke="hsl(var(--accent-foreground))"
              strokeWidth={circleRadius * 0.3}
            />
          ))}
          </svg>
        </div>
      </CardContent>
    </Card>
  )
}
