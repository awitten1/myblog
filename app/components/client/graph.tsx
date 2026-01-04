'use client'

import React, { useRef, useState, createContext, useCallback, useMemo } from "react"
import * as d3 from 'd3';

// interface Point {
//   x: number
//   y: number
//   input_size: number
//   cycles: number
// }

interface LineMeta {
  id: string
  label: string
  color: string
}

interface GraphContextType {
  registerPoints: (id: string, points: Point[]) => void
  unregisterPoints: (id: string) => void
  registerLine: (meta: LineMeta) => void
  unregisterLine: (id: string) => void
}

export const GraphContext = createContext<GraphContextType | null>(null)

function range(start: number, end?: number, step = 1) {
  if (end === undefined) {
    end = start
    start = 0
  }
  let ret: number[] = []
  for (let i = start; i <= (end as number); i += step) {
    ret.push(i)
  }
  return ret;
}

export interface Point {
  x: number;
  y: number;
}

export interface Line {
  points: Point[];
  className: string;
  name: string;
}

export interface ChartMetadata {
  xName: string;
  yName: string;
}

function cantor(k1: number, k2: number): number {
  return ((k1 + k2 + 1) * (k1 + k2) + k2) / 2
}

export function LinePlotClient({ lines, height, width, maxx, maxy, metadata }:
  { lines: Line[], height: number, width: number, maxx: number, maxy: number, metadata: ChartMetadata }) {

  const [activePoint, setActivePoint] = useState<(Point & { x_coord: number, y_coord: number }) | null>(null)

  maxx *= 1.05
  maxy *= 1.05
  for (let line of lines) {
    line.points.sort((a: Point, b: Point) => { return a.x - b.x; });
  }

  const lxMargin = 50;
  const yMargin = 50;
  const rxMargin = 50;

  const dataWidth = width - lxMargin - rxMargin;
  const dataHeight = height - yMargin;

  let boxJsx = (
    <rect x={lxMargin} y={0} height={dataHeight}
      width={dataWidth}
      fill="none"
      className="plotRect"
    />
  )

  const x = d3.scaleLinear([0, maxx], [lxMargin, dataWidth]);
  const y = d3.scaleLinear([0, maxy], [0, dataHeight]);

  let svgpaths = []
  for (let j = 0; j < lines.length; j++) {
    const line = lines[j]
    let dstr = 'M '
    for (let i = 0; i < line.points.length; i++) {
      let point = line.points[i];
      if (i > 0) {
        dstr += ' L '
      }
      dstr += `${x(point.x)} ${dataHeight - y(point.y)}`
    }
    svgpaths.push(
      (
        <path fill="none" stroke={'black'}
          className={line.className} key={j} d={dstr} />
      )
    )
  }

  const pointRadius = 3;
  const activePointRadius = pointRadius + 5;

  let dots = []
  for (let j = 0; j < lines.length; j++) {
    const line = lines[j]
    for (let i = 0; i < line.points.length; i++) {
      let point = line.points[i]
      dots.push(
        (
          <circle key={`${cantor(i, j)}`} cx={x(point.x)} cy={dataHeight - y(point.y)} r={pointRadius}
            fill="black"
            stroke={"black"} strokeWidth={3} />
        )
      )
    }
  }

  let activePointJsx = activePoint && (
    <>
      <circle cx={x(activePoint.x)} cy={dataHeight - y(activePoint.y)} r={activePointRadius}
        className={'activePoint'} />
    </>
  )

  let activePointPopup = activePoint && (
    <div className={'popup'} style={{
      position: 'absolute',
      zIndex: 1000,
      top: activePoint.y_coord - 10,
      left: activePoint.x_coord + 10,
      pointerEvents: 'none'
    }}>
      <div className="popup-label">{metadata.xName}: <strong>{activePoint.x}</strong></div>
      <div className="popup-label">{metadata.yName}: <strong>{activePoint.y}</strong></div>
    </div>
  )

  let num_ticks = 10;
  let horizontalLines = []
  let verticalLines = []
  let tickWidth = dataWidth / num_ticks;
  let tickHeight = dataHeight / num_ticks;
  let xLabels = []
  let yLabels = []
  for (let i = 0; i < num_ticks; i++) {
    const xTick = lxMargin + (i + 1) * tickWidth
    const yTick = (i + 1) * tickHeight
    horizontalLines.push(
      (
        <line className={'gridLine'} x1={xTick} x2={xTick}
          y1={0} y2={height - yMargin}
          key={i}
        />
      )
    )
    xLabels.push(
      (
        <text key={i} className={'xLabel'} x={xTick - 10} y={dataHeight + 20}>
          {Math.round(x.invert(xTick))}
        </text>
      )
    )
    verticalLines.push(
      (
        <line className={'gridLine'} y1={yTick} y2={yTick}
          x1={lxMargin} x2={width - rxMargin} key={i} />
      )
    )
    yLabels.push(
      (
        <text key={i} className={'yLabel'} y={yTick + 5} x={lxMargin - 35}>
          {Math.round(y.invert(dataHeight - yTick))}
        </text>
      )
    )
  }

  function handleMouseMove(event) {
    let dist = Infinity
    const mouseX = event.nativeEvent.offsetX
    const mouseY = event.nativeEvent.offsetY
    let activePointCurr: Point & { x_coord: number, y_coord: number } | null = null
    for (let line of lines) {
      for (let point of line.points) {
        const pointX = x(point.x)
        const pointY = dataHeight - y(point.y)
        const currDist = Math.sqrt((mouseX - pointX) ** 2 + (mouseY - pointY) ** 2);
        if (currDist < dist && currDist < 80) {
          activePointCurr = {
            x: point.x,
            y: point.y,
            x_coord: pointX,
            y_coord: pointY
          }
          dist = currDist
        }
      }
    }

    if (activePointCurr !== null) {
      setActivePoint(activePointCurr)
    }
  }

  const legendItems = lines.map((line, idx) => (
    <div key={idx} className="legend-item">
      <span className={`legend-swatch ${line.className}`}></span>
      <span className="legend-label">{line.name}</span>
    </div>
  ))

  const legend = (
    <div className="legend-container">
        {legendItems}
    </div>
  )

  return (
    <div style={{ position: 'relative', width, height }}>
      {activePointPopup}
      {legend}
      <svg
        width={width}
        height={height}
        onMouseMove={handleMouseMove}
        onMouseLeave={() => setActivePoint(null)}
      >
        {boxJsx}
        {svgpaths}
        {dots}
        {activePointJsx}
        {horizontalLines}
        {xLabels}
        {verticalLines}
        {yLabels}
      </svg>
    </div>
  )
}