"use client"

import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import type { City } from "@/components/aco-tsp-dashboard"

interface CityVisualizationProps {
  cities: City[]
  bestPath: number[]
}

export function CityVisualization({ cities, bestPath }: CityVisualizationProps) {
  return (
    <Card className="flex flex-col">
      <CardHeader>
        <CardTitle className="text-xl">City Positions</CardTitle>
      </CardHeader>
      <CardContent className="flex-1 p-0">
        <svg className="w-full h-full" viewBox="0 0 500 400" preserveAspectRatio="xMidYMid meet">
          {/* Grid lines */}
          <defs>
            <pattern id="grid" width="50" height="50" patternUnits="userSpaceOnUse">
              <path d="M 50 0 L 0 0 0 50" fill="none" stroke="hsl(var(--border))" strokeWidth="0.5" opacity="0.3" />
            </pattern>
          </defs>
          <rect width="500" height="400" fill="url(#grid)" />

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
                      strokeWidth="2"
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
            <g key={city.id}>
              <circle
                cx={city.x}
                cy={city.y}
                r="6"
                fill="hsl(var(--accent))"
                stroke="hsl(var(--accent-foreground))"
                strokeWidth="2"
              />
              <text
                x={city.x}
                y={city.y - 12}
                fontSize="10"
                fill="hsl(var(--muted-foreground))"
                textAnchor="middle"
                className="font-mono"
              >
                {city.id + 1}
              </text>
            </g>
          ))}
        </svg>
      </CardContent>
    </Card>
  )
}
