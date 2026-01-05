'use client'

import React, { useRef, useState, createContext, useCallback, useMemo } from "react"
import * as d3 from 'd3';
import './graph_styles.css'

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
  caption: string;
}

function cantor(k1: number, k2: number): number {
  return ((k1 + k2 + 1) * (k1 + k2) + k2) / 2
}

export function LinePlotClient({ lines, height, width, maxx, maxy, metadata }:
  { lines: Line[], height: number, width: number, maxx: number, maxy: number, metadata: ChartMetadata }) {

  const [activePoint, setActivePoint] = useState<(Point & { x_coord: number, y_coord: number }) | null>(null)

  maxx *= 1.00
  maxy *= 1.05
  for (let line of lines) {
    line.points.sort((a: Point, b: Point) => { return a.x - b.x; });
  }

  const lxMargin = 100;
  const yMargin = 50;
  const rxMargin = 0;

  const dataWidth = width - lxMargin - rxMargin;
  const dataHeight = height - yMargin;

  let boxJsx = (
    <rect x={lxMargin} y={0} height={dataHeight}
      width={dataWidth}
      fill="none"
      className="plotRect"
    />
  )

  const x = d3.scaleLinear([0, maxx], [lxMargin, width - rxMargin - 30]);
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

  const pointRadius = 1;
  const activePointRadius = pointRadius + 3;

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

  const xTicks = x.ticks(Math.max(2, Math.floor(dataWidth / 80)));
  const yTicks = y.ticks(Math.max(2, Math.floor(dataHeight / 50)));

  let horizontalLines = []
  let verticalLines = []
  let xLabels = []
  let yLabels = []

  xTicks.forEach((tick, i) => {
    const xPos = x(tick);
    horizontalLines.push(
      (
        <line className={'gridLine'} x1={xPos} x2={xPos}
          y1={0} y2={dataHeight}
          key={`h-${tick}`}
        />
      )
    );
    xLabels.push(
      (
        <text key={`xl-${tick}`} className={'xLabel'} x={xPos} y={dataHeight + 20} textAnchor="middle">
          {Math.round(tick)}
        </text>
      )
    );
  });

  yTicks.forEach((tick, i) => {
    const yPos = dataHeight - y(tick);
    verticalLines.push(
      (
        <line className={'gridLine'} y1={yPos} y2={yPos}
          x1={lxMargin} x2={width - rxMargin} key={`v-${tick}`} />
      )
    );
    yLabels.push(
      (
        <text key={`yl-${tick}`} className={'yLabel'} y={yPos + 4} x={lxMargin - 15} textAnchor="end">
          {Math.round(tick)}
        </text>
      )
    );
  });

  const xAxisLabel = (
    <text
      x={lxMargin + dataWidth / 2}
      y={dataHeight + 45}
      textAnchor="middle"
      className="axis-label"
    >
      {metadata.xName}
    </text>
  )

  const yAxisLabel = (
    <text
      x={-(dataHeight / 2)}
      y={lxMargin - 45}
      transform="rotate(-90)"
      textAnchor="middle"
      className="axis-label"
    >
      {metadata.yName}
    </text>
  )

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
    <figure className="plot" style={{ position: 'relative', width, height }}>
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
          {xAxisLabel}
          {yAxisLabel}
        </svg>
        <figcaption>
          {metadata.caption}.
        </figcaption>
    </figure>
  )
}