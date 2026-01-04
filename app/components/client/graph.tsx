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

function cantor(k1: number,k2: number): number {
  return ((k1 + k2 + 1)*(k1 + k2) + k2)/2
}

export function LinePlotClient({ lines, height, width, maxx, maxy, metadata }:
  {lines: Line[], height: number, width: number, maxx: number, maxy: number, metadata: ChartMetadata}) {

  const [activePoint, setActivePoint] = useState<Point & {absoluteX: number, absoluteY: number}>(null)

  maxx*=1.1
  maxy*=1.1
  for (let line of lines) {
    line.points.sort((a: Point, b: Point) => { return a.x - b.x; });
  }

  const x = d3.scaleLinear([0,maxx], [0,width]);
  const y = d3.scaleLinear([0,maxy], [0,height]);

  // let svgpaths: React.SVGPathElement = []
  let svgpaths = []
  for (let j = 0; j < lines.length; j++) {
    const line = lines[j]
    let dstr = 'M '
    for (let i = 0; i < line.points.length; i++) {
      let point = line.points[i];
      if (i > 0) {
        dstr += ' L '
      }
      dstr += `${x(point.x)} ${height-y(point.y)}`
    }
    svgpaths.push(
      (
        <path fill="none" stroke={'black'}
          className={line.className} key={j} d={dstr}/>
      )
    )
  }

  const pointRadius = 3;
  const activePointRadius = pointRadius+7;

  let dots = []
  for (let j = 0; j < lines.length; j++) {
    const line = lines[j]
    for (let i = 0; i < line.points.length; i++) {
      let point = line.points[i]
      dots.push(
        (
          <circle key={`${cantor(i,j)}`} cx={x(point.x)} cy={height-y(point.y)} r={pointRadius}
            fill="black"
            stroke={"black"} strokeWidth={3} />
        )
      )
    }
  }

  let activePointJsx = activePoint && (
    <>
      <circle cx={x(activePoint.x)} cy={height - y(activePoint.y)} r={activePointRadius}
      className={'activePoint'} />
    </>
  )

  let activePointPopup = activePoint && (
    <ul className={'popup'} style={{position: 'absolute', zIndex: 1000,
      top: activePoint.absoluteY, left: activePoint.absoluteX}}>
      <li>{`${metadata.xName}: ${activePoint.x}`}</li>
      <li>{`${metadata.yName}: ${activePoint.y}`}</li>
    </ul>
  )

  function handleMouseMove(event) {
    let dist = Infinity
    const mouseX = event.nativeEvent.offsetX
    const mouseY = event.nativeEvent.offsetY
    let activePointCurr: Point & {absoluteX: number, absoluteY: number} | null = null
    for (let line of lines) {
      for (let point of line.points) {
        const currDist = Math.sqrt((mouseX - x(point.x))**2+(mouseY
            - (height - y(point.y)))**2);
        if (currDist < dist && currDist < 80) {
          activePointCurr = {x: point.x, y: point.y,
              absoluteX: event.nativeEvent.pageX,
              absoluteY: event.nativeEvent.pageY
          }
          dist = currDist
          // console.log(activePointCurr)
        }
      }
    }
    // console.log(event)
    if (activePointCurr !== undefined) {
      setActivePoint(activePointCurr)
    }
  }

  return (
    <>
      {/* {activePointPopup} */}
      <svg
        width={width}
        height={height}
        style={{border: '1px solid black'}}
        onMouseMove={handleMouseMove}
        onMouseLeave={() => setActivePoint(null)}
      >
        {svgpaths}
        {dots}
        {activePointJsx}
      </svg>
    </>
  )
}