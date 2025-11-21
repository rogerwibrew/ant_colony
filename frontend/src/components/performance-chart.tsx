"use client"

import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from "recharts"

interface PerformanceChartProps {
  pathHistory: { iteration: number; length: number }[]
}

export function PerformanceChart({ pathHistory }: PerformanceChartProps) {
  return (
    <Card className="flex flex-col">
      <CardHeader>
        <CardTitle className="text-xl">Convergence History</CardTitle>
      </CardHeader>
      <CardContent className="flex-1 p-4">
        {pathHistory.length === 0 ? (
          <div className="flex items-center justify-center h-full text-muted-foreground">
            <p>Waiting for simulation data...</p>
          </div>
        ) : (
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={pathHistory} margin={{ top: 5, right: 20, left: 0, bottom: 5 }}>
              <CartesianGrid strokeDasharray="3 3" stroke="hsl(var(--border))" opacity={0.3} />
              <XAxis
                dataKey="iteration"
                stroke="hsl(var(--muted-foreground))"
                style={{ fontSize: "12px" }}
                label={{
                  value: "Iteration",
                  position: "insideBottom",
                  offset: -5,
                  fill: "hsl(var(--muted-foreground))",
                }}
              />
              <YAxis
                stroke="hsl(var(--muted-foreground))"
                style={{ fontSize: "12px" }}
                label={{
                  value: "Path Length",
                  angle: -90,
                  position: "insideLeft",
                  fill: "hsl(var(--muted-foreground))",
                }}
              />
              <Tooltip
                contentStyle={{
                  backgroundColor: "hsl(var(--popover))",
                  border: "1px solid hsl(var(--border))",
                  borderRadius: "6px",
                  fontSize: "12px",
                }}
                labelStyle={{ color: "hsl(var(--popover-foreground))" }}
                itemStyle={{ color: "hsl(var(--primary))" }}
              />
              <Line
                type="monotone"
                dataKey="length"
                stroke="hsl(var(--primary))"
                strokeWidth={2}
                dot={false}
                name="Best Path Length"
              />
            </LineChart>
          </ResponsiveContainer>
        )}
      </CardContent>
    </Card>
  )
}
