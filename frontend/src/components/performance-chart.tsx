"use client"

import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import dynamic from "next/dynamic"
import { useMemo } from "react"

const Plot = dynamic(() => import("react-plotly.js"), { ssr: false })

interface PerformanceChartProps {
  pathHistory: { iteration: number; length: number }[]
}

export function PerformanceChart({ pathHistory }: PerformanceChartProps) {
  const iterations = pathHistory.map((d) => d.iteration)
  const lengths = pathHistory.map((d) => d.length)

  // Calculate y-axis range dynamically
  const yAxisRange = useMemo(() => {
    if (lengths.length === 0) return [0, 100]

    const minLength = Math.min(...lengths)
    const maxLength = Math.max(...lengths)
    const range = maxLength - minLength
    const padding = range * 0.1 // 10% padding

    return [
      Math.max(0, minLength - padding),
      maxLength + padding
    ]
  }, [lengths])

  return (
    <Card className="flex flex-col h-full overflow-hidden">
      <CardHeader className="py-3">
        <CardTitle className="text-lg">Convergence History</CardTitle>
      </CardHeader>
      <CardContent className="flex-1 p-4">
        {pathHistory.length === 0 ? (
          <div className="flex items-center justify-center h-full text-muted-foreground">
            <p>Waiting for simulation data...</p>
          </div>
        ) : (
          <Plot
            data={[
              {
                x: iterations,
                y: lengths,
                type: "scatter",
                mode: "lines",
                name: "Best Path Length",
                line: {
                  color: "rgb(74, 112, 169)",
                  width: 2,
                },
              },
            ]}
            layout={{
              autosize: true,
              margin: { l: 60, r: 20, t: 20, b: 50 },
              paper_bgcolor: "rgba(0,0,0,0)",
              plot_bgcolor: "rgba(0,0,0,0)",
              xaxis: {
                title: { text: "Iteration", font: { color: "rgb(80, 80, 80)" } },
                gridcolor: "rgba(180, 177, 168, 0.3)",
                linecolor: "rgb(180, 177, 168)",
                tickfont: { color: "rgb(80, 80, 80)" },
              },
              yaxis: {
                title: { text: "Path Length", font: { color: "rgb(80, 80, 80)" } },
                gridcolor: "rgba(180, 177, 168, 0.3)",
                linecolor: "rgb(180, 177, 168)",
                tickfont: { color: "rgb(80, 80, 80)" },
                range: yAxisRange,
                autorange: false,
              },
              font: {
                family: "system-ui, sans-serif",
              },
              hovermode: "x unified",
            }}
            config={{
              displayModeBar: true,
              displaylogo: false,
              modeBarButtonsToRemove: ["lasso2d", "select2d"],
              responsive: true,
            }}
            style={{ width: "100%", height: "100%" }}
            useResizeHandler={true}
          />
        )}
      </CardContent>
    </Card>
  )
}
