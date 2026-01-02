'use client'

import { useRef } from "react"

function range(start, end?: number, step = 1) {
  if (end == undefined) {
    end = start
    start = 0
  }
  let ret = []
  for (let i = start; i <= end; i+=step) {
    ret.push(i)
  }
  return ret;
}

export function TestSvgComponent({ children, height, width }) {
  const horizontalLineRef = useRef(null);
  const verticalLineRef = useRef(null);
  console.log(children)

  const sep = 30

  console.log(10)

  return (
    <div style={{ display: 'flex', justifyContent: 'center', width: '100%',
      flexDirection: 'column'
    }}>
    <svg
      width={width}
      height={height}
      onMouseMove={(event) => {
        const x = event.nativeEvent.offsetX;
        const y = event.nativeEvent.offsetY;

        if (horizontalLineRef.current) {
          horizontalLineRef.current.setAttribute('y1', y);
          horizontalLineRef.current.setAttribute('y2', y);
        }
        if (verticalLineRef.current) {
          verticalLineRef.current.setAttribute('x1', x);
          verticalLineRef.current.setAttribute('x2', x);
        }
      }}
      style={{border: '2px solid black'}}
    >
      <g fill="black">
        <rect width={width} height={height}></rect>
      </g>

      <g fill="white" stroke="white" strokeWidth="0.3">
        {range(0, width, sep).map((i) => (
          <line key={i} x1={i} y1={0} x2={i} y2={height}></line>
        ))}

        {range(0, width, sep).map((i) => (
          <line key={i} x1={0} y1={i} x2={width} y2={i}></line>
        ))}
      </g>

      <g stroke="oklch(0.9 0.3 50)" strokeWidth="2">
        {/* Vertical line */}
        <line
          ref={verticalLineRef}
          x1={0}
          y1={0}
          x2={0}
          y2={height}
        />
        {/* Horizontal line */}
        <line
          ref={horizontalLineRef}
          x1={0}
          y1={0}
          x2={width}
          y2={0}
        />
      </g>
      {children}
    </svg>
    <p>more text</p>
    </div>
  )
}