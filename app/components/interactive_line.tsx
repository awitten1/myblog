'use client'

import React, { useEffect, useContext } from 'react'
import { GraphContext } from './client/graph'

interface Point {
  x: number
  y: number
  input_size: number
  cycles: number
}

interface InteractiveLineProps {
  points: Point[]
  strokeColor: string
  id: string
  label: string
}

export function InteractiveLine({ points, strokeColor, id, label }: InteractiveLineProps) {
  const context = useContext(GraphContext)

  useEffect(() => {
    if (context) {
      context.registerPoints(id, points)
      context.registerLine({ id, label, color: strokeColor })
    }
    return () => {
      if (context) {
        context.unregisterPoints(id)
        context.unregisterLine(id)
      }
    }
  }, [id, points, strokeColor, label, context])

  let dstr = 'M '
  for (let i = 0; i < points.length; i++) {
    if (i > 0) {
      dstr += ' L '
    }
    dstr += `${points[i].x} ${points[i].y}`
  }

  return (
    <>
      <path d={dstr} stroke={strokeColor} strokeWidth={2} fill={'none'} />
      {points.map((p, i) => (
        <circle key={i} cx={p.x} cy={p.y} r={3} fill={'black'} />
      ))}
    </>
  )
}
